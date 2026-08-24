#pragma once

#include "engine/renderer/rendermodelparms.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/rendermatrix.h"

#include <cstddef>
#include <cstdint>

class idDeclEnv;
class idDeclRenderProg;

extern const idRenderMatrix renderMatrix_windowSpaceToClipSpace;

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
  std::uint8_t featureFlags;
  bool skipWorld;
  bool skipModels;
  bool skipLights;
  bool skipBlendLightShadows;
  bool skipSuppress;
  bool skipFeedback;
  bool skipDynamic;
  bool skipAddAlways;
  bool skipOpaqueSurfaces;
  bool skipSubSurfaceScattering;
  bool skipAutosprites;
  bool skipOcclusionBaseModel;
  bool skipBlendedSurfaces;
  bool skipDistortionSurfaces;
  bool skipEmissiveSurfaces;
  bool skipOcclusionSurfaces;
  bool skipGuis;
  bool skipInGameGuis;
  bool skipAdaptiveGlare;
  bool skipPostProcess;
  bool skipEmissiveGlare;
  bool skipBlendLights;
  bool skipSlowLights;
  bool skipSelfShadows;
  bool skipAmbientOcclusion;
  bool skipFog;
  bool skipGodRays;
  bool skipHazeFlare;
  bool skipPostSpecular;
  bool skipNodeCPUCulling;
  bool skipNodeGPUCulling;
  bool skipAreaCPUCulling;
  bool skipAreaGPUCulling;
  bool skipModelCPUCulling;
  bool skipModelGPUCulling;
  bool skipLightCPUCulling;
  bool skipLightGPUCulling;
  bool skipSurfaceCPUCulling;
  bool skipModelRangeCulling;
  bool skipLightRangeCulling;
  std::uint8_t alignmentBeforeSelectors[3];
  int screenSpaceReflections;
  int singleWorldArea;
  int singleLight;
  int singleModel;
  int singleSurface;
  int singleGuiSurface;
  int singleDimShadow;
  int singleSelfShadow;
  bool lockView;
  bool forceTwoSidedDepth;
  bool useAmbientEnv;
  bool feedbackBGRA;
  bool showOcclusionBoxes;
  bool forceOcclusionBoxQueries;
  std::uint8_t alignmentBeforeViewCounts[2];
  int numViews;
  int generateMipMaps;
  int guiFrameCount;
  int queryThreshold;
  int doPreZMask;
  bool dimShadowUseQuery;
  bool dimShadowDepthBoundsTest;
  std::uint8_t alignmentBeforeDimShadowSettings[2];
  int dimShadowResolution;
  int dimShadowResolutionCap;
  int dimShadowForceHighQuality;
  int showDimShadows;
  float dimShadowMaxLixelsPerUnit;
  float dimShadowLixelScale;
  float dimShadowDensity;
  float dimShadowPolyOfsUnits;
  float dimShadowPolyOfsFactor;
  float dimShadowMaxVisibleRange;
  float dimShadowFadeVisibilityRange;
  bool dimShadowSkipRangeCulling;
  std::uint8_t alignmentBeforeSelfShadowSettings[3];
  int selfShadowResolution;
  int selfShadowResolutionCap;
  float selfShadowDensity;
  float selfShadowPolyOfsUnits;
  float selfShadowPolyOfsFactor;
  float selfShadowMaxVisibleRange;
  float selfShadowFadeVisibilityRange;
  float selfShadowLixelScale;
  bool selfShadowSkipRangeCulling;
  std::uint8_t alignmentBeforeSubSurfaceSettings[3];
  float subSurfaceScatteringMaxVisibilityRange;
  float subSurfaceScatteringFadeVisibilityRange;
  bool subSurfaceScatteringSkipRangeCulling;
  bool sortBlendedSurfacesBackToFront;
  bool useLightScissors;
  bool useLightDepthBoundsTest;
  bool useDeferredSlowMapLighting;
  bool showLightScissors;
  std::uint8_t alignmentBeforePostProcessSettings[2];
  int showBlendedLights;
  int postProcessDofMode;
  float sortCoverage;
  float sortSSS;
  float sortSkybox;
  float sortBackgroundFar;
  float sortBackground;
  float sortBackgroundEmit;
  float sortEmit;
  float sortEmitOnly;
  float sortLight;
  float sortDecal;
  float sortTransSort;
  float sortTrans;
  float sortWater;
  float sortPerturber;
  float sortLast;
  float viewNearZ;
  float viewFarZ;
  float guiOriginOffset;
  const idDeclRenderProg* progShowGuiOverdraw;
  idVec3 clearColor;
  alignas(16) idVec4 viewOrigin;
  idBounds viewBounds;
  std::uint8_t opaqueBetweenBounds[8];
  idBounds detailBounds;
  std::uint8_t opaqueTail[8];

  bool IsComboMap() const { return (featureFlags & 0x01u) != 0; }
  bool UseAmbientEnv() const { return useAmbientEnv; }
};

// NEED WORK: the complete render-view declaration is deferred until render.h
// itself is reconstructed.  Named fields are the retail offsets consumed by
// the currently recovered renderer jobs.
class alignas(16) idRenderView {
public:
  std::uint8_t opaqueBeforeRenderTime[0x1A40];
  int renderTime;
  int viewID;
  bool forceFullVirtualTextureLoad;
  bool discontinuousViewPosition;
  bool skipDimShadows;
  bool skipPostProcess;
  bool skipDynamicEnvironment;
  bool forceFullResolution;
  bool useHQViewColorMipsGeneration;
  bool inhibitModelFovScale;
  float fovX;
  float fovY;
  bool cramZNear;
  std::uint8_t alignmentBeforeExplicitProjection[3];
  idRenderMatrix explicitProjectionMatrix;
  bool useExplicitProjectionMatrix;
  std::uint8_t alignmentBeforeViewOrigin[3];
  idVec3 viewOrigin;
  idMat3 viewAxis;
  std::uint8_t opaqueBeforeMatrices[0x18F0];
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

static_assert(offsetof(renderSettings_t, viewBounds) == 0x150,
              "render-settings view bounds offset drift");
static_assert(offsetof(renderSettings_t, singleWorldArea) == 0x30,
              "render-settings world-area selector offset drift");
static_assert(offsetof(renderSettings_t, skipNodeCPUCulling) == 0x1E,
              "render-settings node CPU cull offset drift");
static_assert(offsetof(renderSettings_t, useAmbientEnv) == 0x4E,
              "render-settings ambient environment offset drift");
static_assert(offsetof(renderSettings_t, sortCoverage) == 0xD8,
              "render-settings sort threshold offset drift");
static_assert(offsetof(renderSettings_t, sortBackgroundEmit) == 0xEC,
              "render-settings background-emissive threshold offset drift");
static_assert(offsetof(renderSettings_t, sortDecal) == 0xFC,
              "render-settings decal threshold offset drift");
static_assert(offsetof(renderSettings_t, sortLast) == 0x110,
              "render-settings final threshold offset drift");
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
static_assert(offsetof(idRenderView, viewID) == 0x1A44,
              "render-view ID offset drift");
static_assert(offsetof(idRenderView, viewOrigin) == 0x1AA0,
              "render-view origin offset drift");
