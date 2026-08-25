#include "engine/models/skeletalanimation/md6parsetree.h"

#include <algorithm>

namespace {

idMD6ParseTreeModelCompatible g_modelCompatible = nullptr;
idMD6ParseTreeBranchUpdate g_branchUpdate = nullptr;

}

void SetMD6ParseTreeModelCompatible(idMD6ParseTreeModelCompatible compatible) {
  g_modelCompatible = compatible;
}

void SetMD6ParseTreeBranchUpdate(idMD6ParseTreeBranchUpdate update) {
  g_branchUpdate = update;
}

bool idMD6Anim::AnimIsOkForModel(const idMD6Model* model) const {
  if (g_modelCompatible != nullptr)
    return g_modelCompatible(this, model);
  return model != nullptr && model->skeleton != nullptr &&
         (animData != nullptr || postedForGeneration);
}

bool idMD6Node::IsValid() const {
  if (this == nullptr)
    return false;
  if (type == NODE_LEAF_PAUSE || type == NODE_LEAF_PLAY)
    return static_cast<const idMD6Leaf*>(this)->weightGroup !=
           MD6_WEIGHTGROUP_MAX;
  if (type == NODE_BEST_LEAF)
    return static_cast<const idMD6BestLeaf*>(this)->leafList.num > 0;
  if (type != NODE_BRANCH && type != NODE_BLEND_BRANCH &&
      type != NODE_BLENDA_BRANCH && type != NODE_FUSION_BRANCH)
    return false;
  const auto* branch = static_cast<const idMD6Branch*>(this);
  return branch->right != nullptr && branch->left != nullptr;
}

int idMD6Leaf::GetFrameRate() const {
  return anim != nullptr && anim->animData != nullptr
             ? anim->animData->frameRate
             : 30;
}

int idMD6LeafPlay::GetFrameTime(int gameTime) const {
  return gameTime >= startTime
             ? static_cast<int>(static_cast<float>(gameTime - startTime) *
                                rateScale)
             : 0;
}

void idMD6Branch::UpdateCurrentAlpha(int timeMSec, int lastTimeMSec,
                                     int ticksPerSec) {
  const int elapsed = timeMSec - lastTimeMSec;
  if (elapsed <= 0)
    return;
  const float difference = targetAlpha - currentAlpha;
  const float elapsedSeconds =
      static_cast<float>(elapsed) / static_cast<float>(ticksPerSec);
  if (difference > 0.0f) {
    const float value = elapsedSeconds * alphaRate + currentAlpha;
    currentAlpha = value <= targetAlpha - 0.000001f ? value : targetAlpha;
  } else if (difference < 0.0f) {
    const float value = currentAlpha - elapsedSeconds * alphaRate;
    currentAlpha = targetAlpha + 0.000001f <= value ? value : targetAlpha;
  }
}

void idMD6BlendBranch::Update(bool forceUpdate, bool isSorted) {
  if (g_branchUpdate != nullptr)
    g_branchUpdate(this, forceUpdate, isSorted ? 1 : 0, 0, nullptr);
}

void idMD6BlendAdditiveBranch::Update(bool forceUpdate) {
  if (g_branchUpdate != nullptr)
    g_branchUpdate(this, forceUpdate, 0, 0, nullptr);
}

void idMD6FusionBranch::Update(bool forceUpdate, int gameTime,
                               int ticksPerSec,
                               const idMD6Skel* skeleton) {
  if (g_branchUpdate != nullptr)
    g_branchUpdate(this, forceUpdate, gameTime, ticksPerSec, skeleton);
}

void idBounds::AddBounds(const idBounds& bounds) {
  for (int axis = 0; axis < 3; ++axis) {
    b[0][axis] = std::min(b[0][axis], bounds[0][axis]);
    b[1][axis] = std::max(b[1][axis], bounds[1][axis]);
  }
}
