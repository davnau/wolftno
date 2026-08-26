#pragma once

#include "engine/decls/animwebs/declanimweb.h"
#include "engine/gamelib/animstack/animator_channelweight.h"

#include <cstddef>
#include <cstdint>

class idAnimWebEvent;
class idMD6Allocator;

enum animWebEvent_t : int;

class idAnimator_AnimWeb {
public:
  idIndex<std::int16_t, idDeclAnimWeb::invalidSubWebIndex_t>
  GetCurSubWebIndex() const;
  idIndex<std::int16_t, idDeclAnimWeb::invalidStateIndex_t>
  GetCurStateIndex() const;

  void TriggerEvent(const idDeclAnimWebNode* node,
                    animWebEvent_t eventType,
                    const idAnimWebEvent& event, int eventParm);

  std::uint8_t recoveredBase[0x50];
  const idDeclAnimWeb* decl;
  idMD6Allocator* md6Allocator;
  std::uint8_t recovered60[0x150];
  idList<idAnimWebScalar, 33> scalars;
  idScalarMemBlock scalarMem;
  std::uint8_t recovered1D8[0x100];
  idAnimator_ChannelWeight channelWeightAnimator;
  idList<const idDeclMD6*, 33> modelDecls;
  std::uint8_t recovered3A0[8];
};

static_assert(sizeof(idAnimator_AnimWeb) == 0x3A8,
              "AnimWeb animator ABI drift");
static_assert(offsetof(idAnimator_AnimWeb, decl) == 0x50,
              "AnimWeb declaration offset drift");
static_assert(offsetof(idAnimator_AnimWeb, md6Allocator) == 0x58,
              "AnimWeb allocator offset drift");
static_assert(offsetof(idAnimator_AnimWeb, scalars) == 0x1B0,
              "AnimWeb scalar-list offset drift");
static_assert(offsetof(idAnimator_AnimWeb, scalarMem) == 0x1C8,
              "AnimWeb scalar-memory offset drift");
static_assert(offsetof(idAnimator_AnimWeb, channelWeightAnimator) == 0x2D8,
              "AnimWeb channel-weight offset drift");
static_assert(offsetof(idAnimator_AnimWeb, modelDecls) == 0x388,
              "AnimWeb model-list offset drift");
