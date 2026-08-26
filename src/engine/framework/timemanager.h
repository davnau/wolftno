#pragma once

#include <cstddef>

enum gameTimeUnique_t { GAME_TIME_UNIQUE };

template <typename T, typename UniqueType>
class idTimeValue {
public:
  constexpr idTimeValue() : value{} {}
  constexpr explicit idTimeValue(T input) : value(input) {}
  constexpr operator T() const { return value; }

  T value;
};

using idGameTime = idTimeValue<int, gameTimeUnique_t>;

class idTimeManager {
public:
  constexpr explicit idTimeManager(int hertz)
      : gameHz(hertz), inverseGameHz(1.0f / static_cast<float>(hertz)),
        gameTimePerFrame(
            static_cast<int>(1000.0f / static_cast<float>(hertz))),
        gameTimePerSecond(
            hertz * static_cast<int>(1000.0f / static_cast<float>(hertz))),
        inverseGameTimePerSecond(
            1.0f /
            static_cast<float>(hertz * static_cast<int>(
                                             1000.0f /
                                             static_cast<float>(hertz)))) {}

  void SetGameHz(int hertz);

  int GetGameHz() const { return gameHz; }
  float GetInvGameHz() const { return inverseGameHz; }
  idGameTime GetGameTimePerFrame() const { return gameTimePerFrame; }
  idGameTime GetGameTimePerSec() const { return gameTimePerSecond; }
  float GetInverseGameTimePerSecond() const {
    return inverseGameTimePerSecond;
  }

  int gameHz;
  float inverseGameHz;
  idGameTime gameTimePerFrame;
  idGameTime gameTimePerSecond;
  float inverseGameTimePerSecond;
};

extern idTimeManager timeManager;

static_assert(sizeof(idGameTime) == 0x04, "idGameTime ABI drift");
static_assert(sizeof(idTimeManager) == 0x14, "idTimeManager ABI drift");
static_assert(offsetof(idTimeManager, gameTimePerFrame) == 0x08,
              "idTimeManager::gameTimePerFrame ABI drift");
static_assert(offsetof(idTimeManager, inverseGameTimePerSecond) == 0x10,
              "idTimeManager::inverseGameTimePerSecond ABI drift");

