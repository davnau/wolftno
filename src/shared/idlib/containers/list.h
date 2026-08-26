#pragma once

#include "shared/idlib/sys/sys_alloc.h"

#include <algorithm>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

class idMD6MorphSet;

template <typename T, typename = void>
struct idListAllocReturnsReference : std::false_type {};

// Some retail idList instantiations expose Alloc as T&.  A nested marker lets
// a type request that ABI before the containing list instantiates, including
// for nested types whose enclosing class is not complete soon enough for an
// ordinary explicit specialization.
template <typename T>
struct idListAllocReturnsReference<
    T, std::void_t<typename T::idListAllocReturnsReferenceMarker>>
    : std::true_type {};

template <typename T, int DefaultTag = 5> class idList {
public:
  T* list;
  int num;
  int size;
  std::int16_t granularity;
  std::uint8_t memTag;
  std::uint8_t listStatic;

  idList() : list(nullptr) {
    // Retail initializes every list through the engine allocator boundary,
    // even when the initial allocation is empty.
    idMem::Singleton();
    granularity = 0;
    memTag = DefaultTag;
    listStatic = 0;
    num = 0;
    size = 0;
    ClearFree();
  }
  idList(const idList& other) : idList() { *this = other; }
  idList& operator=(const idList& other) {
    if (this == &other)
      return *this;
    SetNum(other.num);
    for (int i = 0; i < num; ++i)
      list[i] = other.list[i];
    return *this;
  }
  ~idList() { ClearFree(); }

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
  template <int OtherTag> int Append(const idList<T, OtherTag>& other) {
    if (other.num <= 0)
      return num;
    EnsureCapacity(num + other.num);
    if (num + other.num > size)
      return -1;
    for (int i = 0; i < other.num; ++i)
      list[num++] = other.list[i];
    return num;
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
  using AllocResult = std::conditional_t<
      DefaultTag == 85 || std::is_same_v<T, idMD6MorphSet> ||
          idListAllocReturnsReference<T>::value,
      T&, T*>;
  AllocResult Alloc() {
    EnsureCapacity(num + 1);
    const int index = num >= size ? size - 1 : num++;
    if constexpr (DefaultTag == 85 || std::is_same_v<T, idMD6MorphSet> ||
                  idListAllocReturnsReference<T>::value)
      return list[index];
    else
      return index >= 0 ? &list[index] : nullptr;
  }
  void Clear() { num = 0; }
  void ClearFree() {
    if (listStatic == 0 || listStatic == 2) {
      if (list != nullptr) {
        if constexpr (!std::is_trivially_destructible_v<T>) {
          for (int index = 0; index < size; ++index)
            list[index].~T();
        }
        idMem::Singleton()->Free(list, ALIGN_16);
      }
      list = nullptr;
      size = 0;
    }
    num = 0;
  }
  template <typename U = T>
  void DeleteContents() {
    for (int index = 0; index < num; ++index) {
      delete list[index];
      list[index] = nullptr;
    }
    ClearFree();
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
#if defined(WOLFTNO_RETAIL_IDMEM_SINGLETON_ABI)
    idMem* memory = &idMem::Singleton();
#else
    idMem* memory = idMem::Singleton();
#endif
    T* replacement = static_cast<T*>(memory->AllocWithLocation(
        "w:\\build\\tech5\\shared\\idlib\\containers/List.h(29) : tag",
        sizeof(T) * static_cast<std::size_t>(newSize),
        static_cast<memTag_t>(memTag), false, ALIGN_16, HEAP_DEFAULTHEAP));
    if (replacement == nullptr)
      return;
    int constructed = 0;
    try {
      for (; constructed < newSize; ++constructed)
        ::new (static_cast<void*>(&replacement[constructed])) T;
    } catch (...) {
      while (constructed > 0)
        replacement[--constructed].~T();
      memory->Free(replacement, ALIGN_16);
      throw;
    }
    for (int i = 0; i < num; ++i)
      replacement[i] = std::move(list[i]);
    if ((listStatic == 0 || listStatic == 2) && list != nullptr) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (int index = 0; index < size; ++index)
          list[index].~T();
      }
      idMem::Singleton()->Free(list, ALIGN_16);
    }
    list = replacement;
    size = newSize;
    listStatic = 0;
  }
  __forceinline bool ResizeForPort(int newSize) { return Resize(newSize); }

private:
  // Recovered compilands explicitly instantiate Resize for selected
  // element/tag combinations; private access is part of its MSVC ABI name.
  bool Resize(int newSize) {
    if (newSize <= 0) {
      ClearFree();
      return true;
    }
    if (newSize == size || listStatic != 0) {
      return true;
    }
#if defined(WOLFTNO_RETAIL_IDMEM_SINGLETON_ABI)
    idMem* memory = &idMem::Singleton();
#else
    idMem* memory = idMem::Singleton();
#endif
    T* replacement = static_cast<T*>(memory->AllocWithLocation(
        "w:\\build\\tech5\\shared\\idlib\\containers/List.h(29) : tag",
        sizeof(T) * static_cast<std::size_t>(newSize),
        static_cast<memTag_t>(memTag), false, ALIGN_16, HEAP_DEFAULTHEAP));
    if (replacement == nullptr)
      return false;
    int constructed = 0;
    try {
      for (; constructed < newSize; ++constructed)
        ::new (static_cast<void*>(&replacement[constructed])) T;
    } catch (...) {
      while (constructed > 0)
        replacement[--constructed].~T();
      memory->Free(replacement, ALIGN_16);
      throw;
    }
    const int copyCount = std::min(num, newSize);
    for (int i = 0; i < copyCount; ++i)
      replacement[i] = std::move(list[i]);
    if ((listStatic == 0 || listStatic == 2) && list != nullptr) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (int index = 0; index < size; ++index)
          list[index].~T();
      }
      idMem::Singleton()->Free(list, ALIGN_16);
    }
    list = replacement;
    size = newSize;
    num = copyCount;
    listStatic = 0;
    return true;
  }
};

static_assert(sizeof(idList<int>) == 24, "idList ABI drift");
