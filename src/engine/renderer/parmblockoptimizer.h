#pragma once

#include "shared/idlib/containers/list.h"

#include <cstddef>
#include <cstdint>

enum expOpType_t : int {
  OP_TYPE_UNMASKED_MOVE_CONSTANT = 0,
  OP_TYPE_UNMASKED_MOVE = 1,
  OP_TYPE_MOVE_CONSTANT = 2,
  OP_TYPE_MOVE = 3,
  OP_TYPE_SWIZZLE = 4,
  OP_TYPE_ADD = 5,
  OP_TYPE_SUBTRACT = 6,
  OP_TYPE_MULTIPLY = 7,
  OP_TYPE_DOT3 = 8,
  OP_TYPE_DOT4 = 9,
  OP_TYPE_DIVIDE = 10,
  OP_TYPE_MOD = 11,
  OP_TYPE_GT = 12,
  OP_TYPE_GE = 13,
  OP_TYPE_LT = 14,
  OP_TYPE_LE = 15,
  OP_TYPE_EQ = 16,
  OP_TYPE_NE = 17,
  OP_TYPE_AND = 18,
  OP_TYPE_OR = 19,
  OP_TYPE_TABLE = 20,
};

struct expOp_t {
  std::uint16_t type;
  std::int16_t parmIndexDest;
  std::int16_t parmIndexA;
  std::int16_t parmIndexB;
};

union parmValue_t {
  float value[4];
  const void* pointer;
  int swizzle[4];
};

struct idParmBlock {
  idList<expOp_t, 56> ops;
  idList<parmValue_t, 56> constants;
};

struct idParmBlockOptimizer {
  int usedParms[200];
  int numUsedParms;

  static int c_retainedOps;
  static int c_removedOps;

  void OptimizeParmBlockForUsedParms(idParmBlock& parmBlock);
};

static_assert(sizeof(expOp_t) == 8, "expOp_t ABI drift");
static_assert(sizeof(parmValue_t) == 16, "parmValue_t ABI drift");
static_assert(offsetof(idParmBlock, constants) == 0x18,
              "render parameter constant-list offset drift");
static_assert(offsetof(idParmBlockOptimizer, numUsedParms) == 0x320,
              "parameter optimizer used-count offset drift");
static_assert(sizeof(idParmBlockOptimizer) == 0x324,
              "idParmBlockOptimizer ABI drift");
