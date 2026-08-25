#pragma once

#include "shared/idlib/text/str.h"

#include <cstddef>
#include <cstdint>

class idVirtualTexture {
public:
  void SetMapName(const char* mapName);

  std::uint8_t recoveredPrefix[760]{};
  idStrStatic<128> mapName;
  std::uint8_t recoveredTail[184]{};
};

static_assert(offsetof(idVirtualTexture, mapName) == 760,
              "virtual-texture map-name offset drift");
static_assert(sizeof(idVirtualTexture) == 1112,
              "virtual-texture ABI drift");
