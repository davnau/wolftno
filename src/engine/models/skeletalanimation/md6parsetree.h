#pragma once

#include "engine/animation/md6.h"

#include <cstddef>
#include <cstdint>

struct frameSetData_t {
  std::uint16_t firstOffsets[4];
  std::uint16_t rangeOffsets[4];
  std::uint16_t bitsOffsets[4];
  std::uint16_t nextOffsets[4];
  std::uint16_t totalSize;
  std::int16_t frameStart;
  std::int16_t frameRange;
  std::uint8_t pad[10];
};

struct md6FrameInfo_t {
  const idMD6AnimData* animData;
  const frameSetData_t* frameSetData;
  const void* animMods;
  std::uint16_t flags;
  std::uint16_t weightGroup;
  std::uint16_t animInfoSize;
  std::uint16_t frameSetSize;
  std::uint16_t animModsSize;
  std::int16_t frame;
  float frameFraction;
  std::int16_t frameSetFrame;
  std::int16_t originFrame;
  float originFrameFraction;
  std::int16_t originFrameSetFrame;
  std::uint16_t originFrameSetSize;
  std::uint32_t pad52;
  const frameSetData_t* originFrameSetData;
};

struct md6BlendInfo_t {
  std::uint8_t op;
  std::uint8_t filterGroup;
  std::uint8_t originBlend;
  std::uint8_t pad;
  float currentAlpha;
};

enum md6AnimCommandOp_t : std::int32_t {
  OP_END = 1,
  OP_DECODE_AND_PUSH = 2,
  OP_DECODE_FACECOMPOSITE_AND_BLEND = 3,
  OP_POP_AND_BLEND = 4,
};

struct md6AnimCommand_t {
  md6AnimCommandOp_t op;
  std::uint32_t pad4;
  md6FrameInfo_t frameInfo;
  md6BlendInfo_t blendInfo;
  std::uint32_t debugCommand;
  std::uint32_t pad84;
  const char* animName;
};

struct md6BlendOutputs_t {
  float deltaRotation[4];
  float deltaScale[4];
  float deltaTranslation[4];
  float animRotation[4];
  float animScale[4];
  float animTranslation[4];
  bool done;
  bool fudged;
  std::uint8_t pad[2];
  float time;
  float jointBounds[6];
};

using idMD6ParseTreeModelCompatible = bool (*)(const idMD6Anim*,
                                                const idMD6Model*);
using idMD6ParseTreeBranchUpdate = void (*)(idMD6Branch*, bool,
                                             int timeMSec, int ticksPerSec,
                                             const idMD6Skel* skeleton);
void SetMD6ParseTreeModelCompatible(idMD6ParseTreeModelCompatible compatible);
void SetMD6ParseTreeBranchUpdate(idMD6ParseTreeBranchUpdate update);

void GetFrame(idMD6LeafPlay* leaf, int timeMSec, std::int16_t* frame,
              float* fraction, std::uint32_t* loops,
              std::uint32_t ticksPerSec);
md6AnimCommand_t* SetupCommandBlendInfo(md6AnimCommand_t* command,
                                        idMD6Branch* branch);
md6AnimCommand_t* SetupCommandFrameInfo(
    const idMD6Model* model, md6AnimCommand_t* command, idMD6LeafPlay* leaf,
    std::uint32_t lastTimeMSec, int timeMSec, idBounds* frameBounds,
    idBounds* translatedBounds, idBounds* normalizedBounds,
    std::uint16_t animatorFlags, int ticksPerSec);
md6AnimCommand_t* ParseTree_r(
    const idMD6Model* model, std::uint32_t lastTimeMSec, int timeMSec,
    int ticksPerSec, idMD6FusionBranch* branch, md6AnimCommand_t* command,
    idBounds* frameBounds, idBounds* translatedBounds,
    idBounds* normalizedBounds, md6BlendOutputs_t* blendOutputs,
    std::uint16_t animatorFlags);
std::uint32_t ParseTree(
    const idMD6Model* model, std::uint32_t lastTimeMSec, int timeMSec,
    int ticksPerSec, idMD6FusionBranch* tree, md6AnimCommand_t* commands,
    int maxCommands, idBounds* frameBounds, idBounds* translatedBounds,
    idBounds* normalizedBounds, md6BlendOutputs_t* blendOutputs,
    std::uint16_t animatorFlags);

static_assert(sizeof(frameSetData_t) == 48);
static_assert(sizeof(md6FrameInfo_t) == 64);
static_assert(offsetof(md6FrameInfo_t, flags) == 24);
static_assert(offsetof(md6FrameInfo_t, frame) == 34);
static_assert(offsetof(md6FrameInfo_t, originFrameSetData) == 56);
static_assert(sizeof(md6BlendInfo_t) == 8);
static_assert(sizeof(md6AnimCommand_t) == 96);
static_assert(offsetof(md6AnimCommand_t, frameInfo) == 8);
static_assert(offsetof(md6AnimCommand_t, blendInfo) == 72);
static_assert(offsetof(md6AnimCommand_t, animName) == 88);
static_assert(sizeof(md6BlendOutputs_t) == 128);

