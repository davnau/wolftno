#pragma once

#include "shared/idlib/math/vector.h"

#include <cstdint>

struct idColor {
  float r, g, b, a;

  // The retail renderer's debug interface is expressed in idVec4.  idColor
  // is layout-compatible and remains the source-facing convenience type used
  // by game/model call sites.
  operator const idVec4&() const {
    return *reinterpret_cast<const idVec4*>(this);
  }

  static const idColor colorRed;
  static const idColor colorBlack;
  static const idColor colorGreen;
  static const idColor colorLtGrey;
  static const idColor colorBlue;
  static const idColor colorCyan;
  static const idColor colorYellow;
  static const idColor colorWhite;
  static const idColor colorOrange;
  static const idColor colorMagenta;
  static const idColor colorXAxis;
  static const idColor colorYAxis;
  static const idColor colorZAxis;

  const char* ToString(int precision = 2, bool parentheses = false) const;
  void Lerp(const idColor& from, const idColor& to, float fraction);
  static std::uint32_t PackColor(const idVec4& color);
  static void UnpackColor(std::uint32_t color, idVec4& unpackedColor);
};

static_assert(sizeof(idColor) == 16, "idColor ABI drift");
