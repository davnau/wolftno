#pragma once

#include <cstddef>

class idDeclParticle;
class idRenderModelParticle;
class idRenderWorld;

// ABI surface consumed by the impact manager.  The manager implementation is
// owned by the later gamelibeffects compiland; retaining its retail extent here
// lets enclosing game objects use the recovered layout in the meantime.
class idEffectsModelManager {
public:
  idEffectsModelManager();
  ~idEffectsModelManager();

  void Init(idRenderWorld* renderWorld);
  void Shutdown();
  idRenderModelParticle*
  GetNextParticleEffectModel(const idDeclParticle* particleDecl);
  void RecycleParticleFxModel(const idDeclParticle* particleDecl,
                              idRenderModelParticle* model);

  idRenderWorld* world;
  std::byte recoveredState[6504];
};

static_assert(sizeof(idEffectsModelManager) == 6512,
              "idEffectsModelManager ABI drift");
static_assert(offsetof(idEffectsModelManager, world) == 0,
              "idEffectsModelManager world offset drift");
