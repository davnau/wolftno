#pragma once

#include <cstdint>

class idRandom2 {
public:
  std::uint32_t seed;
};

static_assert(sizeof(idRandom2) == 4, "idRandom2 ABI drift");
