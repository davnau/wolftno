#pragma once

#include <cstddef>

class idTextureSampler {
public:
  unsigned int oglSamplerState;
};

struct idTextureSamplerManagerVTable {
  void (*destroy)(struct idTextureSamplerManager* manager);
  void (*Init)(struct idTextureSamplerManager* manager);
  void (*Shutdown)(struct idTextureSamplerManager* manager);
  const idTextureSampler* (*FindSamplerByName)(
      struct idTextureSamplerManager* manager, const char* name,
      bool createIfMissing);
  const char* (*GetSamplerName)(struct idTextureSamplerManager* manager,
                               const idTextureSampler* sampler);
  const idTextureSampler* (*CreateSampler)(
      struct idTextureSamplerManager* manager, const void* options,
      const char* name);
  void (*DeleteSampler)(struct idTextureSamplerManager* manager,
                        const idTextureSampler* sampler);
};

struct idTextureSamplerManager {
  const idTextureSamplerManagerVTable* vtable;
  const idTextureSampler* pointSampler;
  const idTextureSampler* pointSamplerRepeating;
  const idTextureSampler* mipMapSamplers[16];

  const idTextureSampler* FindSamplerByName(const char* name,
                                            bool createIfMissing) {
    return vtable != nullptr && vtable->FindSamplerByName != nullptr
               ? vtable->FindSamplerByName(this, name, createIfMissing)
               : nullptr;
  }
  const char* GetSamplerName(const idTextureSampler* sampler) {
    return vtable != nullptr && vtable->GetSamplerName != nullptr
               ? vtable->GetSamplerName(this, sampler)
               : nullptr;
  }
  const idTextureSampler* CreateSampler(const void* options,
                                        const char* name) {
    return vtable != nullptr && vtable->CreateSampler != nullptr
               ? vtable->CreateSampler(this, options, name)
               : nullptr;
  }
  void DeleteSampler(const idTextureSampler* sampler) {
    if (vtable != nullptr && vtable->DeleteSampler != nullptr) {
      vtable->DeleteSampler(this, sampler);
    }
  }
};

extern idTextureSamplerManager* textureSamplerManager;

static_assert(offsetof(idTextureSamplerManager, mipMapSamplers) == 0x18,
              "texture-sampler manager mip table offset drift");
static_assert(sizeof(idTextureSamplerManager) == 0x98,
              "texture-sampler manager ABI drift");
static_assert(sizeof(idTextureSampler) == 4,
              "texture-sampler ABI drift");
