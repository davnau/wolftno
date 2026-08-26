#include "tungsten/game/ai/ai2_core_support.h"

#include <unordered_map>

namespace {

struct actorWaterEffectsObservation_t {
  int count{0};
  int physId{-1};
  int bodyId{-1};
};

struct actorModelObservation_t {
  idRenderModel* model{nullptr};
};

struct actorPresentationSources_t {
  idAnimStack* animStack{nullptr};
  const idDeclFX* fxDeclaration{nullptr};
};

std::unordered_map<const idActor*, actorWaterEffectsObservation_t>
    actorWaterEffectsObservations;
std::unordered_map<const idActor*, actorModelObservation_t>
    actorModelObservations;
std::unordered_map<const idActor*, actorPresentationSources_t>
    actorPresentationSources;

}  // namespace

// Header-emitted actor support used by the recovered AI translation units.
walkState_t idActor::GetWalkState() const { return walkState; }

void idActor::SetWalkState(walkState_t state) { walkState = state; }

void idActor::SetModel(idRenderModel* model) {
  actorModelObservations[this].model = model;
}

void idActor::UpdateAttachments() { ++actorUpdateAttachmentsCount; }

void idActor::UpdateEntityAttachmentPos() {
  ++actorUpdateEntityAttachmentPosCount;
}

idRenderModel* idActor::GetActorModelForTest() const {
  return actorModelObservations[this].model;
}

idAnimStack* idActor::GetAnimStack() {
  return actorPresentationSources[this].animStack;
}

const idDeclFX* idActor::GetFXDecl() const {
  return actorPresentationSources[this].fxDeclaration;
}

void idActor::SetAnimStackForTest(idAnimStack* value) {
  actorPresentationSources[this].animStack = value;
}

void idActor::SetFXDeclForTest(const idDeclFX* value) {
  actorPresentationSources[this].fxDeclaration = value;
}

// DIA: idActor::Teleport
void idActor::Teleport(const idVec3& origin, const idAngles& angles) {
  lastTeleportOrigin = origin;
  lastTeleportAngles = angles;
  ++teleportCount;
}

void idActor::ApplyWaterEffects(int physId, int bodyId) {
  actorWaterEffectsObservation_t& observation =
      actorWaterEffectsObservations[this];
  ++observation.count;
  observation.physId = physId;
  observation.bodyId = bodyId;
}

int idActor::GetActorWaterEffectsCountForTest() const {
  return actorWaterEffectsObservations[this].count;
}

int idActor::GetActorWaterEffectsPhysIdForTest() const {
  return actorWaterEffectsObservations[this].physId;
}

int idActor::GetActorWaterEffectsBodyIdForTest() const {
  return actorWaterEffectsObservations[this].bodyId;
}

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
void idActor::GetEyePos(idVec3* eyePosition) const {
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
