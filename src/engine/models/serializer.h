#pragma once

#include "shared/idlib/math/vector.h"
#include "shared/idlib/networking/bitmsg.h"

#include <algorithm>
#include <cmath>
#include <cstring>

struct idSerializer {
  bool writing = false;
  std::uint8_t padding[7]{};
  idBitMsg* msg = nullptr;

  void SerializePacked(int* original);

  template <int Maximum, int Bits> void SerializeUQ(float* value) {
    constexpr int quantizedMaximum = (1U << Bits) - 1U;
    constexpr int integerScale = quantizedMaximum / Maximum;
    if (writing) {
      const int quantized = static_cast<int>(*value * integerScale);
      msg->WriteBits(std::max(0, std::min(quantizedMaximum, quantized)),
                     Bits);
    } else {
      *value = static_cast<float>(msg->ReadBits(Bits)) *
               (static_cast<float>(Maximum) /
                static_cast<float>(quantizedMaximum));
    }
  }

  template <int Maximum, int Bits> void SerializeQ(float* value) {
    constexpr int quantizedMaximum = (1U << (Bits - 1)) - 1U;
    constexpr int integerScale = quantizedMaximum / Maximum;
    if (writing) {
      const int quantized = static_cast<int>(*value * integerScale);
      msg->WriteBits(std::max(-quantizedMaximum,
                             std::min(quantizedMaximum, quantized)),
                     -Bits);
    } else {
      *value = static_cast<float>(msg->ReadBits(-Bits)) *
               (static_cast<float>(Maximum) /
                static_cast<float>(quantizedMaximum));
    }
  }

  void Serialize(idVec3* value) {
    float* components = &value->x;
    for (int index = 0; index < 3; ++index) {
      int bits = 0;
      if (writing) {
        std::memcpy(&bits, &components[index], sizeof(bits));
        msg->WriteBits(bits, 32);
      } else {
        bits = msg->ReadBits(32);
        std::memcpy(&components[index], &bits, sizeof(bits));
      }
    }
  }

  void SerializeQ(idMat3* axis, int bits) {
    const float scale = static_cast<float>((1 << (bits - 1)) - 1);
    if (writing) {
      idQuat quaternion = axis->ToQuat();
      float values[4] = {quaternion.x, quaternion.y, quaternion.z, quaternion.w};
      int largest = 0;
      for (int index = 1; index < 4; ++index) {
        if (std::fabs(values[index]) > std::fabs(values[largest]))
          largest = index;
      }
      msg->WriteBits(largest, 2);
      const float sign = values[largest] < 0.0f ? -1.0f : 1.0f;
      for (int offset = 1; offset < 4; ++offset) {
        const int index = (largest + offset) & 3;
        msg->WriteBits(static_cast<int>(values[index] * sign * scale), -bits);
      }
    } else {
      float values[4]{};
      const int largest = msg->ReadBits(2);
      float sum = 0.0f;
      for (int offset = 1; offset < 4; ++offset) {
        const int index = (largest + offset) & 3;
        values[index] = static_cast<float>(msg->ReadBits(-bits)) / scale;
        sum += values[index] * values[index];
      }
      values[largest] = std::sqrt(std::max(1.0f - sum, 1.17549435e-38f));
      *axis = idQuat{values[0], values[1], values[2], values[3]}.ToMat3();
    }
  }
};

static_assert(sizeof(idSerializer) == 16, "idSerializer ABI drift");
