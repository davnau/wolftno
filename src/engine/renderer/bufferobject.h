#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

enum bufferUsageType_t : int {
  BU_STATIC = 0,
  BU_DYNAMIC = 1,
  BU_STREAM = 2,
};

struct idVertexBuffer {
  idVertexBuffer() {}
  idVertexBuffer(int size_, int offset_, void* object_, bufferUsageType_t usage_)
      : size(size_), offsetInOtherBuffer(offset_), apiObject(object_), usage(usage_) {}
  ~idVertexBuffer() { FreeBufferObject(); }

  int size{};
  int offsetInOtherBuffer{};
  void* apiObject{};
  bufferUsageType_t usage{};

  void Reference(const idVertexBuffer* other) {
    if (other == nullptr)
      return;
    size = other->size & 0x7fffffff;
    offsetInOtherBuffer = other->offsetInOtherBuffer & 0x7fffffff;
    apiObject = other->apiObject;
    usage = other->usage;
  }
  void AllocBufferObject(const void* data, int size_, unsigned,
                         bufferUsageType_t usage_) {
    FreeBufferObject();
    size = size_ | static_cast<int>(0x80000000U);
    offsetInOtherBuffer = 0;
    apiObject = size_ > 0 ? static_cast<void*>(new std::byte[size_]) : nullptr;
    usage = usage_;
    if (data != nullptr && apiObject != nullptr)
      std::memcpy(apiObject, data, static_cast<std::size_t>(size_));
  }
  void Update(const void* data, int size_, unsigned) {
    if (size_ > (size & 0x7fffffff)) {
      AllocBufferObject(data, size_, 0, usage);
      return;
    }
    if (data != nullptr && apiObject != nullptr && size_ > 0)
      std::memcpy(apiObject, data, static_cast<std::size_t>(size_));
  }
  void FreeBufferObject() {
    if ((static_cast<unsigned>(size) & 0x80000000U) != 0)
      delete[] static_cast<std::byte*>(apiObject);
    size = 0;
    offsetInOtherBuffer = 0;
    apiObject = nullptr;
  }
  void* MapBuffer(int) { return apiObject; }
  void UnmapBuffer() {}
  void Reference(const idVertexBuffer* other, int refOffset, int refSize) {
    if (other == nullptr)
      return;
    size = refSize;
    offsetInOtherBuffer = refOffset + (other->offsetInOtherBuffer & 0x7fffffff);
    apiObject = other->apiObject;
    usage = other->usage;
  }
};

struct idIndexBuffer {
  idIndexBuffer() {}
  idIndexBuffer(int size_, int offset_, void* object_, bufferUsageType_t usage_)
      : size(size_), offsetInOtherBuffer(offset_), apiObject(object_), usage(usage_) {}
  ~idIndexBuffer() { FreeBufferObject(); }

  int size{};
  int offsetInOtherBuffer{};
  void* apiObject{};
  bufferUsageType_t usage{};

  void Reference(const idIndexBuffer* other) {
    if (other == nullptr)
      return;
    size = other->size & 0x7fffffff;
    offsetInOtherBuffer = other->offsetInOtherBuffer & 0x7fffffff;
    apiObject = other->apiObject;
    usage = other->usage;
  }
  void AllocBufferObject(const void* data, int size_, bufferUsageType_t usage_) {
    FreeBufferObject();
    size = size_ | static_cast<int>(0x80000000U);
    offsetInOtherBuffer = 0;
    apiObject = size_ > 0 ? static_cast<void*>(new std::byte[size_]) : nullptr;
    usage = usage_;
    if (data != nullptr && apiObject != nullptr)
      std::memcpy(apiObject, data, static_cast<std::size_t>(size_));
  }
  void FreeBufferObject() {
    if ((static_cast<unsigned>(size) & 0x80000000U) != 0)
      delete[] static_cast<std::byte*>(apiObject);
    size = 0;
    offsetInOtherBuffer = 0;
    apiObject = nullptr;
  }
  void* MapBuffer(int) { return apiObject; }
  void UnmapBuffer() {}
  void Reference(const idIndexBuffer* other, int refOffset, int refSize) {
    if (other == nullptr)
      return;
    size = refSize;
    offsetInOtherBuffer = refOffset + (other->offsetInOtherBuffer & 0x7fffffff);
    apiObject = other->apiObject;
    usage = other->usage;
  }
};

static_assert(sizeof(idVertexBuffer) == 24, "idVertexBuffer ABI drift");
static_assert(sizeof(idIndexBuffer) == 24, "idIndexBuffer ABI drift");
