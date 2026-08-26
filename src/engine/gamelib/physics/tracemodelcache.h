#pragma once

#include "engine/cm/collisionmodel.h"
#include "shared/idlib/containers/list.h"

#include <cstddef>
#include <cstdint>

struct traceModelWater_t {
  idVec3 xyz;
  float weight;
};

class idTraceModelCache {
public:
  struct trmCache_t {
    const void* trm;
    int refCount;
    float volume;
    idVec3 centerOfMass;
    idMat3 inertiaTensor;
    const void* material;
    idCollisionModel* collisionModel;
    bool hasWater;
    std::uint8_t recoveredPadding[7];
    idList<traceModelWater_t, 54> waterPoints;
  };

  std::uint8_t recoveredPrefix[0x48];
  idList<trmCache_t*, 13> cache;
  std::uint8_t recoveredTail[0x28];
};

static_assert(sizeof(traceModelWater_t) == 0x10,
              "trace-model water point ABI drift");
static_assert(offsetof(traceModelWater_t, weight) == 0x0C,
              "trace-model water weight offset drift");
static_assert(sizeof(idTraceModelCache::trmCache_t) == 0x70,
              "trace-model cache entry ABI drift");
static_assert(offsetof(idTraceModelCache::trmCache_t, volume) == 0x0C,
              "trace-model volume offset drift");
static_assert(offsetof(idTraceModelCache::trmCache_t, collisionModel) == 0x48,
              "trace-model collision offset drift");
static_assert(offsetof(idTraceModelCache::trmCache_t, hasWater) == 0x50,
              "trace-model water flag offset drift");
static_assert(offsetof(idTraceModelCache::trmCache_t, waterPoints) == 0x58,
              "trace-model water-list offset drift");
static_assert(sizeof(idTraceModelCache) == 0x88,
              "idTraceModelCache ABI drift");
static_assert(offsetof(idTraceModelCache, cache) == 0x48,
              "idTraceModelCache list offset drift");
