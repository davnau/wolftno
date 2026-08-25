#pragma once

#include <cstddef>

template <typename T> struct idHeapArray {
  T* buffer{};
  unsigned int num{};
  bool bufferOwnedByObject{};
};

static_assert(sizeof(idHeapArray<unsigned char>) == 0x10,
              "idHeapArray ABI drift");
