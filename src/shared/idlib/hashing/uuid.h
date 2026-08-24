#pragma once

#include "shared/idlib/text/str.h"

#include <array>
#include <cstdint>

class idUUID {
public:
  virtual ~idUUID() = default;
  int version;
  idStr hash;
  std::uint32_t generator;
  std::array<std::uint8_t, 16> data;

  idUUID();
  idUUID& operator=(const idUUID& other);
  void GenerateHash();
  void Nil();
  const char* ToString() const { return hash.data; }
};

class idUUID4 : public idUUID {
public:
  idUUID4();
  void SetData(const idStr& text);
  void Generate();
};
