#pragma once

#include "engine/models/rendermodel.h"
#include "engine/renderer/approximatelighting.h"
#include "engine/renderer/parmblock.h"
#include "engine/renderer/rendermodelparms.h"
#include "engine/renderer/renderindex.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/rendermatrix.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idRenderWorldCommitted;
struct worldArea_t;

struct alignas(16) idRenderModelCommitted {
  const char* name;
  idRenderWorldCommitted* world;
  idRenderIndex index;
  idBounds referenceBounds;
  std::uint8_t alignmentBeforeParms[4];
  idRenderModelParms r;
  std::uint8_t alignmentBeforeParmBlock[8];
  idStaticParmBlock<32> renderParmBlock;
  idList<idRenderModelSurface, 62> surfaces;
  idList<worldArea_t*, 62> references;
  idRenderModelCommitted* nextAddAlwaysModel;
  idRenderModelCommitted* nextDimShadowModel;
  idRenderModelCommitted* nextApproxLightModel;
  idRenderModelCommitted* referenceModel;
  int guiFrameCount;
  idVec4 maxVisibleRange;
  idBounds globalReferenceBounds;
  idVec3 localLightingPoint;
  float localLightingRadius;
  idRenderMatrix modelMatrix;
  idRenderMatrix inverseModelMatrix;
  approximateLighting_t approximateLighting[2];
  std::uint32_t approximateState;
  std::uint8_t committedFlags0;
  std::uint8_t committedFlags1;
  std::uint8_t alignmentBeforeMvp[2];
  idRenderMatrix mvpMatrix;
  idVec4 highlightColor;
  std::uint8_t alignmentTail[12];

  int CurrentApproximateLighting() const {
    return static_cast<int>(approximateState & 1u);
  }
};

static_assert(offsetof(idRenderModelCommitted, index) == 0x10,
              "committed-model index offset drift");
static_assert(offsetof(idRenderModelCommitted, referenceBounds) == 0x14,
              "committed-model reference-bounds offset drift");
static_assert(offsetof(idRenderModelCommitted, renderParmBlock) == 0x110,
              "committed-model parm-block offset drift");
static_assert(offsetof(idRenderModelCommitted, surfaces) == 0x440,
              "committed-model surfaces offset drift");
static_assert(offsetof(idRenderModelCommitted, modelMatrix) == 0x4CC,
              "committed-model matrix offset drift");
static_assert(offsetof(idRenderModelCommitted, inverseModelMatrix) == 0x50C,
              "committed-model inverse-matrix offset drift");
static_assert(offsetof(idRenderModelCommitted, approximateLighting) == 0x54C,
              "committed-model approximate-lighting offset drift");
static_assert(offsetof(idRenderModelCommitted, committedFlags0) == 0x710,
              "committed-model primary flags offset drift");
static_assert(offsetof(idRenderModelCommitted, committedFlags1) == 0x711,
              "committed-model secondary flags offset drift");
static_assert(offsetof(idRenderModelCommitted, mvpMatrix) == 0x714,
              "committed-model MVP offset drift");
static_assert(offsetof(idRenderModelCommitted, highlightColor) == 0x754,
              "committed-model highlight offset drift");
static_assert(sizeof(idRenderModelCommitted) == 0x770,
              "committed-model ABI drift");
