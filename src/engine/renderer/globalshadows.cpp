#include "engine/renderer/globalshadows.h"

#include "engine/framework/fileresource.h"
#include "shared/idlib/csystems/cvarsystemlocal.h"
#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/sys/sys_alloc.h"
#include "shared/idlib/text/str.h"

#include <cstring>
#include <algorithm>
#include <cmath>

idCVar r_globalShadowsCoordinateBias(
    "r_globalShadowsCoordinateBias", "0.5", CVAR_FLOAT,
    "Bias to the shadow check coordinate calculation");
idCVar r_globalShadowsDepthBias(
    "r_globalShadowsDepthBias", "0", CVAR_FLOAT,
    "Bias to the shadow check depth calculation");
idCVar r_useGlobalShadows(
    "r_useGlobalShadows", "1", CVAR_BOOL,
    "If false, always return 1.0 for shadow tests");

float shadowMapCPU_t::ShadowSample(const idVec3& worldPoint) const {
  if (!useShadowMap || map == nullptr)
    return 1.0f;

  const float projected[4] = {
      worldPoint.x * mvp.m[0] + worldPoint.y * mvp.m[1] +
          worldPoint.z * mvp.m[2] + mvp.m[3],
      worldPoint.x * mvp.m[4] + worldPoint.y * mvp.m[5] +
          worldPoint.z * mvp.m[6] + mvp.m[7],
      worldPoint.x * mvp.m[8] + worldPoint.y * mvp.m[9] +
          worldPoint.z * mvp.m[10] + mvp.m[11],
      worldPoint.x * mvp.m[12] + worldPoint.y * mvp.m[13] +
          worldPoint.z * mvp.m[14] + mvp.m[15]};

  int coords[2]{};
  float fractions[2][2]{};
  for (int axis = 0; axis < 2; ++axis) {
    const float coordinate = width * projected[axis] + coordinateBias;
    const float integral = std::floor(coordinate);
    coords[axis] = static_cast<int>(integral);
    fractions[axis][1] = coordinate - integral;
    if (coords[axis] < 0) {
      coords[axis] = 0;
      fractions[axis][1] = 0.0f;
    } else if (coords[axis] >= width - 1) {
      coords[axis] = width - 2;
      fractions[axis][1] = 1.0f;
    }
    fractions[axis][0] = 1.0f - fractions[axis][1];
  }

  const int sampleOffsets[4] = {
      coords[0] + coords[1] * width,
      coords[0] + 1 + coords[1] * width,
      coords[0] + (coords[1] + 1) * width,
      coords[0] + 1 + (coords[1] + 1) * width};
  const float depth = std::clamp(projected[2], 0.0f, 1.0f) * 65535.0f +
                      depthBias;
  const unsigned sampleDepth = static_cast<unsigned>(static_cast<int>(depth));
  const float equalDepthWeight = 1.0f - (depth - std::floor(depth));
  float shadow = 0.0f;
  for (int y = 0; y < 2; ++y) {
    for (int x = 0; x < 2; ++x) {
      const unsigned mapDepth = map[std::abs(sampleOffsets[y * 2 + x])];
      const float weight = fractions[0][x] * fractions[1][y];
      if (mapDepth > sampleDepth)
        shadow += weight;
      else if (mapDepth == sampleDepth)
        shadow += equalDepthWeight * weight;
    }
  }
  return shadow;
}

idGlobalShadows::idGlobalShadows() {
  shadowMap = static_cast<shadowMapCPU_t*>(
      idMem::Singleton()->AllocWithLocation(
          "w:\\build\\tech5\\shared\\idlib\\Heap.h(95) : tag",
          sizeof(shadowMapCPU_t), TAG_RENDERER, false, ALIGN_16,
          HEAP_DEFAULTHEAP));
  shadowMap->width = 1024;
  shadowMap->map = nullptr;
  std::memset(&shadowMap->mvp, 0, sizeof(shadowMap->mvp));
  shadowMap->mvp.m[15] = 1.0F;
  shadowMap->coordinateBias = -0.5F;
  shadowMap->depthBias = -40.0F;
  shadowMap->useShadowMap = true;
}

idGlobalShadows::~idGlobalShadows() {
  if (shadowMap->map != nullptr) {
    idMem::Singleton()->Free(shadowMap->map, ALIGN_16);
    shadowMap->map = nullptr;
  }
  idMem::Singleton()->Free(shadowMap, ALIGN_16);
}

void idGlobalShadows::LoadGlobalShadows(const char* mapName) {
  if (shadowMap->map != nullptr) {
    idMem::Singleton()->Free(shadowMap->map, ALIGN_16);
    shadowMap->map = nullptr;
  }
  std::memset(&shadowMap->mvp, 0, sizeof(shadowMap->mvp));
  shadowMap->mvp.m[15] = 1.0F;

  idStr shadowFilename(mapName);
  shadowFilename += "/globalShadows2.bin";
  if (!idFileResource::FileExists(shadowFilename.data)) {
    return;
  }

  idFileResource* resource = static_cast<idFileResource*>(
      idFileResource::resourceList.Load(shadowFilename.data, false, false));
  if (resource == nullptr || static_cast<int>(resource->length) <= 0) {
    return;
  }

  idFile_Memory* file = resource->GetFile();
  if (file == nullptr) {
    return;
  }
  if (file->Length() <= 0) {
    delete file;
    return;
  }

  idLib::Printf("Loading globalShadows.\n");
  file->Read(&shadowMap->mvp, sizeof(shadowMap->mvp));
  shadowMap->width = 1024;
  shadowMap->map = static_cast<std::uint16_t*>(
      idMem::Singleton()->AllocWithLocation(
          "w:\\build\\tech5\\engine\\renderer\\GlobalShadows.cpp(92) : TAG_RENDERWORLD",
          0x200000U, TAG_RENDERWORLD, false, ALIGN_16, HEAP_DEFAULTHEAP));
  file->Read(shadowMap->map, 0x200000U);
  delete file;

  shadowMap->coordinateBias = r_globalShadowsCoordinateBias.valueFloat;
  shadowMap->depthBias = r_globalShadowsDepthBias.valueFloat;
  shadowMap->useShadowMap = r_useGlobalShadows.valueInteger != 0;
  delete resource;
}

const shadowMapCPU_t* idGlobalShadows::GetShadowMap() {
  shadowMap->coordinateBias = r_globalShadowsCoordinateBias.valueFloat;
  shadowMap->depthBias = r_globalShadowsDepthBias.valueFloat;
  shadowMap->useShadowMap = r_useGlobalShadows.valueInteger != 0;
  return shadowMap;
}
