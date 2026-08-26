#include "engine/framework/streamfilecache_internal.h"

#include "engine/framework/streamfilelog.h"
#include "shared/idlib/csystems/cvarsystemlocal.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/sys/sys_time.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstring>
#include <Windows.h>

namespace {
struct uncachedRequest_t {
  idFile* filePtr;
  fsDevice_t fileDevice;
  std::byte padding0[4];
  std::int64_t fileOfs;
  int length;
  std::byte padding1[4];
  void** dest;
  memTag_t tag;
  std::byte padding2[4];
  volatile bool* completionFlag;
};
struct streamRequest_t {
  idFile* filePtr;
  int fileID;
  int fileLine;
  int priority;
  int persistence;
  std::byte remaining[72];
};
static_assert(sizeof(uncachedRequest_t)==56);
static_assert(sizeof(streamRequest_t)==96);
}

void CacheLineQuickSort(cacheLineSort_t* values,std::int64_t count) {
  if(count<=1) return;
  std::sort(values,values+count,[](const cacheLineSort_t& left,
                                   const cacheLineSort_t& right) {
    return left.sort>right.sort;
  });
}

void idLocklessCacheLineQueueS::Add(idFileCacheLine* line) {
  queue[queueEnd]=line;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  queueEnd=(static_cast<unsigned short>(queueEnd)+1)&0x7FFF;
  Sys_InterlockedIncrement(const_cast<int*>(&numLines.value));
}

idFileCacheLine* idLocklessCacheLineQueueS::Next() {
  const int start=queueStart;
  if(start==queueEnd) return nullptr;
  idFileCacheLine* line=queue[start];
  queueStart=(static_cast<unsigned short>(start)+1)&0x7FFF;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  Sys_InterlockedDecrement(const_cast<int*>(&numLines.value));
  return line;
}

void idLocklessCacheLineQueueM::Add(idFileCacheLine* line) {
  int index=queueEnd;
  while(Sys_InterlockedCompareExchangePointer(
      reinterpret_cast<void**>(&queue[index&0x7FFF]),0,
      reinterpret_cast<std::int64_t>(line))!=0) ++index;
  for(;;) {
    const int end=queueEnd;
    if(queue[end&0x7FFF]==nullptr) break;
    if(Sys_InterlockedCompareExchange(const_cast<int*>(&queueEnd),end,end+1)!=end)
      break;
  }
  Sys_InterlockedIncrement(const_cast<int*>(&numLines.value));
}

idFileCacheLine* idLocklessCacheLineQueueM::Next() {
  for(;;) {
    const int start=queueStart;
    if(start==queueEnd) return nullptr;
    idFileCacheLine* line=queue[start&0x7FFF];
    if(line==nullptr) {
      Sys_InterlockedCompareExchange(const_cast<int*>(&queueStart),start,start+1);
      continue;
    }
    if(Sys_InterlockedCompareExchangePointer(
        reinterpret_cast<void**>(&queue[start&0x7FFF]),
        reinterpret_cast<std::int64_t>(line),0)!=
       reinterpret_cast<std::int64_t>(line)) continue;
    Sys_InterlockedCompareExchange(const_cast<int*>(&queueStart),start,start+1);
    Sys_InterlockedDecrement(const_cast<int*>(&numLines.value));
    return line;
  }
}

void idLocklessCacheLineMRU::Condense() {
  unsigned end=static_cast<unsigned short>(queueEnd);
  unsigned scan=end;
  if(static_cast<int>(end)!=queueStart) {
    do {
      scan=static_cast<unsigned short>(scan-1);
      idFileCacheLine* line=queue[scan];
      if(line!=nullptr && Sys_InterlockedExchangePointer(
          reinterpret_cast<void**>(&queue[scan]),0)==
          reinterpret_cast<std::int64_t>(line)) {
        end=static_cast<unsigned short>(end-1);
        line->indexMRU=static_cast<int>(end);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        queue[end]=line;
      }
    } while(static_cast<int>(scan)!=queueStart);
  }
  std::atomic_thread_fence(std::memory_order_seq_cst);
  queueStart=static_cast<int>(end);
}

