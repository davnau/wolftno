#include "engine/models/skeletalanimation/md6parsetree.h"

#include "shared/idlib/csystems/cvarsystemlocal.h"
#include "shared/idlib/lib.h"

#include <algorithm>
#include <cmath>
#include <cstring>

idCVar anim_showDecodes(
    "anim_showDecodes", "0", CVAR_BOOL,
    "show which animations and frames were queued for decoding.");

namespace {

idMD6ParseTreeModelCompatible modelCompatible = nullptr;
idMD6ParseTreeBranchUpdate branchUpdate = nullptr;

int FrameRate(const idMD6Leaf* leaf) {
  return leaf != nullptr && leaf->anim != nullptr &&
                 leaf->anim->animData != nullptr
             ? leaf->anim->animData->frameRate
             : 30;
}

int FrameTime(const idMD6LeafPlay* leaf, int gameTime) {
  return gameTime >= leaf->startTime
             ? static_cast<int>((gameTime - leaf->startTime) * leaf->rateScale)
             : 0;
}

void UpdateCurrentAlpha(idMD6Branch* branch, int timeMSec,
                        int lastTimeMSec, int ticksPerSec) {
  const int elapsed = timeMSec - lastTimeMSec;
  if (elapsed <= 0)
    return;
  const float difference = branch->targetAlpha - branch->currentAlpha;
  if (difference == 0.0f)
    return;
  const float change = static_cast<float>(elapsed) / ticksPerSec *
                       branch->alphaRate;
  if (difference > 0.0f) {
    const float value = branch->currentAlpha + change;
    branch->currentAlpha = value <= branch->targetAlpha - 0.000001f
                               ? value : branch->targetAlpha;
  } else {
    const float value = branch->currentAlpha - change;
    branch->currentAlpha = branch->targetAlpha + 0.000001f <= value
                               ? value : branch->targetAlpha;
  }
}

idMD6Node* ResolveBestLeaf(idMD6Node* node) {
  if (node == nullptr || node->type != idMD6Node::NODE_BEST_LEAF)
    return node;
  auto* best = reinterpret_cast<idMD6BestLeaf*>(node);
  if (best->leafList.num == 0)
    return nullptr;
  std::uint16_t index = best->bestLeafIndex;
  if (index >= best->leafList.num)
    index = 0;
  return best->leafList.list[index];
}

bool IsValidNode(const idMD6Node* node) {
  if (node == nullptr)
    return false;
  if (node->type == idMD6Node::NODE_LEAF_PAUSE ||
      node->type == idMD6Node::NODE_LEAF_PLAY)
    return reinterpret_cast<const idMD6Leaf*>(node)->weightGroup != 8;
  if (node->type == idMD6Node::NODE_BEST_LEAF)
    return reinterpret_cast<const idMD6BestLeaf*>(node)->leafList.num > 0;
  if (node->type != idMD6Node::NODE_BRANCH &&
      node->type != idMD6Node::NODE_BLEND_BRANCH &&
      node->type != idMD6Node::NODE_BLENDA_BRANCH &&
      node->type != idMD6Node::NODE_FUSION_BRANCH)
    return false;
  const auto* branch = reinterpret_cast<const idMD6Branch*>(node);
  return branch->left != nullptr && branch->right != nullptr;
}

void AddShortBounds(idBounds* destination, const idBoundsShort& source) {
  for (int axis = 0; axis < 3; ++axis) {
    (*destination)[0][axis] = std::min(
        (*destination)[0][axis], static_cast<float>(source.b[0][axis]));
    (*destination)[1][axis] = std::max(
        (*destination)[1][axis], static_cast<float>(source.b[1][axis]));
  }
}

void AddBounds(idBounds* destination, const idBounds& source) {
  for (int axis = 0; axis < 3; ++axis) {
    (*destination)[0][axis] =
        std::min((*destination)[0][axis], source[0][axis]);
    (*destination)[1][axis] =
        std::max((*destination)[1][axis], source[1][axis]);
  }
}

const frameSetData_t* FrameSetForFrame(const idMD6AnimData* data,
                                       std::int16_t frame,
                                       std::uint16_t* setSize) {
  const auto* table = reinterpret_cast<const std::uint8_t*>(data) +
                      data->frameSetTblOffset;
  const auto* offsets = reinterpret_cast<const std::uint32_t*>(
      reinterpret_cast<const std::uint8_t*>(data) +
      data->frameSetOffsetTblOffset);
  const std::uint8_t setIndex = table[frame];
  const std::uint32_t start = offsets[setIndex] * 16u;
  const std::uint32_t end = offsets[setIndex + 1] * 16u;
  *setSize = static_cast<std::uint16_t>(data->nextSize + end - start);
  return reinterpret_cast<const frameSetData_t*>(
      reinterpret_cast<const std::uint8_t*>(data) + start);
}

bool ProcessLeft(const idMD6Branch* branch) {
  const std::uint8_t op = branch->op;
  return branch->filterGroup != 0 ||
         (((op == 1 || op == 2 || op == 7 || op == 9 || op == 10) &&
           branch->currentAlpha != 1.0f) ||
          ((op == 3 || op == 5) && branch->currentAlpha != 0.0f) ||
          op == 4 || op == 6 || op == 8);
}

bool ProcessRight(const idMD6Branch* branch) {
  const std::uint8_t op = branch->op;
  return branch->filterGroup != 0 ||
         (((op == 1 || op == 2 || op == 7 || op == 9 || op == 10 ||
            op == 4 || op == 6 || op == 8) &&
           branch->currentAlpha != 0.0f) || op == 3 || op == 5);
}

} // namespace

