#pragma once

#include "engine/models/rendermodel.h"
#include "engine/models/static/jobs/statictransparencygen.h"
#include "engine/models/transparency/jobs/influencespherecull.h"

#include <cstddef>
#include <cstdint>

class idParallelJobList;

struct updateInViewStats_t {
  bool showStats{};
  std::uint8_t recoveredPadding[3]{};
  int c_numParticleVertices{};
};

class idRenderModelTransparency : public idRenderModel {
public:
  static bool AllocateQuadSegment(int numQuads,
                                  idTransparencyVert*& verts,
                                  float*& quadDepths, int*& quadsUsed,
                                  bool hasEmissivePass);

  std::uint8_t recoveredTransparencyState[48]{};
  visibleInfluenceSpheres_t* visibleInfluenceSpheres{};
  void* influenceSphereCullParms{};
};

class idRenderModelUpdateTools {
public:
  void ApproximateLightingForBounds(idRenderModel* model,
                                    const idBounds* bounds,
                                    approximateLighting_t* lighting);
  void GetCurrentResolutionScale(float* x, float* y);

  idParallelJobList* renderModelJobList{};
  idRenderModelTransparency* transparencyModel{};
  const void* globalShadowMap{};
  updateInViewStats_t pc{};
};

static_assert(sizeof(updateInViewStats_t) == 8,
              "update-in-view stats ABI drift");
static_assert(offsetof(idRenderModelTransparency, visibleInfluenceSpheres) ==
                  2224,
              "idRenderModelTransparency visible spheres offset drift");
static_assert(sizeof(idRenderModelTransparency) == 2240,
              "idRenderModelTransparency ABI drift");
static_assert(sizeof(idRenderModelUpdateTools) == 32,
              "idRenderModelUpdateTools ABI drift");
