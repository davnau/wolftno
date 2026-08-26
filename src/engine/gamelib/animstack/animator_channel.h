#pragma once

#include "engine/gamelib/animstack/animator_base.h"

#include <cstddef>
#include <cstdint>

class idAnimator_Channel : public idAnimator_Base {
public:
  idAnimator_Channel();
  ~idAnimator_Channel() override;

protected:
  bool InternalInit(const idAnimatorParms_Base& parms) override;
  void InternalShutdown(idAnimStack* stack) override;
  void InternalPreBlendTree(const idAnimStack* stack, int curTime,
                            int ticksPerSec) override;
  void InternalPostBlendTree(const idAnimStack* stack, int curTime) override;
  void InternalStart(
      const idAnimStack* stack, int curTime,
      idTypesafeNumber<int, gameTimeUnique_t> startTime) override;
  void InternalEnd(
      const idAnimStack* stack, int curTime,
      idTypesafeNumber<int, gameTimeUnique_t> endTime) override;
  void InternalBlend(
      const idAnimStack* stack, int curTime, float blendWeight,
      idTypesafeNumber<int, gameTimeUnique_t> blendTime) override;
  bool InternalIsContributing() const override;
  const idMD6Branch* InternalGetMergeBranch() const override;
  idMD6Branch* InternalGetMergeBranch() override;

  std::uint8_t channelData[0x62];
  std::uint8_t forceFree : 1;
  std::uint8_t omitFreeLeafWarning : 1;
  std::uint8_t channelFlagPadding : 6;
  std::uint8_t channelTailPadding[5];

  friend class AnimatorPainValidationAccess;
};

static_assert(sizeof(idAnimator_Channel) == 0xB8,
              "idAnimator_Channel ABI drift");
