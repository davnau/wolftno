#pragma once

#include "parmblock.h"
#include "shared/idlib/containers/BitArray.h"

#include <cstddef>

// NEEDS_WORK(engine/renderer): ABI-only parser interface recovered for decl
// consumers. Its expression parser belongs to renderer/parmblockparser.cpp and
// is intentionally deferred until that folder is reconstructed.
class idParser;

class idParmBlockParser {
public:
  idParmBlockParser(idParser& source, idParmBlock& block);
  ~idParmBlockParser();

  idBitArray usedTempParms;
  idList<expOp_t, 56>* ops;
  idList<parmValue_t, 56>* constants;
  parmValue_t defaultConstant;
};

static_assert(sizeof(idParmBlockParser) == 56, "idParmBlockParser ABI drift");
