#pragma once

#include <cstddef>
#include <cstdint>

class idMaterial;

// Verified prefix boundary of the retail idRenderSystemLocal object.  The
// fields before the default material table belong to renderer-system CPPs and
// remain opaque until those CPPs are reconstructed.
struct alignas(8) idRenderSystemLocal {
  std::uint8_t opaquePrefix[0x328];
  const idMaterial* mtrDefaultPointLight;
  const idMaterial* mtrDefaultProjectedLight;
  const idMaterial* mtrDefaultParallelLight;
  const idMaterial* mtrDefaultPointBlendLight;
  const idMaterial* mtrDefaultProjectedBlendLight;
  const idMaterial* mtrDefaultParallelBlendLight;
  const idMaterial* mtrDefaultPointSlowLight;
  const idMaterial* mtrDefaultProjectedSlowLight;
  const idMaterial* mtrDefaultParallelSlowLight;
};

extern idRenderSystemLocal tr;

static_assert(offsetof(idRenderSystemLocal, mtrDefaultPointLight) == 0x328,
              "idRenderSystemLocal default-light table offset drift");
static_assert(offsetof(idRenderSystemLocal, mtrDefaultParallelSlowLight) == 0x368,
              "idRenderSystemLocal default-light table end offset drift");

