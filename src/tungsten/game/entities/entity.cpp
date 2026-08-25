#include "tungsten/game/ai/ai2_core_support.h"

// Header-emitted entity support used by AI.
const idVec3* idEntity::GetPresentableLeftDir() {
  return &presentableAxis[1];
}

const idVec3* idEntity::GetPresentableUpDir() {
  return &presentableAxis[2];
}

idEntity* idEntity::GetTarget(int index) {
  return index >= 0 && index < targets.Num() ? targets[index] : nullptr;
}
