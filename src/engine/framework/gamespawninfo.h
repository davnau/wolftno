#pragma once

#include "shared/idlib/text/str.h"

#include <cstddef>

class idGameSpawnInfo {
public:
  int gameFlags;
  std::byte padding0[4];
  idStrStatic<1024> mapName;
  idStrStatic<1024> spawnSpot;
  idStrStatic<1024> spawnSpotOverride;
  idStrStatic<64> checkpointName;
  idStrStatic<1024> loadingScreen;
  std::byte remaining[0x1614C-0x1110];
  int currentCheckpointSlot;
};

static_assert(sizeof(idGameSpawnInfo)==0x16150,"idGameSpawnInfo ABI drift");
static_assert(offsetof(idGameSpawnInfo,mapName)==0x8,"spawn map ABI drift");
static_assert(offsetof(idGameSpawnInfo,loadingScreen)==0xCE8,"spawn load screen ABI drift");
static_assert(offsetof(idGameSpawnInfo,currentCheckpointSlot)==0x1614C,
              "spawn checkpoint slot ABI drift");
