#pragma once

#include "shared/idlib/containers/list.h"
#include "shared/idlib/text/atomicstring.h"

#include <cstdint>

class idAnimWebScalar;
struct idMD6Node;
struct idScalarMemBlock {
  idScalarMemBlock() : floats(nullptr), num(0) {}
  ~idScalarMemBlock() { Free(); }
  void Alloc(int count);
  void Free();
  void Free(idList<idAnimWebScalar>& scalars);
  void Free(idList<idAnimWebScalar,33>& scalars);
  float* floats;
  int num;
};

class idAnimWebScalar {
public:
  enum flags_t : std::uint8_t { DECLARED = 1, READ_ONLY = 2 };
  idAnimWebScalar() : flags(0), scalarIndex(-1), scalarPtr(nullptr) {}
  void Init(const char* name, std::uint8_t flags, float* scalar = nullptr);
  float GetScalar(const idScalarMemBlock* block) const;
  void SetScalar(idScalarMemBlock* block, float value);
  void SetScalarPtr(float* scalar);
  void SetIndex(const idScalarMemBlock* block, std::int16_t index);
  idAtomicString name;
  std::uint8_t flags;
  std::int16_t scalarIndex;
  float* scalarPtr;
};
static_assert(sizeof(idAnimWebScalar)==24,"AnimWeb scalar ABI drift");
struct idAnimWebScalarPair {
  int scalarIndex=-1;
  int coordinateIndex=-1;
  idMD6Node* node=nullptr;
  std::uint8_t fieldType=0;
  std::uint8_t fieldFlags=0;
  std::uint8_t data1=0;
  std::uint8_t data2=0;
};
static_assert(sizeof(idAnimWebScalarPair)==24,"AnimWeb scalar-pair ABI drift");
