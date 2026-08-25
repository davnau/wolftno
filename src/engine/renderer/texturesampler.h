#pragma once

#include <cstddef>

class idTextureSampler {
public:
  unsigned int oglSamplerState;
};

struct idTextureSamplerManager {
  void* vtable;
  const idTextureSampler* pointSampler;
  const idTextureSampler* pointSamplerRepeating;
  const idTextureSampler* mipMapSamplers[16];
};

extern idTextureSamplerManager* textureSamplerManager;

static_assert(offsetof(idTextureSamplerManager, mipMapSamplers) == 0x18,
              "texture-sampler manager mip table offset drift");
static_assert(sizeof(idTextureSamplerManager) == 0x98,
              "texture-sampler manager ABI drift");
static_assert(sizeof(idTextureSampler) == 4,
              "texture-sampler ABI drift");
