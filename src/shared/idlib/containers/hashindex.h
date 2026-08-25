#pragma once

class idHashIndex {
public:
  int* hash;
  int* indexChain;
  int hashSize;
  int indexSize;
  int granularity;
  int hashMask;
  int lookupMask;
  int memTag;

  static int* INTERNAL_INVALID_INDEX;

  idHashIndex(int initialHashSize = 1024, int initialIndexSize = 1024);
  ~idHashIndex();
  idHashIndex(const idHashIndex&) = delete;
  idHashIndex& operator=(const idHashIndex&) = delete;

  void InternalInit(int initialHashSize, int initialIndexSize);
  void InternalFree();
  void InternalResizeIndex(int newIndexSize);
  int GetSpread() const;
  void InternalAllocate(int newHashSize, int newIndexSize);

  int First(int key) const { return hash == INTERNAL_INVALID_INDEX ? -1 : hash[key & hashMask]; }
  int Next(int index) const {
    return indexChain == INTERNAL_INVALID_INDEX ? -1 : indexChain[index & lookupMask];
  }
  void Add(int key, int index) {
    if (hash == INTERNAL_INVALID_INDEX)
      InternalAllocate(hashSize, indexSize);
    if (index >= indexSize)
      InternalResizeIndex(index + 1);
    const int bucket = key & hashMask;
    indexChain[index] = hash[bucket];
    hash[bucket] = index;
  }
  void Remove(int key, int index) {
    if (hash == INTERNAL_INVALID_INDEX)
      return;
    const int bucket = key & hashMask;
    if (hash[bucket] == index) {
      hash[bucket] = indexChain[index & lookupMask];
    } else {
      for (int current = hash[bucket]; current >= 0; current = indexChain[current & lookupMask]) {
        if (indexChain[current & lookupMask] == index) {
          indexChain[current & lookupMask] = indexChain[index & lookupMask];
          break;
        }
      }
    }
    indexChain[index & lookupMask] = -1;
  }
  void RemoveIndex(int key, int index);
  void Clear() {
    if (hash != INTERNAL_INVALID_INDEX) {
      for (int index = 0; index < hashSize; ++index)
        hash[index] = -1;
      for (int index = 0; index < indexSize; ++index)
        indexChain[index] = -1;
    }
  }
};

static_assert(sizeof(idHashIndex) == 40, "idHashIndex ABI drift");
