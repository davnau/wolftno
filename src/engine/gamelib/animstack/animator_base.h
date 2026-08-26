#pragma once

#include "engine/animation/md6.h"

#include <cstddef>
#include <cstdint>

class idAnimStack;
class idAnimatorParms_Base;
class idClip;
class idGameTimeManager;
class idSerializer;
class idTreeAnimator;

enum gameTimeUnique_t : int;
enum serializeType_t { SERIALIZE_TYPE_NONE = 0 };

template <typename T, typename UniqueTag>
class idTypesafeNumber {
public:
  T value;
};

class idAnimator_Base {
public:
  enum priority_t { PRIORITY_DEFAULT = 0 };

  idAnimator_Base();
  virtual ~idAnimator_Base();

  virtual priority_t GetStackPriority() const;
  virtual serializeType_t GetSerializeType();
  virtual void SerializeSnapshot(idSerializer& serializer);
  virtual void PreBlendSnapshot(idAnimStack* stack, int curTime,
                                int ticksPerSec, float blendWeight);
  virtual void PreSerializeInit(idGameTimeManager* gameTimeManager);

protected:
  virtual bool InternalInit(const idAnimatorParms_Base& parms) = 0;
  virtual bool InternalPostInit(const idAnimatorParms_Base& parms);
  virtual void InternalShutdown(idAnimStack* stack) = 0;
  virtual void InternalPreBlendTree(const idAnimStack* stack, int curTime,
                                    int ticksPerSec) = 0;
  virtual void InternalPostBlendTree(const idAnimStack* stack, int curTime) = 0;
  virtual void InternalStart(
      const idAnimStack* stack, int curTime,
      idTypesafeNumber<int, gameTimeUnique_t> startTime) = 0;
  virtual void InternalEnd(
      const idAnimStack* stack, int curTime,
      idTypesafeNumber<int, gameTimeUnique_t> endTime) = 0;
  virtual void InternalBlend(
      const idAnimStack* stack, int curTime, float blendWeight,
      idTypesafeNumber<int, gameTimeUnique_t> blendTime) = 0;
  virtual bool InternalIsContributing() const = 0;
  virtual const idMD6Branch* InternalGetMergeBranch() const = 0;
  virtual idMD6Branch* InternalGetMergeBranch() = 0;
  virtual void InternalPause(
      const idAnimStack* stack,
      idTypesafeNumber<int, gameTimeUnique_t> pauseTime);
  virtual void InternalUnpause(
      const idAnimStack* stack,
      idTypesafeNumber<int, gameTimeUnique_t> unpauseTime);
  virtual const idMD6Branch* InternalGetEndBranch() const;
  virtual idMD6Branch* InternalGetEndBranch();

private:
  void* gameTimeManager;
  md6WeightGroup_t filterGroup;
  md6WeightGroup_t weightGroup;
  std::uint8_t serializeProps[0x30];
  std::uint8_t initialized : 1;
  std::uint8_t shutdownCalled : 1;
  std::uint8_t flagsPadding : 6;
  std::uint8_t tailPadding[7];
};

static_assert(sizeof(idTypesafeNumber<int, gameTimeUnique_t>) == 0x04,
              "idTypesafeNumber ABI drift");
static_assert(sizeof(idAnimator_Base) == 0x50,
              "idAnimator_Base ABI drift");

