#pragma once

#include "engine/animation/md6.h"

#include <cstddef>
#include <cstdint>

class idAnimator_Base;
class idClip;
class idGameTimeManager;
class idJointAnimator;
class idSerializer;
class idTreeAnimator;

class idAnimStack {
public:
  enum animatorFlag_t {
    ANIMATORFLAG_INITIALIZED = 1,
    ANIMATORFLAG_ENABLED = 2,
    ANIMATORFLAG_CONTRIBUTED_LAST_SERIALIZE = 4,
    ANIMATORFLAG_SERIALIZE = 8,
    ANIMATORFLAG_CLIENT_AUTHORITATIVE_ENABLED = 16,
    ANIMATORFLAG_CLIENT_AUTHORITATIVE_CONTRIBUTED = 32,
  };

  void AddAnimator(idAnimator_Base* animator);
  void RemoveAnimator(idAnimator_Base* animator);
  int GetAnimatorIndex(const idAnimator_Base* animator) const;
  void SetAnimatorFlag(const idAnimator_Base* animator,
                       animatorFlag_t flag);
  void ClearAnimatorFlag(const idAnimator_Base* animator,
                         animatorFlag_t flag);
  bool IsAnimatorFlagSet(const idAnimator_Base* animator,
                         animatorFlag_t flag) const;
  idAnimator_Base* GetAnimatorByIndex(int index) const;
  void SerializeSnapshot(idSerializer& serializer, idClip* clip,
                         int curTime, idGameTimeManager* gameTimeManager,
                         idTreeAnimator* treeAnimator);

  idMD6Allocator* GetAllocator() {
    return reinterpret_cast<idMD6Allocator*>(
        reinterpret_cast<std::uint8_t*>(this) + 0x10);
  }
  const idMD6Allocator* GetAllocator() const {
    return reinterpret_cast<const idMD6Allocator*>(
        reinterpret_cast<const std::uint8_t*>(this) + 0x10);
  }

  void* treeAnimator;
  idJointAnimator* jointAnimator;
  std::uint8_t remainingData[0x348];
};

static_assert(offsetof(idAnimStack, treeAnimator) == 0x00,
              "idAnimStack::treeAnimator ABI drift");
static_assert(offsetof(idAnimStack, jointAnimator) == 0x08,
              "idAnimStack::jointAnimator ABI drift");
static_assert(sizeof(idAnimStack) == 0x358, "idAnimStack ABI drift");
