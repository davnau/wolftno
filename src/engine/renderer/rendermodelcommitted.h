#pragma once

#include "engine/renderer/parmblock.h"

#include <cstddef>
#include <cstdint>

// NEED WORK: fields surrounding this block belong to rendermodelcommitted.cpp.
struct alignas(16) idRenderModelCommitted {
  std::uint8_t opaquePrefix[0x110];
  idStaticParmBlock<32> renderParmBlock;
};

static_assert(offsetof(idRenderModelCommitted, renderParmBlock) == 0x110,
              "committed-model parm-block offset drift");

