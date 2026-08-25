#include "tungsten/game/ai/ai2_core_support.h"

// Header-emitted actor support used by the recovered AI translation units.
walkState_t idActor::GetWalkState() const { return walkState; }

void idActor::SetWalkState(walkState_t state) { walkState = state; }

idRecoveredValue idActor::SaveDamageGroup(...) { return {}; }

void idActor::SetLastDamageInfo(
    const idEntity* inflictor,
    const idDeclDamage* damageDefinition) {
  lastDamageInflictor = inflictor;
  lastDamageDefinition = damageDefinition;
}

const idAnimator_AF* idActor::GetAF() const {
  return &articulatedFigureProperties;
}

// DIA: idActor::GetEyePos
void idActor::GetEyePos(idVec3* eyePosition) {
  const idVec3 origin =
      GetPhysics() != nullptr ? GetPhysics()->GetOrigin() : idVec3{};
  *eyePosition = origin + GetEyeOffset();
}

void idActor::GetLastEyePos(idVec3* eyePosition) const {
  *eyePosition = lastEyePosition;
}

void idActor::GetEyeAxis(idMat3* outputEyeAxis) const {
  *outputEyeAxis = eyeAxis;
}

void idActor::SetEyeAxis(const idMat3* inputEyeAxis) {
  eyeAxis = *inputEyeAxis;
}

const idInventoryCollection* idActor::GetInventory() const {
  return &inventory;
}

const idAttachmentCollection* idActor::GetAttachments() const {
  return &attachments;
}

const idList<idEntityPtr<idEntity>, 5>*
idActor::GetTriggerTouchList() const {
  return &touching;
}

idStrId idActor::GetDisplayName() const { return displayName; }

int idActor::GetPosture() const { return posture; }

leanState_t idActor::GetLeanState() const { return leanState; }

void idActor::SetLeanState(leanState_t newLeanState) {
  leanState = newLeanState;
}

weaponReadyState_t idActor::GetWeaponReadyState() const {
  return weaponReadyState;
}

void idActor::SetWeaponReadyState(weaponReadyState_t state) {
  weaponReadyState = state;
}