void SetMD6ParseTreeModelCompatible(idMD6ParseTreeModelCompatible compatible) {
  modelCompatible = compatible;
}

void SetMD6ParseTreeBranchUpdate(idMD6ParseTreeBranchUpdate update) {
  branchUpdate = update;
}

void GetFrame(idMD6LeafPlay* leaf, int timeMSec, std::int16_t* frame,
              float* fraction, std::uint32_t* loops,
              std::uint32_t ticksPerSec) {
  *loops = 0;
  float floatFrame = 0.0f;
  const int numFrames = leaf->anim != nullptr && leaf->anim->animData != nullptr
                            ? leaf->anim->animData->numFrames : 0;
  const int frameRate = FrameRate(leaf);
  if (leaf->type == idMD6Node::NODE_LEAF_PAUSE) {
    floatFrame = reinterpret_cast<idMD6LeafPause*>(leaf)->frame;
  } else if (leaf->type == idMD6Node::NODE_LEAF_PLAY) {
    const int frameTime = FrameTime(leaf, timeMSec);
    *loops = static_cast<std::uint32_t>(frameRate * frameTime /
                                       ticksPerSec / (numFrames - 1));
    floatFrame = *loops == 0 || leaf->wrapMode != 0
                     ? (1.0f / static_cast<float>(ticksPerSec)) *
                           static_cast<float>(frameTime) * frameRate
                     : static_cast<float>(numFrames - 1);
  }
  if (leaf->wrapMode != 0) {
    if (leaf->wrapMode == 1) {
      const int integerFrame = static_cast<int>(floatFrame);
      *fraction = floatFrame - integerFrame;
      *frame = numFrames > 1
                   ? static_cast<std::int16_t>(integerFrame % (numFrames - 1))
                   : static_cast<std::int16_t>(integerFrame);
      return;
    }
    *frame = 0;
    return;
  }
  if (floatFrame < 0.0f) {
    *fraction = 0.0f;
    *frame = 0;
  } else if (floatFrame < numFrames) {
    const int integerFrame = static_cast<int>(floatFrame);
    *fraction = floatFrame - integerFrame;
    *frame = static_cast<std::int16_t>(integerFrame);
  } else {
    *fraction = 0.0f;
    *frame = static_cast<std::int16_t>(numFrames - 1);
  }
}

