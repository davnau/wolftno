#pragma once

#include <cstddef>
#include <cstdint>

// Narrow retail ABI surface used by idGameLibEffects. The implementation and
// concrete ribbonEffects_t storage belong to ribbonmanager.cpp.
class idRibbonModelManager {
public:
  idRibbonModelManager();
  ~idRibbonModelManager();

  void Init();
  void Shutdown();

  union {
    std::uint64_t recoveredAlignment;
    std::uint8_t recoveredRibbonEffects[0x3F8];
  };
  int handleId;
  std::uint8_t recoveredTail[4];
};

static_assert(sizeof(idRibbonModelManager) == 0x400,
              "ribbon model manager ABI drift");
static_assert(offsetof(idRibbonModelManager, handleId) == 0x3F8,
              "ribbon model manager handle offset drift");
