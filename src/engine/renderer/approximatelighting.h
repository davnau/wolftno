#pragma once

#include "shared/idlib/math/vector.h"

#include <cstddef>

struct approximateLighting_t {
  idVec4 lightingModel;
  idVec4 primeLightDir;
  idVec4 lightOrigin[3];
  idVec4 lightColor[3];
  idVec4 channelLighting[6];
};

static_assert(sizeof(approximateLighting_t) == 0xE0,
              "approximate-lighting ABI drift");
static_assert(offsetof(approximateLighting_t, primeLightDir) == 0x10,
              "approximate-lighting prime-light offset drift");
static_assert(offsetof(approximateLighting_t, lightColor) == 0x50,
              "approximate-lighting light-color offset drift");
static_assert(offsetof(approximateLighting_t, channelLighting) == 0x80,
              "approximate-lighting channel offset drift");
