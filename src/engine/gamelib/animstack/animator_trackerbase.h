#pragma once

#include "engine/gamelib/animstack/animator_base.h"
#include "engine/gamelib/animstack/treeanimator_abi.h"

#include <cstddef>
#include <cstdint>

enum DegreesUnique_t : int;

class idAnimator_TrackerBase : public idAnimator_Base {
public:
  idAnimator_TrackerBase();
  ~idAnimator_TrackerBase() override;

  void SerializeSnapshot(idSerializer& serializer) override;
  void PreBlendSnapshot(idAnimStack* stack, int curTime, int ticksPerSec,
                        float blendWeight) override;

  virtual void SetReferenceJointOverride(idJointIndex_t jointIndex);
  virtual const idJointIndex_t GetReferenceJointOverride() const;
  virtual void GetModelSpaceFrameOfReferenceForChild(
      const idTreeAnimator* treeAnimator, idVec3& modelPosition,
      idMat3& modelAxis) const;
  virtual void GetWorldSpaceFrameOfReferenceForChild(
      const idTreeAnimator* treeAnimator, idVec3& worldPosition,
      idMat3& worldAxis) const;
  virtual void GetWorldSpaceFrameOfReference(
      const idAnimator_TrackerBase* parentTracker,
      const idTreeAnimator* treeAnimator, idVec3& worldPosition,
      idMat3& worldAxis) const;
  virtual void GetModelSpaceFrameOfReference(
      const idAnimator_TrackerBase* parentTracker,
      const idTreeAnimator* treeAnimator, idVec3& modelPosition,
      idMat3& modelAxis) const;

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
  virtual void InternalUpdate(const idAnimator_TrackerBase* parentTracker);

  const idAnimator_TrackerBase* parentTracker;
  idTreeAnimator* treeAnimator;
  idVec3 focusPoint;
  std::uint8_t focusPoints[0x18];
  idTypesafeNumber<float, DegreesUnique_t> yawRight;
  idTypesafeNumber<float, DegreesUnique_t> yawLeft;
  idTypesafeNumber<float, DegreesUnique_t> pitchUp;
  idTypesafeNumber<float, DegreesUnique_t> pitchDown;
  int trackGroupIndex;
  idMD6Branch mergeBranch;
  idMD6LeafPause leaf;
  idVec3 msDirToFocus;
  idVec3 wsFocusJointPos;
  idVec3 msReferenceJointPos;
  idAngles idealOffsetAngles;
  idAngles curOffsetAngles;
  idAngles lastAngles;
  int lastValidTime;
  std::uint8_t trackerFlags;
  std::uint8_t trackerTailPadding[3];

  friend class AnimatorTorsoTrackerValidationAccess;
};

static_assert(sizeof(idMD6Branch) == 0x38, "idMD6Branch ABI drift");
static_assert(sizeof(idMD6LeafPause) == 0x70,
              "idMD6LeafPause ABI drift");
static_assert(sizeof(idAnimator_TrackerBase) == 0x190,
              "idAnimator_TrackerBase ABI drift");
