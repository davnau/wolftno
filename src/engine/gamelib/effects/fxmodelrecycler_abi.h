#pragma once

#include <cstdint>

// Narrow retail ABI surface used by idGameLibEffects. fxmanager.cpp owns the
// concrete particle/static model records and hash-index implementation.
class alignas(8) idFXModelRecycler {
public:
  idFXModelRecycler();
  ~idFXModelRecycler();

  void Init();
  void Shutdown();

  std::uint8_t recoveredStorage[0x1A00];
};

static_assert(sizeof(idFXModelRecycler) == 0x1A00,
              "FX model recycler ABI drift");