void idLocklessCacheLineMRU::AddMRU(idFileCacheLine* line) {
  while(queueStart!=queueEnd) {
    if(queue[queueStart]!=nullptr) break;
    queueStart=static_cast<unsigned short>(queueStart+1);
  }
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int end=queueEnd;
  int adjusted=end;
  if(adjusted<queueStart) adjusted+=0x10000;
  if(end+0x8000==queueStart || adjusted-queueStart>2*numLines.value)
    Condense();
  line->indexMRU=queueEnd;
  queue[queueEnd]=line;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  queueEnd=static_cast<unsigned short>(queueEnd+1);
  Sys_InterlockedIncrement(const_cast<int*>(&numLines.value));
}

void idLocklessCacheLineMRU::Sort() {
  Condense();
  const int replacementSnapshot=replacementCount.value;
  const int now=static_cast<int>(Sys_Milliseconds());
  std::array<cacheLineSort_t,32768> sorted{};
  int count=0;
  int index=queueStart;
  while(index!=queueEnd) {
    idFileCacheLine* line=queue[index];
    if(line!=nullptr) {
      sorted[count].index=index;
      sorted[count].sort=
          (static_cast<std::int64_t>(line->persistence-
               (replacementSnapshot-line->replacements))<<32)-
          static_cast<std::int64_t>(now-line->time);
      ++count;
    }
    index=static_cast<unsigned short>(index+1);
  }
  CacheLineQuickSort(sorted.data(),count);
  queueEnd=static_cast<unsigned short>(queueEnd+count);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int end=queueEnd;
  for(int sortedIndex=0;sortedIndex<count;++sortedIndex) {
    const int oldIndex=static_cast<int>(sorted[sortedIndex].index);
    idFileCacheLine* line=queue[oldIndex];
    if(line!=nullptr && Sys_InterlockedExchangePointer(
        reinterpret_cast<void**>(&queue[oldIndex]),0)==
        reinterpret_cast<std::int64_t>(line)) {
      end=static_cast<unsigned short>(end-1);
      line->indexMRU=end;
      std::atomic_thread_fence(std::memory_order_seq_cst);
      queue[end]=line;
    }
  }
  std::atomic_thread_fence(std::memory_order_seq_cst);
  queueStart=end;
}

void idLocklessCacheLineMRU::MoveMRU(idFileCacheLine* line) {
  const int index=line->indexMRU;
  if(index!=-1 && Sys_InterlockedCompareExchangePointer(
      reinterpret_cast<void**>(&queue[index]),
      reinterpret_cast<std::int64_t>(line),0)==
      reinterpret_cast<std::int64_t>(line)) {
    line->indexMRU=-1;
    Sys_InterlockedDecrement(const_cast<int*>(&numLines.value));
  }
  AddMRU(line);
}

idFileCacheLine* idLocklessCacheLineMRU::NextLRU(memoryType_t type) {
  int start=queueStart;
  while(start!=queueEnd) {
    idFileCacheLine* line=queue[start];
    if(line!=nullptr && (type==MEMORY_TYPE_ANY || line->memType==type)) {
      if(Sys_InterlockedIncrement(const_cast<int*>(&line->modifyLock.value))==1) {
        if(Sys_InterlockedDecrement(const_cast<int*>(&line->pinned.value))==-1 &&
           Sys_InterlockedCompareExchangePointer(
               reinterpret_cast<void**>(&queue[start]),
               reinterpret_cast<std::int64_t>(line),0)==
               reinterpret_cast<std::int64_t>(line)) {
          line->indexMRU=-1;
          Sys_InterlockedDecrement(const_cast<int*>(&numLines.value));
          Sys_InterlockedIncrement(const_cast<int*>(&replacementCount.value));
          return line;
        }
        Sys_InterlockedIncrement(const_cast<int*>(&line->pinned.value));
      }
      Sys_InterlockedDecrement(const_cast<int*>(&line->modifyLock.value));
    }
    start=static_cast<unsigned short>(start+1);
  }
  return nullptr;
}

