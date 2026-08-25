#pragma once

#include "shared/idlib/geometry/rendermatrix.h"

#include <cstddef>
#include <cstdint>

class idCVar;

struct shadowMapCPU_t {
  int width;
  std::uint16_t* map;
  idRenderMatrix mvp;
  float coordinateBias;
  float depthBias;
  bool useShadowMap;
};

class idGlobalShadows {
public:
  idGlobalShadows();
  ~idGlobalShadows();

  void LoadGlobalShadows(const char* mapName);
  const shadowMapCPU_t* GetShadowMap();

  shadowMapCPU_t* shadowMap;
};

extern idCVar r_globalShadowsCoordinateBias;
extern idCVar r_globalShadowsDepthBias;
extern idCVar r_useGlobalShadows;

static_assert(offsetof(shadowMapCPU_t, map) == 0x08,
              "global shadow map pointer offset drift");
static_assert(offsetof(shadowMapCPU_t, mvp) == 0x10,
              "global shadow MVP offset drift");
static_assert(offsetof(shadowMapCPU_t, coordinateBias) == 0x50,
              "global shadow coordinate-bias offset drift");
static_assert(offsetof(shadowMapCPU_t, depthBias) == 0x54,
              "global shadow depth-bias offset drift");
static_assert(offsetof(shadowMapCPU_t, useShadowMap) == 0x58,
              "global shadow enable offset drift");
static_assert(sizeof(shadowMapCPU_t) == 0x60,
              "global shadow map ABI drift");
static_assert(sizeof(idGlobalShadows) == sizeof(void*),
              "global-shadows ABI drift");
