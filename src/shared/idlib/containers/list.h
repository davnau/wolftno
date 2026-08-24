#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>

template <typename T, int DefaultTag = 5> class idList {
public:
  T* list;
  int num;
  int size;
  std::int16_t granularity;
  std::uint8_t memTag;
  std::uint8_t listStatic;

  idList() : list(nullptr), num(0), size(0), granularity(0), memTag(DefaultTag), listStatic(0) {}
  idList(const idList& other) : idList() { *this = other; }
  idList& operator=(const idList& other) {
    if (this == &other)
      return *this;
    SetNum(other.num);
    for (int i = 0; i < num; ++i)
      list[i] = other.list[i];
    return *this;
  }
  ~idList() {
    if (listStatic == 0 || listStatic == 2)
      delete[] list;
  }

  int Num() const { return num; }
  bool IsFull() const { return listStatic != 0 && num >= size; }
  T& operator[](int index) { return list[index]; }
  const T& operator[](int index) const { return list[index]; }
  int Append(const T& value) {
    EnsureCapacity(num + 1);
    if (num >= size)
      return num - 1;
    list[num] = value;
    return num++;
  }
  int Insert(const T& value, int index) {
    index = std::max(0, std::min(index, num));
    EnsureCapacity(num + 1);
    for (int i = num; i > index; --i)
      list[i] = std::move(list[i - 1]);
    list[index] = value;
    ++num;
    return index;
  }
  bool RemoveIndex(int index) {
    if (index < 0 || index >= num)
      return false;
    for (int i = index + 1; i < num; ++i)
      list[i - 1] = std::move(list[i]);
    --num;
    return true;
  }
  bool RemoveFast(const T& value) {
    for (int index = 0; index < num; ++index) {
      if (list[index] == value) {
        list[index] = std::move(list[num - 1]);
        --num;
        return true;
      }
    }
    return false;
  }
  bool RemoveIndexFast(int index) {
    if (index < 0 || index >= num)
      return false;
    if (index != num - 1)
      list[index] = std::move(list[num - 1]);
    --num;
    return true;
  }
  T* Alloc() {
    EnsureCapacity(num + 1);
    if (num >= size)
      return nullptr;
    return &list[num++];
  }
  void Clear() { num = 0; }
  void ClearFree() {
    if (listStatic == 0 || listStatic == 2) {
      delete[] list;
      list = nullptr;
      size = 0;
    }
    num = 0;
  }
  bool SetNum(int count) {
    if (count > size && !Resize(count))
      return false;
    num = std::min(count, size);
    return true;
  }
  bool SetNum(int count, const T& initialValue) {
    const int oldNum = num;
    EnsureCapacity(count);
    for (int i = oldNum; i < count; ++i)
      list[i] = initialValue;
    num = count;
    return true;
  }
  void EnsureCapacity(int required) {
    if (required <= size)
      return;
    if (listStatic != 0)
      return;
    int newSize = size;
    if (newSize <= 0)
      newSize = granularity > 0 ? granularity : 1;
    while (newSize < required) {
      if (granularity > 0)
        newSize += granularity;
      else if (newSize <= 0x3fffffff)
        newSize *= 2;
      else {
        newSize = required;
        break;
      }
    }
    T* replacement = new T[newSize];
    for (int i = 0; i < num; ++i)
      replacement[i] = std::move(list[i]);
    if (listStatic == 0 || listStatic == 2)
      delete[] list;
    list = replacement;
    size = newSize;
    listStatic = 0;
  }
  __forceinline bool ResizeForPort(int newSize) { return Resize(newSize); }

private:
  bool Resize(int newSize) {
    if (newSize <= 0) {
      ClearFree();
      return true;
    }
    if (newSize == size || listStatic != 0) {
      return true;
    }
    T* replacement = new T[newSize];
    const int copyCount = std::min(num, newSize);
    for (int i = 0; i < copyCount; ++i)
      replacement[i] = std::move(list[i]);
    delete[] list;
    list = replacement;
    size = newSize;
    num = copyCount;
    listStatic = 0;
    return true;
  }
};

static_assert(sizeof(idList<int>) == 24, "idList ABI drift");
