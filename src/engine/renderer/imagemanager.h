#pragma once

// NEEDS_WORK(engine/renderer): PDB-derived image-manager boundary used by SWF.

#include "engine/renderer/image.h"

class idImageManager {
public:
  idImage* ScratchImage(const char* name, const idImageOpts* options);
  idImage* ImageFromFile(const char* name, const void* filter, bool forceLoad);

  idImage* defaultImage{};
  idImage* whiteImage{};
  idImage* borderedPointWhiteImage{};
  idImage* blackImage{};
  idImage* ps3ReplacementImage{};
  struct transSortImageAtlas_t* transSortImageAtlas{};
  textureFilter_t textureMinFilter{};
  textureFilter_t textureMaxFilter{};
  textureFilter_t textureMipFilter{};
  float textureAnisotropy{};
  float textureLODBias{};
};

struct transSortImageAtlas_t {
  std::uint8_t recoveredPrefix[20]{};
  int ATLAS_WIDTH{};
  int ATLAS_HEIGHT{};
};

static_assert(offsetof(idImageManager, transSortImageAtlas) == 40,
              "idImageManager transparency atlas offset drift");
static_assert(sizeof(idImageManager) == 72, "idImageManager ABI drift");

extern idImageManager* globalImages;
