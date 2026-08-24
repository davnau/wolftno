#pragma once

#include "engine/network/serializerlog.h"
#include "engine/network/snapshot_jobs.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/networking/bitmsg.h"

#include <cstdint>
#include <new>

template <typename T, int BlockSize, int Tag> class idBlockAlloc {
public:
  union element_t {
    T data;
    element_t* next;
    std::uint8_t buffer[sizeof(T)];
    element_t() {}
    ~element_t() {}
  };
  struct alignas(8) idBlock {
    element_t elements[BlockSize];
    idBlock* next;
    element_t* free;
    int freeCount;
  };

  idBlockAlloc() : allowAllocs(true), clearAllocs(true) {}
  ~idBlockAlloc() { Shutdown(); }
  void AllocNewBlock() {
    auto* block = static_cast<idBlock*>(::operator new(sizeof(idBlock)));
    block->next = blocks;
    block->free = nullptr;
    block->freeCount = BlockSize;
    blocks = block;
    for (int i = 0; i < BlockSize; ++i) {
      block->elements[i].next = free;
      free = &block->elements[i];
    }
    total += BlockSize;
  }
  T* Alloc() {
    if (free == nullptr) {
      if (!allowAllocs) return nullptr;
      AllocNewBlock();
    }
    element_t* element = free;
    free = element->next;
    ++active;
    return new (&element->data) T();
  }
  void Free(T* value) {
    if (value == nullptr) return;
    value->~T();
    auto* element = reinterpret_cast<element_t*>(value);
    element->next = free;
    free = element;
    --active;
  }
  void Shutdown() {
    while (blocks != nullptr) {
      idBlock* block = blocks;
      blocks = block->next;
      ::operator delete(block);
    }
    free = nullptr;
    total = 0;
    active = 0;
  }

  idBlock* blocks{};
  element_t* free{};
  int total{};
  int active{};
  bool allowAllocs;
  bool clearAllocs;
};

class idSnapShot {
public:
  struct objectBuffer_t {
    objectBuffer_t() = default;
    explicit objectBuffer_t(unsigned int size) { Alloc(size); }
    objectBuffer_t(const objectBuffer_t& other) { *this = other; }
    ~objectBuffer_t();
    objectBuffer_t& operator=(const objectBuffer_t& other);
    void Alloc(unsigned int size);
    void _AddRef();
    void _Release();
    int NumRefs() const { return data == nullptr ? 0 : data[size]; }
    unsigned int Size() const { return size; }
    std::uint8_t* Ptr() { return data; }
    const std::uint8_t* Ptr() const { return data; }
    std::uint8_t* data{};
    unsigned int size{};
  };

  struct objectState_t {
    void Print(const char* name) const;
    std::uint16_t objectNum{};
    objectBuffer_t buffer;
    std::uint32_t visMask{0xFFFFFFFFu};
    bool stale{};
    bool deleted{};
    int changedCount{};
    int expectedSequence{-1};
    bool createdFromTemplate{};
  };

  struct submitDeltaJobsInfo_t {
    objParms_t* objParms{};
    int maxObjParms{};
    std::uint8_t* objMemory{};
    objHeader_t* headers{};
    int maxHeaders{};
    int maxObjMemory{};
    lzwParm_t* lzwParms{};
    int maxDeltaParms{};
    idSnapShot* oldSnap{};
    int visIndex{};
    int baseSequence{};
    idSnapShot* templateStates{};
    lzwInOutData_t* lzwInOutData{};
  };

  idSnapShot();
  idSnapShot(const idSnapShot& other);
  ~idSnapShot();
  idSnapShot& operator=(const idSnapShot& other);
  void Clear();
  void SetSerializerLog(idSerializerLog* log);
  idSerializerLog* GetSerializerLog() const { return serializeLog; }
  int GetTime() const { return time; }
  void SetTime(int value) { time = value; }
  int GetRecvTime() const { return recvTime; }
  void SetRecvTime(int value) { recvTime = value; }
  static void PeekDeltaSequence(char* memory, int size, int& sequence, int& baseSequence);
  bool ReadDeltaForJob(char* memory, int size, int visIndex, idSnapShot* templateStates);
  void SubmitWriteDeltaToJobs(const submitDeltaJobsInfo_t& info);
  objectState_t* S_AddObject(int objectNum, std::uint32_t visMask, const char* data,
                             unsigned int size);
  objectState_t* S_AddObject(int objectNum, std::uint32_t visMask, const idBitMsg& message) {
    return S_AddObject(objectNum, visMask,
                       reinterpret_cast<const char*>(message.readData), message.GetSize());
  }
  bool CopyObject(const idSnapShot& older, int objectNum, bool forceStale = false);
  int CompareObject(const idSnapShot* older, int objectNum, int start = 0, int end = 0,
                    int oldStart = 0);
  int NumObjects() const { return objectStates.Num(); }
  int GetObjectMsgByIndex(int index, idBitMsg& message, bool ignoreIfStale = false) const;
  int FindObjectIndexByID(int objectNum) const;
  objectState_t* FindObjectByID(int objectNum) const;
  int ObjectChangedCountByIndex(int index) const;
  void UpdateExpectedSeq(int sequence);
  void ApplyToExistingState(int objectID, idBitMsg& message);
  objectState_t* GetTemplateState(int objectNum, idSnapShot* templates,
                                  objectState_t* newState = nullptr);

private:
  int BinarySearch(int objectNum) const;
  objectState_t* FindOrCreateObjectByID(int objectNum);
  void FreeObjectState(int index);
  void SubmitLZWJob(const submitDeltaJobsInfo_t& info, objParms_t*& base,
                    objParms_t*& current, lzwParm_t*& lzw, bool saveDictionary);

  idList<objectState_t*, 57> objectStates;
  idBlockAlloc<objectState_t, 16, 57> allocatedObjs;
  idSerializerLog* serializeLog{};
  int time{};
  int recvTime{};
  bool isGlobalHeap{};
};

static_assert(sizeof(idSnapShot::objectBuffer_t) == 16, "Wolf snapshot buffer ABI drift");
static_assert(sizeof(idSnapShot::objectState_t) == 48, "Wolf snapshot object ABI drift");
static_assert(sizeof(idBlockAlloc<idSnapShot::objectState_t, 16, 57>) == 32,
              "Wolf snapshot allocator ABI drift");
static_assert(sizeof(idSnapShot) == 80, "Wolf snapshot ABI drift");