void idLocklessCacheLineHash::Remove(idFileCacheLine* line) {
  const int key=(line->fileLine+32*line->fileID)&0x7FFF;
  const int count=numHashEntries[key];
  for(int index=0;index<count;++index) {
    const int slot=(index+4*key)&0x1FFFF;
    idFileCacheLine* found=hash[slot];
    if(found!=nullptr && found->fileID==line->fileID &&
       found->fileLine==line->fileLine) {
      hash[slot]=nullptr;
      Sys_InterlockedDecrement(const_cast<int*>(&numLines.value));
      return;
    }
  }
}

void idLocklessCacheLineHash::Add(idFileCacheLine* line) {
  const int key=(line->fileLine+32*line->fileID)&0x7FFF;
  const int index=numHashEntries[key]++;
  hash[(4*key+index)&0x1FFFF]=line;
  Sys_InterlockedIncrement(const_cast<int*>(&numLines.value));
}

idFileCacheLine* idLocklessCacheLineHash::Find(int fileID,int fileLine) {
  const int key=(fileLine+32*fileID)&0x7FFF;
  const int count=numHashEntries[key];
  for(int index=0;index<count;++index) {
    idFileCacheLine* line=hash[(4*key+index)&0x1FFFF];
    if(line!=nullptr && line->fileID==fileID && line->fileLine==fileLine)
      return line;
  }
  return nullptr;
}

void idLocklessCacheLineHash::Condense() {
  for(int key=0;key<32768;++key) {
    int offset=4*key;
    int index=numHashEntries[key]-1;
    while(index>=0) {
      const int tail=(4*key+index)&0x1FFFF;
      idFileCacheLine* line=hash[tail];
      if(line==nullptr || ((line->fileLine+32*line->fileID)&0x7FFF)!=key) {
        --numHashEntries[key];
        --index;
        continue;
      }
      const int tailOffset=tail;
      while(hash[offset]!=nullptr) {
        offset=(offset+1)&0x1FFFF;
        if(offset==tailOffset) break;
      }
      if(offset==tailOffset) break;
      if(Sys_InterlockedIncrement(
             const_cast<int*>(&line->modifyLock.value))==1) {
          hash[tail]=nullptr;
          hash[offset]=line;
          std::atomic_thread_fence(std::memory_order_seq_cst);
          --numHashEntries[key];
          Sys_InterlockedDecrement(const_cast<int*>(&line->modifyLock.value));
          --index;
      } else {
        Sys_InterlockedDecrement(const_cast<int*>(&line->modifyLock.value));
        break;
      }
    }
  }
}

idStreamFileCache::idStreamFileCache()
    :interfaceMutex(),controlThread(nullptr),cancelToTerminate(false) {}

idStreamFileCache::~idStreamFileCache()=default;

idStreamControlThread::idStreamControlThread():idSysThread(),recoveredStorage{} {
  Field<bool>(88)=false;
  Field<bool>(89)=false;
  Field<int>(92)=0;
  Field<int>(96)=0;
  Field<int>(8296)=Field<int>(8300)=0;
  Field<int>(8304)=0;
  Field<bool>(8308)=Field<bool>(8309)=false;
  Field<int>(925816)=Field<int>(925820)=0;
  Field<int>(1843328)=Field<int>(1843332)=0;
  Field<int>(2760840)=Field<int>(2760844)=0;
  Field<int>(4333712)=Field<int>(4333716)=0;
}

void idStreamControlThread::UncachedBackgroundRead(
    idFile* file,std::int64_t offset,int length,void** destination,memTag_t tag,
    volatile bool* completion) {
  auto* queue=&Field<uncachedRequest_t>(8312);
  int& start=Field<int>(925816);
  int& end=Field<int>(925820);
  while(((static_cast<unsigned short>(end)+1)&0x3FFF)==start) {
    idLib::Warning("idStreamControlThread::UncachedBackgroundRead: queue is full");
    Sys_Sleep(1);
  }
  uncachedRequest_t& request=queue[end];
  request.filePtr=file;
  request.fileDevice=file->GetDevice();
  request.fileOfs=offset;
  request.length=length;
  request.dest=destination;
  request.tag=tag;
  request.completionFlag=completion;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  end=(static_cast<unsigned short>(end)+1)&0x3FFF;
}

