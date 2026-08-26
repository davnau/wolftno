#pragma once

#include "engine/gamelib/animstack/animator_treesway_abi.h"
#include "engine/models/rendermodel.h"

#include <cstddef>
#include <cstdint>

class idTreeAnimator : public idRenderModel {
public:
  bool GetModelSpaceJointAxis(animationPose_t pose,
                              idJointIndex_t jointIndex,
                              idMat3& axis) const;
  bool GetModelSpaceJointTransform(animationPose_t pose,
                                   idJointIndex_t jointIndex,
                                   idVec3& origin, idMat3& axis) const;
  bool GetWorldSpaceJointTransform(animationPose_t pose,
                                   idJointIndex_t jointIndex,
                                   idVec3& origin, idMat3& axis) const;

  const idDeclMD6* decl;
  idJointAnimator* jointAnimator;
  std::uint8_t remainingTreeAnimatorData[0xC0];
};

static_assert(offsetof(idTreeAnimator, decl) == 0x880,
              "idTreeAnimator::decl ABI drift");
static_assert(offsetof(idTreeAnimator, jointAnimator) == 0x888,
              "idTreeAnimator::jointAnimator ABI drift");
static_assert(sizeof(idTreeAnimator) == 0x950,
              "idTreeAnimator ABI drift");
