#pragma once

#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idRenderDebug;

struct ambientHeader_t {
  int magic;
  int numNodes;
  int numSamples;
};

struct ambientNode_t {
  std::uint16_t children[8];
};

struct ambientColors_t {
  std::uint8_t channels[6][3];
};

struct ambientSample_t {
  ambientColors_t colors;
  std::int16_t origin[3];
};

struct ambientMapCPU_t {
  void SamplesForPoint(const idVec3& worldPoint, int* cube,
                       ambientSample_t* cubeSamples) const;

  ambientHeader_t* data;
  int dataLen;
  ambientColors_t forcedColors;
  bool useForcedColors;
  std::uint8_t alignmentPadding;
  ambientNode_t* nodes;
  ambientSample_t* samples;
};

class idAmbientLighting {
public:
  idAmbientLighting();
  ~idAmbientLighting();

  void LoadAmbientLighting(const char* mapName);
  void DebugDrawAmbientLightingSamples(idRenderDebug* renderDebug,
                                       const idVec3& position) const;
  static void SwapNodes(ambientNode_t* nodes, int numNodes);
  static void SwapSamples(ambientSample_t* samples, int numSamples);

private:
  void FreeData();

  ambientMapCPU_t* ambientMap;
};

static_assert(sizeof(ambientHeader_t) == 0x0C,
              "ambient header ABI drift");
static_assert(sizeof(ambientNode_t) == 0x10,
              "ambient node ABI drift");
static_assert(sizeof(ambientColors_t) == 0x12,
              "ambient colors ABI drift");
static_assert(sizeof(ambientSample_t) == 0x18,
              "ambient sample ABI drift");
static_assert(offsetof(ambientMapCPU_t, useForcedColors) == 0x1E,
              "ambient map forced-color flag offset drift");
static_assert(offsetof(ambientMapCPU_t, nodes) == 0x20,
              "ambient map node offset drift");
static_assert(sizeof(ambientMapCPU_t) == 0x30,
              "ambient map ABI drift");
static_assert(sizeof(idAmbientLighting) == sizeof(void*),
              "ambient-lighting ABI drift");