md6AnimCommand_t* SetupCommandBlendInfo(md6AnimCommand_t* command,
                                        idMD6Branch* branch) {
  std::memset(command, 0, sizeof(*command));
  command->op = OP_POP_AND_BLEND;
  command->blendInfo.op = branch->op;
  command->blendInfo.filterGroup = branch->filterGroup;
  command->blendInfo.originBlend = branch->originBlend;
  float alpha = branch->currentAlpha;
  switch (branch->blendType) {
    case 0:
      break;
    case 1: {
      const float squared = alpha * alpha;
      alpha = alpha <= branch->targetAlpha ? squared : 2.0f * alpha - squared;
      break;
    }
    case 2: {
      const float squared = alpha * alpha;
      alpha = alpha > branch->targetAlpha ? squared : 2.0f * alpha - squared;
      break;
    }
    case 3: {
      const float doubled = alpha * 2.0f;
      alpha = doubled >= 1.0f
                  ? ((2.0f * (doubled - 1.0f) -
                      (doubled - 1.0f) * (doubled - 1.0f)) + 1.0f) * 0.5f
                  : doubled * doubled * 0.5f;
      break;
    }
    default:
      idLib::Warning("Unknown MD6 blend type");
      break;
  }
  command->blendInfo.currentAlpha = alpha;
  return command + 1;
}

md6AnimCommand_t* SetupCommandFrameInfo(
    const idMD6Model* model, md6AnimCommand_t* command, idMD6LeafPlay* leaf,
    std::uint32_t lastTimeMSec, int timeMSec, idBounds* frameBounds,
    idBounds* translatedBounds, idBounds* normalizedBounds,
    std::uint16_t animatorFlags, int ticksPerSec) {
  if (command == nullptr || model == nullptr || leaf == nullptr)
    return nullptr;
  std::memset(command, 0, sizeof(*command));
  command->op = OP_DECODE_AND_PUSH;
  command->frameInfo.weightGroup = leaf->weightGroup;
  const idMD6Anim* animation = leaf->anim;
  if (animation != nullptr) {
    std::uint32_t loops;
    GetFrame(leaf, timeMSec, &command->frameInfo.frame,
             &command->frameInfo.frameFraction, &loops, ticksPerSec);
    command->animName = animation->name.str;
    if (anim_showDecodes.GetInteger() != 0)
      idLib::Printf("frame %d (%g), anim %s\n", command->frameInfo.frame,
                    command->frameInfo.frameFraction, animation->name.str);
    if (leaf->type == idMD6Node::NODE_LEAF_PLAY &&
        leaf->startTime < timeMSec) {
      std::int16_t lastFrame;
      float lastFraction;
      std::uint32_t lastLoops;
      GetFrame(leaf, static_cast<int>(lastTimeMSec), &lastFrame,
               &lastFraction, &lastLoops, ticksPerSec);
      if (loops > lastLoops)
        command->frameInfo.flags |= 4u;
    }
    command->frameInfo.flags |= (animatorFlags & 1u) != 0 ? 8u : 0u;
    if (modelCompatible != nullptr && !modelCompatible(animation, model)) {
      idLib::Warning("Model '%s' cannot use animation '%s'", model->name.str,
                     animation->name.str);
      return nullptr;
    }
    const idMD6AnimData* data = animation->animData;
    command->frameInfo.animData = data;
    command->frameInfo.animInfoSize = data->size;
    command->frameInfo.frameSetData = FrameSetForFrame(
        data, command->frameInfo.frame, &command->frameInfo.frameSetSize);
    command->frameInfo.frameSetFrame = static_cast<std::int16_t>(
        command->frameInfo.frame - command->frameInfo.frameSetData->frameStart);
    command->frameInfo.originFrame = command->frameInfo.frame;
    command->frameInfo.originFrameFraction = command->frameInfo.frameFraction;
    command->frameInfo.originFrameSetFrame = command->frameInfo.frameSetFrame;

    if ((animatorFlags & 1u) != 0 && (data->flags & 0x1eu) != 0) {
      command->frameInfo.flags |= 0x20u;
      std::uint32_t originLoops;
      const bool useFutureFrame = (animatorFlags & 2u) != 0;
      const int originTime = useFutureFrame
                                 ? 2 * timeMSec -
                                       static_cast<int>(lastTimeMSec)
                                 : static_cast<int>(lastTimeMSec);
      if (useFutureFrame)
        command->frameInfo.flags |= 0x40u;
      GetFrame(leaf, originTime, &command->frameInfo.originFrame,
               &command->frameInfo.originFrameFraction, &originLoops,
               ticksPerSec);
      bool resolveOriginSet = true;
      if (!useFutureFrame && originLoops == 0 &&
          command->frameInfo.originFrame == command->frameInfo.frame &&
          command->frameInfo.frame == 0 &&
          command->frameInfo.frameFraction == 0.0f) {
        command->frameInfo.flags |= 0x40u;
        GetFrame(leaf,
                 2 * timeMSec - static_cast<int>(lastTimeMSec),
                 &command->frameInfo.originFrame,
                 &command->frameInfo.originFrameFraction, &originLoops,
                 ticksPerSec);
        resolveOriginSet = false;
      }
      if (resolveOriginSet) {
        const frameSetData_t* currentSet = command->frameInfo.frameSetData;
        if (command->frameInfo.originFrame < currentSet->frameStart ||
            command->frameInfo.originFrame >=
                currentSet->frameStart + currentSet->frameRange) {
          command->frameInfo.originFrameSetData = FrameSetForFrame(
              data, command->frameInfo.originFrame,
              &command->frameInfo.originFrameSetSize);
          command->frameInfo.originFrameSetFrame = static_cast<std::int16_t>(
              command->frameInfo.originFrame -
              command->frameInfo.originFrameSetData->frameStart);
        }
      }
    }
    if ((data->flags & 0x21u) == 0) {
      AddShortBounds(translatedBounds, animation->translatedBounds);
      AddShortBounds(normalizedBounds, animation->normalizedBounds);
      AddShortBounds(frameBounds,
                     (animatorFlags & 1u) != 0 && (data->flags & 0x0eu) != 0
                         ? animation->normalizedBounds
                         : animation->translatedBounds);
    }
  }
  if (leaf->type == idMD6Node::NODE_LEAF_PAUSE) {
    auto* pause = reinterpret_cast<idMD6LeafPause*>(leaf);
    command->frameInfo.animModsSize =
        static_cast<std::uint16_t>(pause->currentDeferred) << 6;
    if (command->frameInfo.animModsSize != 0) {
      const std::size_t activeOffset =
          static_cast<std::size_t>(pause->currentDeferred & 1) * 24u;
      std::memcpy(&command->frameInfo.animMods,
                  pause->animMods + activeOffset,
                  sizeof(command->frameInfo.animMods));
    }
    command->frameInfo.flags |= static_cast<std::uint16_t>(pause->flags);
    if ((pause->flags & 0x10) != 0) {
      *frameBounds = *reinterpret_cast<idBounds*>(pause->bounds);
      *translatedBounds = *frameBounds;
      *normalizedBounds = *frameBounds;
    } else {
      const idBounds& pauseBounds = *reinterpret_cast<idBounds*>(pause->bounds);
      AddBounds(frameBounds, pauseBounds);
      AddBounds(translatedBounds, pauseBounds);
      AddBounds(normalizedBounds, pauseBounds);
    }
    if ((pause->flags & 0x80) != 0)
      command->op = OP_DECODE_FACECOMPOSITE_AND_BLEND;
  }
  return command + 1;
}

