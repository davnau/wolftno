#pragma once

#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/rendermatrix.h"

#include <cstddef>
#include <cstdint>

class idDeclEnv;

struct idScreenRect {
  int x1;
  int y1;
  int x2;
  int y2;
};

struct envBlend_t {
  const idDeclEnv* env;
  float blendStartTime;
  float blendDuration;
  int frameExposureFinished;
  bool cubeFinished;
  bool colorCorrectionFinished;
  bool hasOverrideParms;
  bool skipBlending;
};

// NEED WORK: render.cpp owns the fields represented by these opaque ranges.
// The exposed members and offsets below are recovered from the retail PDB and
// RenderCull assembly so renderer jobs can share the correct ABI now.
struct alignas(16) renderSettings_t {
  std::uint8_t opaquePrefix[0x150];
  idBounds viewBounds;
  std::uint8_t opaqueBetweenBounds[8];
  idBounds detailBounds;
  std::uint8_t opaqueTail[8];

  bool UseAmbientEnv() const { return opaquePrefix[0x4E] != 0; }
};

// NEED WORK: the complete render-view declaration is deferred until render.h
// itself is reconstructed.  Named fields are the retail offsets consumed by
// the currently recovered renderer jobs.
struct alignas(16) idRenderView {
  std::uint8_t opaqueBeforeRenderTime[0x1A40];
  int renderTime;
  std::uint8_t opaqueBeforeRenderFlags[0x0B];
  bool inhibitModelFovScale;
  std::uint8_t opaqueBeforeFovY[4];
  float fovY;
  bool cramZNear;
  std::uint8_t opaqueBeforeMatrices[0x1967];
  idRenderMatrix projectionMatrix;
  idRenderMatrix inverseProjectionMatrix;
  idRenderMatrix viewMatrix;
  idRenderMatrix inverseViewMatrix;
  idRenderMatrix worldSpaceMVPMatrix;
  idRenderMatrix worldSpaceInverseMVPMatrix;
  idRenderMatrix viewSpaceModelMatrix;
  idRenderMatrix viewSpaceInverseModelMatrix;
  idRenderMatrix viewSpaceMVPMatrix;
  int windowWidth;
  int windowHeight;
  int renderWidth;
  int renderHeight;
  int feedbackWidth;
  int feedbackHeight;
  idScreenRect viewport;
  idScreenRect scissorRect;
  idScreenRect testScissor;
  std::int64_t firstOcclusionQueryInPreviousFrame;
  void* occlusionState;
};

// Recovered prefix of idRenderModelParms.  Later fields are deferred with the
// rest of render.h; SetupModelMVPMatrix consumes this entire prefix.
struct idRenderModelParms {
  idVec3 origin;
  idMat3 axis;
  idVec3 scale;
  std::uint8_t alignmentPadding[4];
  const void* customMaterial;
  int customSkin;
  int groupMasterIndex;
  int lightGroupID;
  int allowSurfaceOnlyInViewID;
  int suppressSurfaceInViewID;
  int suppressShadowInViewID;
  int suppressShadowInLightIndex;
  int firstVisibleFrameCount;
  float fovScale;
  float modelDepthHack;
  int castDimShadows;
  std::uint8_t flags0;
  std::uint8_t flags1;
  std::uint8_t flags2;
  std::uint8_t flags3;

  bool MvpIsIdentity() const { return (flags1 & 0x80u) != 0; }
  bool MvpIsOrthographic() const { return (flags2 & 0x01u) != 0; }
};

static_assert(offsetof(renderSettings_t, viewBounds) == 0x150,
              "render-settings view bounds offset drift");
static_assert(offsetof(renderSettings_t, detailBounds) == 0x170,
              "render-settings detail bounds offset drift");
static_assert(sizeof(renderSettings_t) == 0x190,
              "render-settings recovered ABI prefix drift");
static_assert(sizeof(envBlend_t) == 24, "environment blend ABI drift");
static_assert(offsetof(idRenderView, worldSpaceMVPMatrix) == 0x34C0,
              "render-view world MVP offset drift");
static_assert(offsetof(idRenderView, projectionMatrix) == 0x33C0,
              "render-view projection offset drift");
static_assert(offsetof(idRenderView, viewMatrix) == 0x3440,
              "render-view view-matrix offset drift");
static_assert(offsetof(idRenderView, windowWidth) == 0x3600,
              "render-view dimensions offset drift");
static_assert(offsetof(idRenderView, viewport) == 0x3618,
              "render-view viewport offset drift");
static_assert(offsetof(idRenderView, scissorRect) == 0x3628,
              "render-view scissor offset drift");
static_assert(sizeof(idRenderView) == 0x3660,
              "render-view recovered ABI prefix drift");
static_assert(offsetof(idRenderView, renderTime) == 0x1A40,
              "render-view render-time offset drift");
static_assert(offsetof(idRenderModelParms, fovScale) == 0x68,
              "render-model fov scale offset drift");
static_assert(offsetof(idRenderModelParms, flags1) == 0x75,
              "render-model identity flags offset drift");
static_assert(sizeof(idRenderModelParms) == 0x78,
              "render-model parameters recovered prefix drift");
