#pragma once

#include "engine/renderer/parmblock.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/color.h"
#include "shared/idlib/geometry/rendermatrix.h"

#include <cstddef>
#include <cstdint>

class idRenderModel;
class idRenderLight;
class idRenderModelUpdateTools;
class idRenderView;
class idTraceOpts;
struct idRenderModelSurface;

class idRenderDebug {
public:
  virtual ~idRenderDebug() = default;
  virtual void DebugPoint(const idColor& color, const idVec3& point,
                          int lifetime, bool depthTest) = 0;
};

struct approximateLighting_t {
  idVec4 lightingModel;
  idVec4 primeLightDir;
  idVec4 lightOrigin[3];
  idVec4 lightColor[3];
  idVec4 channelLighting[6];
};

struct localTrace_t {
  float fraction;
  idVec3 point;
  idVec3 normal;
  int jointIndex;
  int indexes[3];
};

struct modelTrace_t {
  localTrace_t tr;
  std::uint32_t padding;
  const idRenderModelSurface* surf;
  idRenderModel* entity;
};

struct renderView_t {
  int renderTime{};
  int viewID{};
  std::uint8_t recoveredFlags[8]{};
  float fov_x{};
  float fov_y{};
  bool cramZNear{};
  std::uint8_t recoveredProjectionPadding[3]{};
  idRenderMatrix explicitProjectionMatrix{};
  bool useExplicitProjectionMatrix{};
  std::uint8_t recoveredOriginPadding[3]{};
  idVec3 vieworg{};
  idMat3 viewaxis{};
  std::uint8_t recoveredTail[6384]{};
};

class idRenderWorldCommitted {
public:
  void ApproximateLightingForBounds(const idBounds* bounds,
                                    approximateLighting_t* lighting);
  void ApproximateAmbientForBounds(const idBounds* bounds,
                                   approximateLighting_t* lighting);
  void BoundsInAreas(const idBounds* bounds, int* areas, int* numAreas,
                     int maxAreas);
};

class idRenderWorld {
public:
  virtual ~idRenderWorld() = default;
  virtual idRenderDebug* GetRenderDebug() { return nullptr; }
};

class idRenderView {
public:
  renderView_t g{};
  std::uint8_t recoveredViewState[192]{};
  renderView_t r{};
  idRenderMatrix projectionMatrix{};
  idRenderMatrix inverseProjectionMatrix{};
  idRenderMatrix viewMatrix{};
  idRenderMatrix inverseViewMatrix{};
  idRenderMatrix worldSpaceMVPMatrix{};
  idRenderMatrix worldSpaceInverseMVPMatrix{};
  idRenderMatrix viewSpaceModelMatrix{};
  idRenderMatrix viewSpaceInverseModelMatrix{};
  idRenderMatrix viewSpaceMVPMatrix{};
  int windowWidth{};
  int windowHeight{};
  int renderWidth{};
  int renderHeight{};
  int feedbackWidth{};
  int feedbackHeight{};
  std::uint8_t recoveredTail[72]{};
};

class idRenderWorldLocal : public idRenderWorld {
public:
  void ApproximateLightingForBounds(const idBounds* bounds,
                                    approximateLighting_t* lighting);
  void ApproximateAmbientForBounds(const idBounds* bounds,
                                   approximateLighting_t* lighting);
  void BoundsInAreas(const idBounds* bounds, int* areas, int* numAreas,
                     int maxAreas);
  bool Trace(modelTrace_t* trace, const idVec3* start, const idVec3* end,
             const idTraceOpts* options);
  const idParmBlock* GetCurrentBlendedParms();
  void GetStaticWorldBounds(idBounds* bounds);
  void MarkLightSerialized(idRenderLight* light, bool changed, bool deleted);
  bool& CommitLight(int index) {
    return *reinterpret_cast<bool*>(reinterpret_cast<std::uint8_t*>(this) +
                                    8208 + index);
  }

  std::uint8_t recoveredPrefix[221552]{};
  idRenderWorldCommitted* committedWorld = nullptr;
};

static_assert(sizeof(approximateLighting_t) == 224,
              "approximateLighting_t ABI drift");
static_assert(offsetof(approximateLighting_t, primeLightDir) == 16,
              "approximateLighting_t prime light direction offset drift");
static_assert(offsetof(approximateLighting_t, lightColor) == 80,
              "approximateLighting_t light color offset drift");
static_assert(offsetof(approximateLighting_t, channelLighting) == 128,
              "approximateLighting_t channel lighting offset drift");
static_assert(sizeof(localTrace_t) == 44, "localTrace_t ABI drift");
static_assert(offsetof(modelTrace_t, surf) == 48,
              "modelTrace_t surface offset drift");
static_assert(sizeof(modelTrace_t) == 64, "modelTrace_t ABI drift");
static_assert(sizeof(renderView_t) == 6528, "renderView_t ABI drift");
static_assert(offsetof(renderView_t, vieworg) == 96,
              "renderView_t view origin offset drift");
static_assert(offsetof(renderView_t, viewaxis) == 108,
              "renderView_t view axis offset drift");
static_assert(offsetof(idRenderWorldLocal, committedWorld) == 221560,
              "idRenderWorldLocal committedWorld offset drift");
static_assert(sizeof(idRenderWorldLocal) == 221568,
              "idRenderWorldLocal ABI drift");
static_assert(offsetof(idRenderView, projectionMatrix) == 13248,
              "idRenderView projection offset drift");
static_assert(offsetof(idRenderView, renderWidth) == 13832,
              "idRenderView renderWidth offset drift");
static_assert(sizeof(idRenderView) == 13920, "idRenderView ABI drift");