void idStreamControlThread::BatchUncachedBackgroundRead(
    idFile* file,std::int64_t* offsets,int* lengths,void*** destinations,
    memTag_t tag,volatile bool** completions,int count) {
  auto* queue=&Field<uncachedRequest_t>(8312);
  int& start=Field<int>(925816);
  int& end=Field<int>(925820);
  const fsDevice_t device=file->GetDevice();
  for(int index=0;index<count;++index) {
    while(((static_cast<unsigned short>(end)+index+1)&0x3FFF)==start) {
      idLib::Warning("idStreamControlThread::BatchUncachedBackgroundRead: queue is full");
      Sys_Sleep(1);
    }
    uncachedRequest_t& request=queue[(end+index)&0x3FFF];
    request.filePtr=file;
    request.fileDevice=device;
    request.fileOfs=offsets[index];
    request.length=lengths[index];
    request.dest=destinations[index];
    request.tag=tag;
    request.completionFlag=completions[index];
  }
  std::atomic_thread_fence(std::memory_order_seq_cst);
  end=(static_cast<unsigned short>(end)+count)&0x3FFF;
}

void idStreamControlThread::AddCacheLineRequest(
    idFile* file,int fileID,int fileLine,int priority,int persistence) {
  auto* queue=&Field<streamRequest_t>(2760848);
  int& start=Field<int>(4333712);
  int& end=Field<int>(4333716);
  while(static_cast<unsigned short>(end+1)==start) {
    idLib::Warning("idStreamControlThread::AddCacheLineRequest: queue is full");
    SignalWork();
    WaitForThread();
  }
  streamRequest_t& request=queue[end];
  request.priority=priority;
  request.filePtr=file;
  request.fileID=fileID;
  request.fileLine=fileLine;
  request.persistence=persistence;
  std::atomic_thread_fence(std::memory_order_seq_cst);
  end=static_cast<unsigned short>(end+1);
}

void idStreamControlThread::BatchFreeMemory(void** blocks,int count) {
  auto** queue=&Field<void*>(104);
  int& start=Field<int>(8296);
  int& end=Field<int>(8300);
  for(int index=0;index<count;++index) {
    bool signalled=false;
    while(((static_cast<unsigned short>(end)+index+1)&0x3FF)==start) {
      idLib::Warning("idStreamControlThread::BatchFreeMemory: queue is full");
      if(!signalled){SignalWork();signalled=true;}
      Sys_Sleep(1);
    }
    queue[(end+index)&0x3FF]=blocks[index];
  }
  std::atomic_thread_fence(std::memory_order_seq_cst);
  end=(static_cast<unsigned short>(end)+count)&0x3FF;
  SignalWork();
}

unsigned char* idStreamControlThread::AllocProvidedLineBuffer() {
  auto* total=&Field<idSysInterlockedInteger>(7618864);
  auto* used=&Field<idSysInterlockedInteger>(7618868);
  const int maximum=Field<int>(7618872);
  if(total->value<=0 || (maximum>=0 && used->value>=maximum)) return nullptr;
  auto** buffers=&Field<void*>(7610672);
  for(int index=0;index<1024;++index) {
    void* buffer=buffers[index];
    if(buffer!=nullptr && Sys_InterlockedCompareExchangePointer(
        &buffers[index],reinterpret_cast<std::int64_t>(buffer),0)==
        reinterpret_cast<std::int64_t>(buffer)) {
      Sys_InterlockedIncrement(const_cast<int*>(&used->value));
      return static_cast<unsigned char*>(buffer);
    }
  }
  return nullptr;
}

void idStreamControlThread::FreeLineBuffer(unsigned char* buffer,
                                            memoryType_t type) {
  auto* totalUsed=&Field<idSysInterlockedInteger>(7610664);
  if(type==MEMORY_TYPE_ALLOCATED) {
    VirtualFree(buffer,0,MEM_RELEASE);
    Sys_InterlockedDecrement(const_cast<int*>(&totalUsed->value));
  } else if(type==MEMORY_TYPE_PROVIDED) {
    auto* used=&Field<idSysInterlockedInteger>(7618868);
    auto* total=&Field<idSysInterlockedInteger>(7618864);
    Sys_InterlockedDecrement(const_cast<int*>(&used->value));
    auto** buffers=&Field<void*>(7610672);
    bool returned=false;
    for(int index=0;index<1024;++index) {
      if(buffers[index]==nullptr && Sys_InterlockedCompareExchangePointer(
          &buffers[index],0,reinterpret_cast<std::int64_t>(buffer))==0) {
        returned=true;
        break;
      }
    }
    if(!returned) Sys_InterlockedDecrement(const_cast<int*>(&total->value));
    Sys_InterlockedDecrement(const_cast<int*>(&totalUsed->value));
  }
}