md6AnimCommand_t* ParseTree_r(
    const idMD6Model* model, std::uint32_t lastTimeMSec, int timeMSec,
    int ticksPerSec, idMD6FusionBranch* tree, md6AnimCommand_t* command,
    idBounds* frameBounds, idBounds* translatedBounds,
    idBounds* normalizedBounds, md6BlendOutputs_t* blendOutputs,
    std::uint16_t animatorFlags) {
  auto* branch = static_cast<idMD6Branch*>(tree);
  UpdateCurrentAlpha(branch, timeMSec, lastTimeMSec, ticksPerSec);
  if (branchUpdate != nullptr &&
      (branch->type == idMD6Node::NODE_FUSION_BRANCH ||
       branch->type == idMD6Node::NODE_BLEND_BRANCH ||
       branch->type == idMD6Node::NODE_BLENDA_BRANCH)) {
    branchUpdate(branch, false, timeMSec, ticksPerSec,
                 model != nullptr ? model->skeleton : nullptr);
  }
  const int leftTime = branch->leftTimeOverride >= 0
                           ? branch->leftTimeOverride : timeMSec;
  const int rightTime = branch->rightTimeOverride >= 0
                            ? branch->rightTimeOverride : timeMSec;
  md6AnimCommand_t* next = command;
  md6AnimCommand_t* leftStart = ProcessLeft(branch) ? command : nullptr;
  idMD6Node* left = ResolveBestLeaf(branch->left);
  md6AnimCommand_t* leftEnd = nullptr;
  if (left != nullptr) {
    leftEnd = left->type == idMD6Node::NODE_LEAF_PAUSE ||
                      left->type == idMD6Node::NODE_LEAF_PLAY
                  ? SetupCommandFrameInfo(
                        model, leftStart, reinterpret_cast<idMD6LeafPlay*>(left),
                        lastTimeMSec, leftTime, frameBounds, translatedBounds,
                        normalizedBounds, animatorFlags, ticksPerSec)
                  : ParseTree_r(model, lastTimeMSec, leftTime, ticksPerSec,
                                reinterpret_cast<idMD6FusionBranch*>(left),
                                leftStart, frameBounds, translatedBounds,
                                normalizedBounds, blendOutputs, animatorFlags);
  }
  if (leftEnd != nullptr)
    next = leftEnd;

  md6AnimCommand_t* rightStart = ProcessRight(branch) ? next : nullptr;
  idMD6Node* right = ResolveBestLeaf(branch->right);
  md6AnimCommand_t* rightEnd = nullptr;
  if (right != nullptr) {
    rightEnd = right->type == idMD6Node::NODE_LEAF_PAUSE ||
                       right->type == idMD6Node::NODE_LEAF_PLAY
                   ? SetupCommandFrameInfo(
                         model, rightStart,
                         reinterpret_cast<idMD6LeafPlay*>(right),
                         lastTimeMSec, rightTime, frameBounds,
                         translatedBounds, normalizedBounds, animatorFlags,
                         ticksPerSec)
                   : ParseTree_r(model, lastTimeMSec, rightTime, ticksPerSec,
                                 reinterpret_cast<idMD6FusionBranch*>(right),
                                 rightStart, frameBounds, translatedBounds,
                                 normalizedBounds, blendOutputs, animatorFlags);
  }
  if (rightEnd != nullptr)
    next = rightEnd;
  return leftEnd != nullptr && leftEnd->op !=
                                   OP_DECODE_FACECOMPOSITE_AND_BLEND &&
                 rightEnd != nullptr
             ? SetupCommandBlendInfo(next, branch)
             : next;
}

