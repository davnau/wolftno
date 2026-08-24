#pragma once

#include "parmblock.h"

#include <cstddef>

// NEEDS_WORK(engine/renderer): ABI-only parser interface recovered for decl
// consumers. Its expression parser belongs to renderer/parmblockparser.cpp and
// is intentionally deferred until that folder is reconstructed.
class idParser;

class idBitArray {
public:
  idBitArray() : buffer(nullptr), bits(0), memTag(0), free(false) {}
  ~idBitArray();

  unsigned char* buffer;
  std::size_t bits;
  short memTag;
  bool free;
};

class idParmBlockParser {
public:
  idParmBlockParser(idParser& source, idParmBlock& block);
  ~idParmBlockParser();

  idBitArray usedTempParms;
  idList<expOp_t, 56>* ops;
  idList<parmValue_t, 56>* constants;
  parmValue_t defaultConstant;
};

static_assert(sizeof(idBitArray) == 24, "idBitArray ABI drift");
static_assert(sizeof(idParmBlockParser) == 56, "idParmBlockParser ABI drift");