bool idStreamControlThread::FreeLRU() {
  ++Field<int>(92);
  auto& mru=Field<idLocklessCacheLineMRU>(5906688);
  idFileCacheLine* line=mru.NextLRU(MEMORY_TYPE_ALLOCATED);
  if(line==nullptr) return false;
  unsigned char* data=line->data;
  const memoryType_t type=line->memType;
  line->data=nullptr;
  Field<idLocklessCacheLineHash>(6430992).Remove(line);
  line->state=CACHE_LINE_STATE_FREE;
  Field<idLocklessCacheLineQueueM>(5644528).Add(line);
  if(data==nullptr) return false;
  FreeLineBuffer(data,type);
  return true;
}

int idStreamControlThread::CondenseCacheLineQueue(
    idLocklessCacheLineQueueS& source,int timeout) {
  int end=source.queueEnd;
  int scan=end;
  int removed=0;
  if(end!=source.queueStart) {
    do {
      scan=(static_cast<unsigned short>(scan)-1)&0x7FFF;
      idFileCacheLine* line=source.queue[scan];
      source.queue[scan]=nullptr;
      if(line!=nullptr) {
        if(line->time<=timeout) {
          Sys_InterlockedDecrement(const_cast<int*>(&source.numLines.value));
          Field<idLocklessCacheLineHash>(6430992).Remove(line);
          line->state=CACHE_LINE_STATE_FREE;
          Field<idLocklessCacheLineQueueM>(5644528).Add(line);
          ++removed;
        } else {
          end=(static_cast<unsigned short>(end)-1)&0x7FFF;
          source.queue[end]=line;
        }
      }
    } while(scan!=source.queueStart);
  }
  source.queueStart=end;
  return removed;
}

void idStreamFileCache::NewFrame() {
  controlThread->Field<bool>(88)=true;
  streamFileLog.NoteFrame();
}

void idStreamFileCache::ServiceBink(fsDevice_t source) {
  controlThread->Field<int>(8304)=(source!=FS_DEVICE_OPTICAL_DISK_DRIVE)+1;
  controlThread->SignalWork();
}

void idStreamFileCache::SignalWork() {
  idScopedCriticalSection lock(interfaceMutex);
  controlThread->SignalWork();
}

void idStreamFileCache::UncachedBackgroundRead(
    idFile* file,std::int64_t offset,int length,void** destination,memTag_t tag,
    volatile bool* completion) {
  idScopedCriticalSection lock(interfaceMutex);
  controlThread->UncachedBackgroundRead(file,offset,length,destination,tag,
                                        completion);
}

void idStreamFileCache::BatchUncachedBackgroundRead(
    idFile* file,std::int64_t* offsets,int* lengths,void*** destinations,
    memTag_t tag,volatile bool** completions,int count) {
  idScopedCriticalSection lock(interfaceMutex);
  controlThread->BatchUncachedBackgroundRead(file,offsets,lengths,destinations,
                                             tag,completions,count);
}

void idStreamFileCache::BatchFreeMemory(void** blocks,int count) {
  idScopedCriticalSection lock(interfaceMutex);
  controlThread->BatchFreeMemory(blocks,count);
}

std::int64_t idStreamFileCache::GetCurrentUsageLimit() {
  return static_cast<std::int64_t>(controlThread->Field<idSysInterlockedInteger>(
      7610664).value)<<16;
}

bool idStreamFileCache::OutOfMemoryCallback() {
  return controlThread!=nullptr && controlThread->FreeLRU();
}

idStreamFileCache StreamFileCacheLocal;
idStreamFileCache* streamFileCache=&StreamFileCacheLocal;