std::uint32_t ParseTree(
    const idMD6Model* model, std::uint32_t lastTimeMSec, int timeMSec,
    int ticksPerSec, idMD6FusionBranch* tree, md6AnimCommand_t* commands,
    int maxCommands, idBounds* frameBounds, idBounds* translatedBounds,
    idBounds* normalizedBounds, md6BlendOutputs_t* blendOutputs,
    std::uint16_t animatorFlags) {
  (void)maxCommands;
  frameBounds->b[0] = {1.0e30f, 1.0e30f, 1.0e30f};
  frameBounds->b[1] = {-1.0e30f, -1.0e30f, -1.0e30f};
  if (blendOutputs != nullptr)
    blendOutputs->fudged = false;
  int count = 0;
  if (tree != nullptr && IsValidNode(tree)) {
    idMD6Node* root = ResolveBestLeaf(tree);
    if (root != nullptr &&
        (root->type == idMD6Node::NODE_LEAF_PAUSE ||
         root->type == idMD6Node::NODE_LEAF_PLAY)) {
      if (SetupCommandFrameInfo(model, commands,
                                reinterpret_cast<idMD6LeafPlay*>(root),
                                lastTimeMSec, timeMSec, frameBounds,
                                translatedBounds, normalizedBounds,
                                animatorFlags, ticksPerSec) != nullptr)
        count = 1;
    } else if (root != nullptr) {
      count = static_cast<int>(ParseTree_r(
          model, lastTimeMSec, timeMSec, ticksPerSec,
          reinterpret_cast<idMD6FusionBranch*>(root), commands, frameBounds,
          translatedBounds, normalizedBounds, blendOutputs, animatorFlags) -
                               commands);
    }
  } else {
    std::memset(commands, 0, sizeof(*commands));
    commands[0].op = OP_DECODE_AND_PUSH;
    count = 1;
  }
  std::memset(&commands[count], 0, sizeof(commands[count]));
  commands[count].op = OP_END;
  return static_cast<std::uint32_t>(count + 1);
}
