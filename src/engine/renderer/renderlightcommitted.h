#pragma once

#include "engine/renderer/parmblock.h"
#include "engine/renderer/renderindex.h"
#include "engine/renderer/renderlightparms.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/geometry/rendermatrix.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idMaterial;
class idRenderWorldCommitted;
struct worldArea_t;

struct alignas(16) idRenderLightCommitted {
  bool CullModelBounds(const idRenderMatrix& modelMatrix,
                       const idBounds& localBounds) const;

  const char* name = nullptr;
  idRenderWorldCommitted* world = nullptr;
  idRenderIndex index{};
  std::uint32_t recoveredPad20{};
  idRenderLightParms r{};
  std::uint8_t recoveredPad280[8]{};
  idStaticParmBlock<32> renderParmBlock;
  idList<worldArea_t*, 62> references;
  bool needsReferences = false;
  std::uint8_t recoveredPad1129[3]{};
  idBounds globalReferenceBounds{};
  idVec3 globalLightOrigin{};
  idVec3 currentColor{};
  idVec4 maxVisibleRange{};
  idRenderMatrix baseLightProject{};
  idRenderMatrix inverseBaseLightProject{};
  const idMaterial* lightMtr = nullptr;
  bool castsShadows = false;
  std::uint8_t recoveredTail[7]{};
};

static_assert(offsetof(idRenderLightCommitted, r) == 0x18,
              "committed-light parms offset drift");
static_assert(offsetof(idRenderLightCommitted, renderParmBlock) == 0x120,
              "committed-light parm-block offset drift");
static_assert(offsetof(idRenderLightCommitted, inverseBaseLightProject) == 0x4EC,
              "committed-light inverse-project offset drift");
static_assert(offsetof(idRenderLightCommitted, lightMtr) == 0x530,
              "committed-light material offset drift");
static_assert(sizeof(idRenderLightCommitted) == 0x540,
              "committed-light ABI drift");
