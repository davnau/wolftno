#include "tungsten/game/ai/ai2_actions.h"
#include "tungsten/game/ai/animwebai.h"
#include "tungsten/game/ai/ai2_core_support.h"
#include "tungsten/game/ai/aivoicecontroller.h"
#include "tungsten/game/ai/aiorientation.h"
#include "tungsten/game/ai/aifirecontrol.h"
#include "tungsten/game/ai/aimemory.h"
#include "tungsten/game/ai/aimovestate.h"
#include "tungsten/game/ai/gamestate/aientitystate.h"
#include "tungsten/game/animation/facemgr.h"
#include "engine/animation/md6.h"
#include "shared/idlib/lib.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string_view>

idRecoveredValue idAI2::RecordCoreCall(const char* methodName) {
  const std::string_view call = methodName != nullptr ? methodName : "";
  coreState.lastCall = methodName != nullptr ? methodName : "";
  ++coreState.sequence;
  if (call == "SetSleeping") coreState.sleeping = true;
  if (call == "SetBlinded") coreState.blinded = true;
  if (call == "DeactivateExtendedClipModel") coreState.extendedClipModel = false;
  if (call == "ActivateExtendedClipModel") coreState.extendedClipModel = true;
  if (call == "SetAIClip") coreState.aiClip = true;
  if (call == "ClearAIClip") coreState.aiClip = false;
  if (call == "KillAI" || call == "Killed" || call == "Death") {
    animEventState.dead = true;
    animEventState.health = 0;
  }
  if (call == "GetModifiedSkill") return {coreState.modifiedSkill};
  if (call == "PerceptionFlagIsSet") return {coreState.perceptionFlags != 0};
  if (call == "TeslaIsActive") return {aiEventState.teslaActive};
  if (call == "GetEnableHeadTracking") return {animEventState.headTracking};
  if (call == "IsIncapacitated") return {animEventState.dead};
  return {coreState.sequence};
}

idAI2::idAI2()
    : localVoiceController(std::make_unique<idAIVoiceController>(VC_LOCAL)),
      faceManager(std::make_unique<idFaceMgr>()),
      bodyOrientation(std::make_unique<idAIOrientation>()),
      moveState(std::make_unique<idAIMoveState>()),
      actionFSM(std::make_unique<idLegsFSM>()) {
  focusInfos[0] = std::make_unique<idFocusInfo>();
  focusInfos[1] = std::make_unique<idFocusInfo>();
}
idAI2::~idAI2() = default;

// DIA: idAnimatedEntity::SetModelByName
// DIA: idAIEditable::idAIPerception::~idAIPerception
// DIA: idAnimator_Drone::~idAnimator_Drone
// DIA: idArray<idFocusInfo,2>::~idArray<idFocusInfo,2>
// DIA: idAI2::idAIVolatile::idAIWorldStateInfo::~idAIWorldStateInfo
idAI2::idAIVolatile::idAIWorldStateInfo::~idAIWorldStateInfo() = default;
// DIA: idAI2::idAIVolatile::idPlayerAIControl::~idPlayerAIControl
idAI2::idAIVolatile::idPlayerAIControl::~idPlayerAIControl() = default;
// DIA: idAI2::idAIVolatile::idAIDebugging::~idAIDebugging
idAI2::idAIVolatile::idAIDebugging::~idAIDebugging() = default;
// DIA: idArray<idTrajectory_Parabolic,4>::~idArray<idTrajectory_Parabolic,4>
// DIA: idArray<idAnimWebAI,1>::~idArray<idAnimWebAI,1>
// DIA: idAnimator_TurretNxN<3>::~idAnimator_TurretNxN<3>
// DIA: idAI2::ApplyDamage
idRecoveredValue idAI2::ApplyDamage(...) { return RecordCoreCall("ApplyDamage"); }
// DIA: idAI2::GetModifiedSkill
int idAI2::GetModifiedSkill() { return coreState.modifiedSkill; }
// DIA: idAI2::InitFireControl
void idAI2::InitFireControl() {
  fireControl = std::make_unique<idAIFireControl_Human>();
}
// DIA: idAI2::PerceptionFlagIsSet
bool idAI2::PerceptionFlagIsSet(int flags) {
  return (flags & coreState.perceptionFlags) != 0;
}
// DIA: idAI2::SaveFSM
idRecoveredValue idAI2::SaveFSM(...) { return RecordCoreCall("SaveFSM"); }
// DIA: idScriptFunction::~idScriptFunction
// DIA: idAI2::GetEventResponseFlags
unsigned int idAI2::GetEventResponseFlags() {
  return ~static_cast<unsigned int>(aiEventState.overloadDisableFlags) &
         static_cast<unsigned int>(coreState.aiEventResponseFlags |
                                   aiEventState.overloadEnableFlags);
}
// DIA: idAI2::RespondsToAIEvent
bool idAI2::RespondsToAIEvent(const idAIEvent* event) const {
  const unsigned int responseFlags =
      ~static_cast<unsigned int>(aiEventState.overloadDisableFlags) &
      static_cast<unsigned int>(coreState.aiEventResponseFlags |
                                aiEventState.overloadEnableFlags);
  return (event->GetEventClass() & responseFlags) != 0;
}
// DIA: idBounds::Compare
// DIA: idAI2::DeactivateExtendedClipModel
idRecoveredValue idAI2::DeactivateExtendedClipModel(...) { return RecordCoreCall("DeactivateExtendedClipModel"); }
// DIA: idStr::GetFileName
// DIA: idAI2::SetMovePushStatus
idRecoveredValue idAI2::SetMovePushStatus(...) { return RecordCoreCall("SetMovePushStatus"); }
// DIA: idAI2::SetWorldCollision
void idAI2::SetWorldCollision(bool collideWithWorld) {
  idPhysics* physicsObject = GetPhysics();
  const int numberOfClipModels = physicsObject->GetNumClipModels();
  if (numberOfClipModels == 1) {
    physicsObject->SetClipMask(collideWithWorld ? defaultPhysicsClipMask : 0, 0);
  } else if (numberOfClipModels == 2) {
    physicsObject->SetClipMask(collideWithWorld ? defaultPhysicsClipMask : 0, 0);
    physicsObject->SetClipMask(0, 1);
  }
}
// DIA: idAI2::GetWorldCollisionBounds
const idBounds* idAI2::GetWorldCollisionBounds() {
  return &GetPhysics()->GetAbsBounds();
}
// DIA: idAI2::GetEnemyLookDot
idRecoveredValue idAI2::GetEnemyLookDot(...) { return RecordCoreCall("GetEnemyLookDot"); }
// DIA: idAI2::CalcTurnRadius
float idAI2::CalcTurnRadius(float speed, float turnRate) {
  return ((360.0F / turnRate) * speed) / (3.1415927F * 2.0F);
}
// DIA: OkToFocus
bool OkToFocus(const idVec3* to, const idVec3* from,
               const idVec3* actualPosition) {
  const idVec3 intendedDelta = *to - *from;
  const float intendedLengthSquared = intendedDelta.LengthSqr();
  const float intendedInverseLength =
      1.0F / std::sqrt(std::max(intendedLengthSquared,
                                std::numeric_limits<float>::min()));
  if (intendedInverseLength * intendedLengthSquared <= 16.0F) {
    return false;
  }

  const idVec3 actualDelta = *actualPosition - *from;
  const float actualInverseLength =
      1.0F / std::sqrt(std::max(actualDelta.LengthSqr(),
                                std::numeric_limits<float>::min()));
  return ((actualDelta * actualInverseLength) *
          (intendedDelta * intendedInverseLength)) >
         -0.00000011920929F;
}
// DIA: idAI2::GetFacialAnimationSet
const idDeclFacialAnimationSet* idAI2::GetFacialAnimationSet() const {
  return currentFacialAnimationSet != nullptr ? currentFacialAnimationSet
                                               : defaultFacialAnimationSet;
}
// DIA: idAI2::Draw
idRecoveredValue idAI2::Draw(...) { return RecordCoreCall("Draw"); }
// DIA: idAI2::AlertCycleToSubWeb
int idAI2::AlertCycleToSubWeb(int alertCycle) {
  static const int alertCycleToSubWebMap[11] = {
      24, 0, 3, 1, 4, 8, 8, 24, 24, 24, 24};
  return alertCycle < 11 ? alertCycleToSubWebMap[alertCycle] : 0;
}
// DIA: idAI2::GetAlertCycleFSM
idRecoveredValue idAI2::GetAlertCycleFSM(...) { return RecordCoreCall("GetAlertCycleFSM"); }
// DIA: idAI2::EnableAIEventResponse
void idAI2::EnableAIEventResponse(int flags) {
  coreState.aiEventResponseFlags |= flags;
}
// DIA: idAI2::DisableAIEventResponse
void idAI2::DisableAIEventResponse(int flags) {
  coreState.aiEventResponseFlags &= ~flags;
}
// DIA: idAI2::GetEnableHeadTracking
bool idAI2::GetEnableHeadTracking() { return animEventState.headTracking; }
// DIA: idAI2::SetCoverAction
void idAI2::SetCoverAction(coverAction_t action, int currentTime) {
  coreState.coverAction = action;
  coreState.coverActionTimestamp = currentTime;
}
// DIA: idAI2::GetCoverActionDuration
int idAI2::GetCoverActionDuration(coverAction_t action, int currentTime) {
  return coreState.coverAction == action
             ? currentTime - coreState.coverActionTimestamp
             : 0;
}
// DIA: idAI2::GetBodyOrientation
idAIOrientation* idAI2::GetBodyOrientation() { return bodyOrientation.get(); }
// DIA: idAI2::SetBodyOrientation
void idAI2::SetBodyOrientation(const idMat3& axis) {
  bodyOrientation->Init(axis);
}
// DIA: idAI2::CheckForGoreByName
idRecoveredValue idAI2::CheckForGoreByName(...) { return RecordCoreCall("CheckForGoreByName"); }
// DIA: idAI2::PlayHurtSound
idRecoveredValue idAI2::PlayHurtSound(...) { return RecordCoreCall("PlayHurtSound"); }
// DIA: idAI2::GetRenderParam
idRecoveredValue idAI2::GetRenderParam(...) { return RecordCoreCall("GetRenderParam"); }
// DIA: idRange<float>::GetRatio
template <>
float idRange<float>::GetRatio(float current) const {
  const float range = maxRange - minRange;
  const float clampedCurrent = std::max(0.0F, current - minRange);
  return range != 0.0F ? std::min(1.0F, clampedCurrent / range) : 0.0F;
}
// DIA: idCmdSystem::ArgCompletion_String<&aiWalkModifierNames,7>
// DIA: idList<idHandle<int,enum invalidAIEvent_t,-1>,5>::RemoveIndex
// DIA: idList<idVec3,5>::RemoveIndexFast
// DIA: idFXManager::SetExternalTransform
void idFXManager::SetExternalTransform(const idVec3& position,
                                       const idMat3& rotation) {
  externalPosition = position;
  externalRotation = rotation;
  hasExternalPositionAndRotation = true;
}
// DIA: idBaseHealth::SetCurBaseHealth
void idBaseHealth::SetCurBaseHealth(float health) {
  if (health > 0.0F) {
    ClearDeadFlag();
    SetCurBaseHealth_Impl(health);
  }
}
// DIA: idBaseHealth::DamageShield_Impl
bool idBaseHealth::DamageShield_Impl(float damage, bool allowDeath,
                                     bool overflow, float* remainingDamage,
                                     bool ignoreArmor) {
  (void)overflow;
  return Damage_Impl(damage, allowDeath, remainingDamage, ignoreArmor);
}
// DIA: idCoverActions::Clear
void idCoverActions::Clear() { actionMask &= 0xE0u; }
// DIA: idFSMCallback::~idFSMCallback
// DIA: idFSMCallback::`scalar deleting destructor'
// DIA: idFSMManager::~idFSMManager
// DIA: idFSMManager::`scalar deleting destructor'
// DIA: idAnimatedEntity::GetTreeAnimatorFromPresentable
idTreeAnimator* idAnimatedEntity::GetTreeAnimatorFromPresentable() const {
  if (presentable == nullptr ||
      presentable->GetAnimatedEntityInterface() == nullptr) {
    return nullptr;
  }
  idPresentable* animatedPresentable =
      presentable != nullptr ? presentable->GetAnimatedEntityInterface()
                             : nullptr;
  return static_cast<idTreeAnimator*>(
      animatedPresentable->GetAnimatedTreeAnimator());
}
// DIA: idDamageGroup::HealthRatio
float idDamageGroup::HealthRatio() const {
  return maxHealth > 0.0F ? health / maxHealth : 0.0F;
}
// DIA: idActor::GetWalkState
walkState_t idActor::GetWalkState() const { return walkState; }
// DIA: idActor::SetWalkState
void idActor::SetWalkState(walkState_t state) { walkState = state; }
// DIA: idPlayer::GetUCmdTracker1
idRecoveredValue idPlayer::GetUCmdTracker1(...) { return {}; }
// DIA: idPlayer::GetIsSprinting
idRecoveredValue idPlayer::GetIsSprinting(...) { return {}; }
// DIA: idPlayer::GetFocusTracker
idRecoveredValue idPlayer::GetFocusTracker(...) { return {}; }
// DIA: idPlayer::IsZoomedIn
idRecoveredValue idPlayer::IsZoomedIn(...) { return {}; }
// DIA: idAIFSMCallback::~idAIFSMCallback
// DIA: `idAIFSMCallback::~idAIFSMCallback'::`1'::dtor$0
// DIA: idAIFSMCallback::`scalar deleting destructor'
// DIA: idAI2::UsesTurrets
bool idAI2::UsesTurrets() const {
  return turretAnimationSupported && turretActionSetting;
}
// DIA: idAI2::SetAIClip
void idAI2::SetAIClip() {
  aiClipMask |= 0x400;
  coreState.aiClip = true;
}
// DIA: idAI2::ClearAIClip
void idAI2::ClearAIClip() {
  aiClipMask &= ~0x400;
  coreState.aiClip = false;
}
// DIA: idAction_MoveToPoint::~idAction_MoveToPoint
// DIA: idAI2::TeslaIsActive
bool idAI2::TeslaIsActive() const {
  return teslaDischargeTime >= 0 || teslaExplodeTime >= 0;
}
// DIA: idAI2::IsTeslaDangerous
bool idAI2::IsTeslaDangerous(idVec3* position, float* radius) const {
  if (teslaDischargeTime >= 0 || teslaExplodeTime >= 0) {
    if (teslaHeavyGuardGun) {
      if (teslaExplodeTime >= 0 && teslaEvaluationTime < teslaExplodeTime &&
          teslaDamageDecl != nullptr) {
        *position = teslaExplodePosition;
        *radius = teslaDamageDecl->radius;
        return true;
      }
    } else if (teslaDischargeTime >= 0 &&
               teslaEvaluationTime < teslaDischargeTime) {
      *position = GetPhysics()->GetOrigin();
      *radius = teslaRange;
      return true;
    }
  }
  return false;
}
// DIA: idVec3::DistanceFast
float idVec3::DistanceFast(const idVec3& other) const {
  const float deltaX = other.x - x;
  const float deltaY = other.y - y;
  const float deltaZ = other.z - z;
  const float squaredDistance = deltaX * deltaX + deltaY * deltaY +
                                deltaZ * deltaZ;
  const float reciprocalLength =
      1.0F /
      std::sqrt(std::max(squaredDistance, std::numeric_limits<float>::min()));
  return reciprocalLength * squaredDistance;
}
// DIA: idAI2::SetBlinded
bool idAI2::SetBlinded(bool state, bool forceForever,
                       int blindedOverrideTimeMilliseconds) {
  if (!usesBlindness || volumeDying || animEventState.dead) {
    return false;
  }
  if (!state) {
    if (endBlindedTime == 0) {
      return false;
    }
    const float cooldown = reblindCooldown;
    endBlindedTime = 0;
    coreState.blinded = false;
    if (cooldown != -1.0F) {
      const float scaledCooldown = incomingDamageScale * cooldown;
      endReblindCooldown =
          aiEvaluationTime - static_cast<int>(scaledCooldown * -1000.0F) - 1;
      return true;
    }
    endReblindCooldown = -1;
    return true;
  }
  if (endBlindedTime != 0 || endStunnedTime != 0) {
    return false;
  }
  if (blindedDuration == -1.0F || forceForever) {
    endBlindedTime = -1;
    endReblindCooldown = -1;
    coreState.blinded = true;
    return true;
  }
  if (blindedOverrideTimeMilliseconds == -1) {
    blindedOverrideTimeMilliseconds =
        static_cast<int>(blindedDuration * 1000.0F);
  }
  endReblindCooldown = -1;
  endBlindedTime = blindedOverrideTimeMilliseconds + aiEvaluationTime;
  coreState.blinded = true;
  return true;
}
// DIA: idAI2::OkToLKWBlind
bool idAI2::OkToLKWBlind() const {
  return !animEventState.dead && endBlindedTime == 0 && painAnimationDone;
}
// DIA: idAI2::IsIncapacitated
bool idAI2::IsIncapacitated() const { return endStunnedTime != 0; }
// DIA: idAI2::SetSleeping
void idAI2::SetSleeping(bool sleep) {
  if (!usesSleeping) {
    return;
  }
  if (sleep) {
    if (!coreState.sleeping) {
      coreState.sleeping = true;
    }
  } else if (coreState.sleeping && !volumeDying && !animEventState.dead) {
    coreState.sleeping = false;
  }
}
// DIA: idAI2::ApplyWaterEffects
idRecoveredValue idAI2::ApplyWaterEffects(...) { return RecordCoreCall("ApplyWaterEffects"); }
// DIA: idAI2::InitEncounterGroup
idRecoveredValue idAI2::InitEncounterGroup(...) { return RecordCoreCall("InitEncounterGroup"); }
// DIA: idAI2::OnActivate
idRecoveredValue idAI2::OnActivate(...) { return RecordCoreCall("OnActivate"); }
// DIA: idAI2::Dropped
idRecoveredValue idAI2::Dropped(...) { return RecordCoreCall("Dropped"); }
// DIA: idAI2::IsModesAllowed
bool idAI2::IsModesAllowed(aiModeRestricFlag_t flags) const {
  return (!stationaryMode || (flags & 4) == 0) &&
         (flags & movementModeRestrictionFlags) == 0;
}
// DIA: idAI2::IsModesRestricted
bool idAI2::IsModesRestricted(aiModeRestricFlag_t flags) const {
  return (stationaryMode && (flags & 4) != 0) ||
         (flags & movementModeRestrictionFlags) != 0;
}
// DIA: idAI2::SetWalkState
void idAI2::SetWalkState(walkState_t state) {
  walkState_t clampedState = state;
  if (state == WALKSTATE_RUNNING &&
      (movementModeRestrictionFlags & 8) != 0) {
    clampedState = WALKSTATE_WALKING;
  }
  if (state <= maximumWalkState) {
    if (state < minimumWalkState) {
      clampedState = minimumWalkState;
    }
    idActor::SetWalkState(clampedState);
  } else {
    idActor::SetWalkState(maximumWalkState);
  }
}
// DIA: idAI2::InitScriptObjects
idRecoveredValue idAI2::InitScriptObjects(...) { return RecordCoreCall("InitScriptObjects"); }
// DIA: idAI2::SendGrappleActivationTest
idRecoveredValue idAI2::SendGrappleActivationTest(...) { return RecordCoreCall("SendGrappleActivationTest"); }
// DIA: idAI2::IsWeaponEquipped
bool idAI2::IsWeaponEquipped() const {
  return GetEquippedWeapon(2) != nullptr || GetEquippedWeapon(1) != nullptr;
}
// DIA: idAI2::TurretFire
void idAI2::TurretFire() { lastFireTime = aiEvaluationTime; }
// DIA: idAI2::FindWeaponToDraw
const idWeapon* idAI2::FindWeaponToDraw() const {
  for (int slot = 14; slot >= 0; --slot) {
    const idWeapon* weapon = GetEquippedWeapon(static_cast<unsigned int>(slot));
    if (weapon != nullptr &&
        weapon->GetHolsterSlot() == static_cast<unsigned int>(slot)) {
      return weapon;
    }
  }
  return nullptr;
}
// DIA: idAI2::GetEffectiveAccuracy
int idAI2::GetEffectiveAccuracy() const {
  if ((groupStatusCondition & 0x2000) != 0) {
    return 6;
  }
  return editableAimAccuracy != 0 ? editableAimAccuracy : skillAimAccuracy;
}
// DIA: idAI2::InitHeadTracking
idRecoveredValue idAI2::InitHeadTracking(...) { return RecordCoreCall("InitHeadTracking"); }
// DIA: idAI2::SetModel
idRecoveredValue idAI2::SetModel(...) { return RecordCoreCall("SetModel"); }
// DIA: idAI2::InitAAS
idRecoveredValue idAI2::InitAAS(...) { return RecordCoreCall("InitAAS"); }
// DIA: idAI2::InitMovement
idRecoveredValue idAI2::InitMovement(...) { return RecordCoreCall("InitMovement"); }
// DIA: idAI2::SetPerceptionFlags
void idAI2::SetPerceptionFlags(int flags, bool setFlag) {
  if (setFlag) {
    coreState.perceptionFlags |= static_cast<std::uint64_t>(flags);
    if (fireControl != nullptr) {
      fireControl->SetFireMode(this, FIREMODE_HOLDFIRE);
    }
  } else {
    coreState.perceptionFlags &= ~static_cast<std::uint64_t>(flags);
  }
}
// DIA: idAI2::JobSync
idRecoveredValue idAI2::JobSync(...) { return RecordCoreCall("JobSync"); }
// DIA: idAI2::ApplyImpulse
void idAI2::ApplyImpulse(int physicsId, int bodyId, const idVec3* point,
                         const idVec3* impulse) {
  (void)physicsId;
  if (!animEventState.dead ||
      (articulatedFigureLoaded &&
       (articulatedFigureActive || articulatedFigureClientActive))) {
    GetPhysics()->ApplyImpulse(bodyId, *point, *impulse);
  }
}
// DIA: idAI2::ApplyForce
void idAI2::ApplyForce(int physicsId, int bodyId, const idVec3* point,
                       const idVec3* force) {
  (void)physicsId;
  if (!animEventState.dead ||
      (articulatedFigureLoaded &&
       (articulatedFigureActive || articulatedFigureClientActive))) {
    GetPhysics()->ApplyForce(bodyId, *point, *force);
  }
}
// DIA: idAI2::GetModelTransform
void idAI2::GetModelTransform(idVec3* modelOrigin,
                              idMat3* modelAxis) const {
  if (articulatedFigureActive || articulatedFigureClientActive) {
    *modelOrigin = articulatedFigureOrigin;
    *modelAxis = articulatedFigureAxis;
    return;
  }
  *modelAxis = bodyOrientation->GetAxis() * GetPhysics()->GetAxis();
  *modelOrigin = GetPhysics()->GetOrigin() + (*modelAxis * modelOffset);
}
// DIA: idAI2::GetSoundTransform
void idAI2::GetSoundTransform(idVec3* modelOrigin, idMat3* modelAxis) const {
  idEntity::GetSoundTransform(modelOrigin, modelAxis);
  if (usesDrone) {
    *modelOrigin = dronePresentOrigin;
  }
}
// DIA: idAI2::ShouldLeaveDormancy
bool idAI2::ShouldLeaveDormancy() const {
  return ai_enableDormancy.GetInteger() == 0 ||
         idEntity::ShouldLeaveDormancy();
}
// DIA: idAI2::InternalCallEvent
idRecoveredValue idAI2::InternalCallEvent(...) { return RecordCoreCall("InternalCallEvent"); }
// DIA: idAI2::InternalRespondsTo
idRecoveredValue idAI2::InternalRespondsTo(...) { return RecordCoreCall("InternalRespondsTo"); }
// DIA: idAI2::CheckForOneShotKill
idRecoveredValue idAI2::CheckForOneShotKill(...) { return RecordCoreCall("CheckForOneShotKill"); }
// DIA: idAI2::GetJointIndexFromTrace
idRecoveredValue idAI2::GetJointIndexFromTrace(...) { return RecordCoreCall("GetJointIndexFromTrace"); }
// DIA: idAI2::ApplyFakeDamageImpulse
idRecoveredValue idAI2::ApplyFakeDamageImpulse(...) { return RecordCoreCall("ApplyFakeDamageImpulse"); }
// DIA: idAI2::PlayAdditivePainAnim
idRecoveredValue idAI2::PlayAdditivePainAnim(...) { return RecordCoreCall("PlayAdditivePainAnim"); }
// DIA: idAI2::PlayAdditiveFireAnim
idRecoveredValue idAI2::PlayAdditiveFireAnim(...) { return RecordCoreCall("PlayAdditiveFireAnim"); }
// DIA: idAI2::PlayAdditiveAlarmAnimation
idRecoveredValue idAI2::PlayAdditiveAlarmAnimation(...) { return RecordCoreCall("PlayAdditiveAlarmAnimation"); }
// DIA: idAI2::CheckPain
idRecoveredValue idAI2::CheckPain(...) { return RecordCoreCall("CheckPain"); }
// DIA: idAI2::DeathPending
bool idAI2::DeathPending() const {
  if (deathImpulseIndex >= 0) {
    return true;
  }
  return animEventState.dead;
}
// DIA: idAI2::KillAI
void idAI2::KillAI() {
  if (!animEventState.dead) {
    animEventState.health = 0;
    animEventState.dead = true;
  }
  walkIKEnabled = false;
}
// DIA: idAI2::StartBodyDeathFade
idRecoveredValue idAI2::StartBodyDeathFade(...) { return RecordCoreCall("StartBodyDeathFade"); }
// DIA: idAI2::StartDisintigrateDeathFade
idRecoveredValue idAI2::StartDisintigrateDeathFade(...) { return RecordCoreCall("StartDisintigrateDeathFade"); }
// DIA: idAI2::DisableAnimatorsOnDeath
void idAI2::DisableAnimatorsOnDeath() {
  faceManager->SetEnableAnimation(this, false);
  faceManager->SetEnableEyeTracking(this, false);
  deathAnimatorsEnabled = false;
  walkIKEnabled = false;
}
// DIA: idAI2::DoMeleeDamage
idRecoveredValue idAI2::DoMeleeDamage(...) { return RecordCoreCall("DoMeleeDamage"); }
// DIA: idAI2::DoBlockedMeleeReaction
idRecoveredValue idAI2::DoBlockedMeleeReaction(...) { return RecordCoreCall("DoBlockedMeleeReaction"); }
// DIA: idAI2::GetMeleeInterval
idRecoveredValue idAI2::GetMeleeInterval(...) { return RecordCoreCall("GetMeleeInterval"); }
// DIA: idAI2::IsTargetInMeleeRange
idRecoveredValue idAI2::IsTargetInMeleeRange(...) { return RecordCoreCall("IsTargetInMeleeRange"); }
// DIA: idAI2::MeleeMaskForTarget
idRecoveredValue idAI2::MeleeMaskForTarget(...) { return RecordCoreCall("MeleeMaskForTarget"); }
// DIA: idAI2::GetDirectionTo
idRecoveredValue idAI2::GetDirectionTo(...) { return RecordCoreCall("GetDirectionTo"); }
// DIA: idAI2::GetDirectionToForReactions
idRecoveredValue idAI2::GetDirectionToForReactions(...) { return RecordCoreCall("GetDirectionToForReactions"); }
// DIA: idAI2::GetTakedownName
idRecoveredValue idAI2::GetTakedownName(...) { return RecordCoreCall("GetTakedownName"); }
// DIA: idAI2::TakedownWindoIsOpen
idRecoveredValue idAI2::TakedownWindoIsOpen(...) { return RecordCoreCall("TakedownWindoIsOpen"); }
// DIA: idAI2::IsCrosshairSubdued
idRecoveredValue idAI2::IsCrosshairSubdued(...) { return RecordCoreCall("IsCrosshairSubdued"); }
// DIA: idAI2::HandleLoot
idRecoveredValue idAI2::HandleLoot(...) { return RecordCoreCall("HandleLoot"); }
// DIA: idAI2::SetDelayedVoice
idRecoveredValue idAI2::SetDelayedVoice(...) { return RecordCoreCall("SetDelayedVoice"); }
// DIA: idAI2::CheckDelayedVoice
idRecoveredValue idAI2::CheckDelayedVoice(...) { return RecordCoreCall("CheckDelayedVoice"); }
// DIA: idAI2::IsThrowingGrenade
bool idAI2::IsThrowingGrenade() const {
  return usesGrenades && grenadeItemDeclaration != nullptr &&
         static_cast<unsigned int>(grenadeOverrideAnimation - 27) <= 4U;
}
// DIA: idAI2::GetPlayerViewDataFitness
float idAI2::GetPlayerViewDataFitness() const {
  if (playerViewRange < 200.0F || playerViewRange > 1024.0F) {
    return 0.0F;
  }
  float spottedScale = 0.0F;
  const float spottedSeconds =
      static_cast<float>(playerViewSpottedTime) * 0.001F;
  if (spottedSeconds >= 0.0F) {
    spottedScale = spottedSeconds <= 1.0F ? spottedSeconds : 1.0F;
  }
  return spottedScale * playerViewDot * 1024.0F /
         (1024.0F - playerViewRange);
}
// DIA: idAI2::CanReceiveAIEvents
bool idAI2::CanReceiveAIEvents(int currentTime) const {
  return !IsHidden() && currentTime >= nextAIEventTime;
}
// DIA: idAI2::OnAIEvent_VehicleSound
idRecoveredValue idAI2::OnAIEvent_VehicleSound(...) { return RecordCoreCall("OnAIEvent_VehicleSound"); }
// DIA: idAI2::OnAIEvent_Body
idRecoveredValue idAI2::OnAIEvent_Body(...) { return RecordCoreCall("OnAIEvent_Body"); }
// DIA: idAI2::OnAIEvent_PlayerInteraction
idRecoveredValue idAI2::OnAIEvent_PlayerInteraction(...) { return RecordCoreCall("OnAIEvent_PlayerInteraction"); }
// DIA: idAI2::SetSubWeb
idRecoveredValue idAI2::SetSubWeb(...) { return RecordCoreCall("SetSubWeb"); }
// DIA: idAI2::UpdateAttachments
idRecoveredValue idAI2::UpdateAttachments(...) { return RecordCoreCall("UpdateAttachments"); }
// DIA: `idAI2::UpdateAttachments'::`1'::dtor$0
// DIA: idAI2::ActivateExtendedClipModel
idRecoveredValue idAI2::ActivateExtendedClipModel(...) { return RecordCoreCall("ActivateExtendedClipModel"); }
// DIA: `idAI2::ActivateExtendedClipModel'::`1'::dtor$0
// DIA: `idAI2::ActivateExtendedClipModel'::`1'::dtor$1
// DIA: idAI2::AimDebug
idRecoveredValue idAI2::AimDebug(...) { return RecordCoreCall("AimDebug"); }
// DIA: idAI2::LookDebug
idRecoveredValue idAI2::LookDebug(...) { return RecordCoreCall("LookDebug"); }
// DIA: idAI2::Debug_ScenePoints
idRecoveredValue idAI2::Debug_ScenePoints(...) { return RecordCoreCall("Debug_ScenePoints"); }
// DIA: idAI2::idVisibilityObscurity::GetDestination
idRecoveredValue idAI2::idVisibilityObscurity::GetDestination(...) { return {}; }
// DIA: idAI2::CanTaunt
bool idAI2::CanTaunt() const {
  return (tauntDodgeFlags & 0x200) != 0 &&
         aiEvaluationTime > nextAITauntTime;
}
// DIA: idAI2::GetTauntInterval
int idAI2::GetTauntInterval() {
  const float intervalRange = tauntMaximumInterval - tauntMinimumInterval;
  sharedAIRandomSeed = 1664525U * sharedAIRandomSeed + 1013904223U;
  const float randomFraction =
      static_cast<float>((sharedAIRandomSeed >> 10U) & 0x7FFFU) *
      0.000030518509F;
  return static_cast<int>((randomFraction * intervalRange) * 1000.0F +
                          tauntMinimumInterval * 1000.0F);
}
// DIA: idAI2::GetEncounterGroup
idEncounterGroup* idAI2::GetEncounterGroup() const {
  return aiEventEncounterGroup;
}
// DIA: idAI2::GetWalkState
walkState_t idAI2::GetWalkState() const {
  const int forcedWalkState = ai_forceWalk.GetInteger();
  if (forcedWalkState > 0 && forcedWalkState < 4) {
    return static_cast<walkState_t>(forcedWalkState);
  }
  return idActor::GetWalkState();
}
// DIA: idAI2::SetSolid
idRecoveredValue idAI2::SetSolid(...) { return RecordCoreCall("SetSolid"); }
// DIA: idAI2::SetAimFocusEntity
idRecoveredValue idAI2::SetAimFocusEntity(...) { return RecordCoreCall("SetAimFocusEntity"); }
// DIA: idAI2::SetAimFocusAimPoint
void idAI2::SetAimFocusAimPoint(aimPoint_t point) {
  aimFocusAimPoint = point;
}
// DIA: idAI2::GetMissTimeAimPosition
idRecoveredValue idAI2::GetMissTimeAimPosition(...) { return RecordCoreCall("GetMissTimeAimPosition"); }
// DIA: idAI2::ReloadWeapon
idRecoveredValue idAI2::ReloadWeapon(...) { return RecordCoreCall("ReloadWeapon"); }
// DIA: idAI2::GetRepullTriggerInterval
idRecoveredValue idAI2::GetRepullTriggerInterval(...) { return RecordCoreCall("GetRepullTriggerInterval"); }
// DIA: idAI2::GetBurstDuration
idRecoveredValue idAI2::GetBurstDuration(...) { return RecordCoreCall("GetBurstDuration"); }
// DIA: idAI2::IsInsideTrailUpdateRange
bool idAI2::IsInsideTrailUpdateRange(
    const idVec3* suspiciousPosition) const {
  float range = 96.0F;
  if (usesCloseCover) {
    range = std::max(96.0F, closeMinRange);
  }
  return range >= GetPhysics()->GetOrigin().DistanceFast(*suspiciousPosition);
}
// DIA: idAI2::IsRecentlySpawned
bool idAI2::IsRecentlySpawned(float seconds) const {
  return aiEvaluationTime - firstThinkTimestamp <=
         static_cast<int>(seconds * 1000.0F);
}
// DIA: idAI2::IsRecentlyGroupSpotted
idRecoveredValue idAI2::IsRecentlyGroupSpotted(...) { return RecordCoreCall("IsRecentlyGroupSpotted"); }
// DIA: idAI2::GetSpottedDuration
idRecoveredValue idAI2::GetSpottedDuration(...) { return RecordCoreCall("GetSpottedDuration"); }
// DIA: idAI2::Event_DetectPlayer
idRecoveredValue idAI2::Event_DetectPlayer(...) { return RecordCoreCall("Event_DetectPlayer"); }
// DIA: idAI2::GetEnemyInsideMaxFireRange
bool idAI2::GetEnemyInsideMaxFireRange(const idEntity* enemy) const {
  if (enemy == nullptr || maxFireRange <= 0.0F) {
    return false;
  }
  const idVec3 origin = usesDrone ? dronePresentOrigin
                                  : GetPhysics()->GetOrigin();
  const idVec3 delta = enemy->GetPhysics()->GetOrigin() - origin;
  return maxFireRange * maxFireRange >= delta.LengthSqr();
}
// DIA: idAI2::GetEnemyInsideGoNutsRange
bool idAI2::GetEnemyInsideGoNutsRange() const {
  return currentEnemyRange > 0.0F && goNutsRange >= currentEnemyRange;
}
// DIA: idAI2::SetFOV
void idAI2::SetFOV(const float* fieldOfView) {
  aiEventState.fieldOfView = *fieldOfView;
  float halfFOVRadians = (*fieldOfView * 0.5F) * 0.017453292F;
  if (halfFOVRadians < 0.0F || halfFOVRadians >= 6.2831855F) {
    halfFOVRadians -=
        std::floor(halfFOVRadians * 0.15915494F) * 6.2831855F;
  }
  fieldOfViewDot = std::cos(halfFOVRadians);
}
// DIA: idAI2::Teleport
idRecoveredValue idAI2::Teleport(...) { return RecordCoreCall("Teleport"); }
// DIA: idAI2::SetEnableAutoFocus
void idAI2::SetEnableAutoFocus(bool enable) {
  if (!enable || autoFocusEnabled) {
    autoFocusEnabled = enable;
  } else {
    lookFocusTimeout = aiEvaluationTime;
    autoFocusEnabled = enable;
  }
}
// DIA: idAI2::ShowFlareEffect
idRecoveredValue idAI2::ShowFlareEffect(...) { return RecordCoreCall("ShowFlareEffect"); }
// DIA: idAI2::StartWeaponFX
idRecoveredValue idAI2::StartWeaponFX(...) { return RecordCoreCall("StartWeaponFX"); }
// DIA: idAI2::StopWeaponFX
idRecoveredValue idAI2::StopWeaponFX(...) { return RecordCoreCall("StopWeaponFX"); }
// DIA: idCheckSurroundingsState::UpdateLookInterests
// DIA: idCheckSurroundingsState::FindNetDirectionInterest
// DIA: idCheckSurroundingsState::GetLookedAtElapsedTime
// DIA: idCheckSurroundingsState::SetLastLookedAtTime
// DIA: `idAI2::FindClosestFocus'::`2'::idLookDebug::~idLookDebug
// DIA: idAI2::SetSpeakingVO
void idAI2::SetSpeakingVO() {
  if (mutantActorClass) {
    return;
  }
  if (speakingMoodCapability) {
    faceManager->SetEnableMoodAnimation(this, true);
  }
  if (speakingLipSyncCapability) {
    faceManager->SetEnableLipSyncAnimation(this, true);
    faceManager->StartLipsync();
  }
  if (speakingGestureCapability) {
    faceManager->SetEnableGestureAnimation(this, true);
  }
  if (speakingEyeTrackingCapability) {
    faceManager->SetEnableEyeTracking(this, true);
  }
  if (speakingBlinkingCapability) {
    faceManager->SetEnableBlinking(this, true);
  }
  if (speakingLidDeformationCapability) {
    faceManager->SetEnableLidDeformation(this, true);
  }
}
// DIA: idAI2::SetActionScript
idRecoveredValue idAI2::SetActionScript(...) { return RecordCoreCall("SetActionScript"); }
// DIA: idAI2::EndActionScript
void idAI2::EndActionScript() {
  if (actionScriptCount <= 0) {
    actionScriptRunning = false;
    actionScriptFlags = 0;
  }
}
// DIA: idAI2::OnActionScriptFinished
void idAI2::OnActionScriptFinished(int currentTime) {
  (void)currentTime;
  painAllowedAfterActionScript =
      !behaviorDeclaration->disablePainAfterActionScript;
}
// DIA: idAI2::SetActionScriptFlag
void idAI2::SetActionScriptFlag(int flags, bool setFlag) {
  if (setFlag) {
    actionScriptFlags |= flags;
  } else {
    actionScriptFlags &= ~flags;
  }
}
// DIA: idAI2::ActionScriptFlagIsSet
bool idAI2::ActionScriptFlagIsSet(int flags) const {
  return actionScriptRunning && (flags & actionScriptFlags) != 0;
}
// DIA: idAI2::GetActionFSM
idLegsFSM* idAI2::GetActionFSM() { return actionFSM.get(); }
// DIA: idAI2::GetActionStatus
int idAI2::GetActionStatus() const { return actionFSM->GetStatus(); }
// DIA: idSpawnFocusWithRequirement::RequirementFilled
// DIA: idAIFSMCallback::OnTransition
// DIA: idAIFSMCallback::OnRestart
// DIA: idAIFSMCallback::OnError
// DIA: idAI2::HideMeshNoAutoRestore
idRecoveredValue idAI2::HideMeshNoAutoRestore(...) { return RecordCoreCall("HideMeshNoAutoRestore"); }
// DIA: idAI2::PerformStandUpAction
idRecoveredValue idAI2::PerformStandUpAction(...) { return RecordCoreCall("PerformStandUpAction"); }
// DIA: `idAI2::PerformStandUpAction'::`1'::dtor$0
// DIA: idAI2::PerformCrouchAction
idRecoveredValue idAI2::PerformCrouchAction(...) { return RecordCoreCall("PerformCrouchAction"); }
// DIA: idAI2::PerformOverrideAnimAction
idRecoveredValue idAI2::PerformOverrideAnimAction(...) { return RecordCoreCall("PerformOverrideAnimAction"); }
// DIA: `idAI2::PerformOverrideAnimAction'::`1'::dtor$0
// DIA: `idAI2::PerformOverrideAnimAction'::`1'::dtor$1
// DIA: `idAI2::PerformOverrideAnimAction'::`1'::dtor$2
// DIA: idAI2::SetIdealAimFocusPoint
void idAI2::SetIdealAimFocusPoint(const idVec3& point, int timeout,
                                  aiFocus_t focusType) {
  focusInfos[0]->SetExplicitFocusPoint(nullptr, point, false, timeout,
                                       focusType);
}
// DIA: idAI2::SetIdealLookFocusPoint
void idAI2::SetIdealLookFocusPoint(const idVec3& point, int timeout,
                                   aiFocus_t focusType) {
  focusInfos[1]->SetExplicitFocusPoint(nullptr, point, false, timeout,
                                       focusType);
}
// DIA: idAI2::ClearAimFocus
void idAI2::ClearAimFocus() { focusInfos[0]->Clear(nullptr); }
// DIA: idAI2::ClearLookFocus
void idAI2::ClearLookFocus() { focusInfos[1]->Clear(nullptr); }
// DIA: idAI2::SetEnableHeadTracking
void idAI2::SetEnableHeadTracking(bool enable) {
  animEventState.headTracking = enable;
  for (const auto& focus : focusInfos) {
    focus->SetEnableTracking(nullptr, enable);
  }
}
// DIA: idAI2::SetLockHeadTracking
void idAI2::SetLockHeadTracking(bool lock) {
  for (const auto& focus : focusInfos) {
    focus->SetLockTracking(lock);
  }
  headTrackingLocked = lock;
}
// DIA: idAI2::IsInCover
bool idAI2::IsInCover() const {
  const auto* combatScenePoint =
      dynamic_cast<const idScenePoint_Combat*>(currentScenePoint);
  return combatScenePoint != nullptr && combatScenePoint->IsTactical() &&
         currentScenePointActivationTime > 0 &&
         coreState.coverAction == COVERACTION_HIDE;
}
// DIA: idAI2::IsBlindFiring
bool idAI2::IsBlindFiring() const {
  const auto* combatScenePoint =
      dynamic_cast<const idScenePoint_Combat*>(currentScenePoint);
  if (combatScenePoint == nullptr || !combatScenePoint->IsTactical() ||
      currentScenePointActivationTime <= 0) {
    return false;
  }

  const coverAction_t action = coreState.coverAction;
  return action == COVERACTION_BLINDFIRE_OVER ||
         action == COVERACTION_BLINDFIRE_LEFT ||
         action == COVERACTION_BLINDFIRE_RIGHT;
}
// DIA: idAI2::GetAnimationDelta
bool idAI2::GetAnimationDelta(const idMD6Anim* animation,
                              idVec3* outDelta) const {
  if (animation == nullptr) {
    return false;
  }

  const float* startDelta = animation->animData->startDelta;
  const float* endDelta = animation->animData->endDelta;
  const idVec3 delta{endDelta[8] - startDelta[8],
                     endDelta[9] - startDelta[9],
                     endDelta[10] - startDelta[10]};
  const float lengthSquared =
      startDelta[0] * startDelta[0] + startDelta[1] * startDelta[1] +
      startDelta[2] * startDelta[2] + startDelta[3] * startDelta[3];
  const float inverseLength =
      1.0F / std::sqrt(std::max(lengthSquared,
                               std::numeric_limits<float>::min()));
  const idQuat rotation{-startDelta[0] * inverseLength,
                        -startDelta[1] * inverseLength,
                        -startDelta[2] * inverseLength, startDelta[3]};

  const float xx = rotation.x * rotation.x;
  const float yy = rotation.y * rotation.y;
  const float zz = rotation.z * rotation.z;
  const float ww = rotation.w * rotation.w;
  const float xy2 = rotation.x * rotation.y * 2.0F;
  const float xz2 = rotation.x * rotation.z * 2.0F;
  const float yz2 = rotation.y * rotation.z * 2.0F;
  const float wx2 = rotation.w * rotation.x * 2.0F;
  const float wy2 = rotation.w * rotation.y * 2.0F;
  const float wz2 = rotation.w * rotation.z * 2.0F;
  outDelta->x = (((xx - yy) - zz) + ww) * delta.x +
                (wz2 + xy2) * delta.y + (xz2 - wy2) * delta.z;
  outDelta->z = (wy2 + xz2) * delta.x + (yz2 - wx2) * delta.y +
                ((zz - (yy + xx)) + ww) * delta.z;
  outDelta->y = (((yy - xx) - zz) + ww) * delta.y +
                (xy2 - wz2) * delta.x + (yz2 + wx2) * delta.z;
  return true;
}
// DIA: idAI2::GetRunCycleIndexForType
int idAI2::GetRunCycleIndexForType(runIndexType_t type) const {
  switch (type) {
    case ANIMWEBAI_RUNINDEXTYPE_NORMAL:
      return 0;
    case ANIMWEBAI_RUNINDEXTYPE_INJURED:
      idLib::Warning("idAI2::GetRunCycleIndexForType run index type "
                     "ANIMWEBAI_RUNINDEXTYPE_INJURED cannot be selected this way");
      return -1;
    case ANIMWEBAI_RUNINDEXTYPE_SCRAMBLE:
      return behaviorDeclaration->runTypeBehaviors.scrambleRunIndex;
    case ANIMWEBAI_RUNINDEXTYPE_FAR:
      return behaviorDeclaration->runTypeBehaviors.farRunIndex;
    case ANIMWEBAI_RUNINDEXTYPE_GUNDOWN:
      return behaviorDeclaration->runTypeBehaviors.gunDownIndex;
    case ANIMWEBAI_RUNINDEXTYPE_NARROW:
      return behaviorDeclaration->runTypeBehaviors.narrowIndex;
    case ANIMWEBAI_RUNINDEXTYPE_UP_STAIRS:
      return behaviorDeclaration->runTypeBehaviors.upStairsIndex;
    case ANIMWEBAI_RUNINDEXTYPE_DOWN_STAIRS:
      return behaviorDeclaration->runTypeBehaviors.downStairsIndex;
    case ANIMWEBAI_RUNINDEXTYPE_DUCK:
      return behaviorDeclaration->runTypeBehaviors.duckIndex;
    default:
      return -1;
  }
}
// DIA: idAI2::GetIdleIndexForType
int idAI2::GetIdleIndexForType(runIndexType_t type) const {
  switch (type) {
    case ANIMWEBAI_RUNINDEXTYPE_INJURED:
      idLib::Warning("idAI2::GetIdleIndexForType idle index type "
                     "ANIMWEBAI_RUNINDEXTYPE_INJURED cannot be selected this way");
      return -1;
    case ANIMWEBAI_RUNINDEXTYPE_GUNDOWN:
      return behaviorDeclaration->idleBehaviors.gunDownIndex;
    case ANIMWEBAI_RUNINDEXTYPE_NARROW:
      return behaviorDeclaration->idleBehaviors.narrowIndex;
    case ANIMWEBAI_RUNINDEXTYPE_DUCK:
      return behaviorDeclaration->idleBehaviors.duckIndex;
    default:
      return 0;
  }
}
// DIA: idAI2::PlayAnimWebPathPerfect
idRecoveredValue idAI2::PlayAnimWebPathPerfect(...) { return RecordCoreCall("PlayAnimWebPathPerfect"); }
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$0
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$1
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$2
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$3
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$4
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$5
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$6
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$7
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$8
// DIA: GetCoverTransionAnimName
idRecoveredValue GetCoverTransionAnimName(...) { return {}; }
// DIA: `GetCoverTransionAnimName'::`1'::dtor$0
// DIA: `GetCoverTransionAnimName'::`1'::dtor$1
// DIA: `GetCoverTransionAnimName'::`1'::dtor$2
// DIA: `GetCoverTransionAnimName'::`1'::dtor$3
// DIA: `GetCoverTransionAnimName'::`1'::dtor$4
// DIA: idAI2::AllocPresentable
idRecoveredValue idAI2::AllocPresentable(...) { return RecordCoreCall("AllocPresentable"); }
// DIA: idAI2::DisableInfluenceTrail
idRecoveredValue idAI2::DisableInfluenceTrail(...) { return RecordCoreCall("DisableInfluenceTrail"); }
// DIA: idAI2::IsOnElevator
idRecoveredValue idAI2::IsOnElevator(...) { return RecordCoreCall("IsOnElevator"); }
// DIA: idAI2::StartRagdoll
idRecoveredValue idAI2::StartRagdoll(...) { return RecordCoreCall("StartRagdoll"); }
// DIA: idAI2::StopRagdoll
idRecoveredValue idAI2::StopRagdoll(...) { return RecordCoreCall("StopRagdoll"); }
// DIA: idAI2::BeginSync
idRecoveredValue idAI2::BeginSync(...) { return RecordCoreCall("BeginSync"); }
// DIA: idAI2::EndSync
idRecoveredValue idAI2::EndSync(...) { return RecordCoreCall("EndSync"); }
// DIA: idAI2::GetMoveOrientation
idAIOrientation* idAI2::GetMoveOrientation() {
  return moveState->GetMoveOrientation();
}
// DIA: idAI2::IsMoveDone
bool idAI2::IsMoveDone(std::int64_t checkAlignment) const {
  (void)checkAlignment;
  return moveState->GetMoveStatus() == MOVE_STATUS_DONE;
}
// DIA: idAI2::GetMoveInterface
idAIMoveState* idAI2::GetMoveInterface() { return moveState.get(); }
// DIA: idAI2::PlayDeathSound
idRecoveredValue idAI2::PlayDeathSound(...) { return RecordCoreCall("PlayDeathSound"); }
// DIA: idAI2::UpdateWetness
idRecoveredValue idAI2::UpdateWetness(...) { return RecordCoreCall("UpdateWetness"); }
// DIA: idAI2::UpdateRenderParamBlends
idRecoveredValue idAI2::UpdateRenderParamBlends(...) { return RecordCoreCall("UpdateRenderParamBlends"); }
// DIA: idAIScriptProxy::PauseFiberTillActionCompletes
// DIA: idAIScriptProxy::PauseThreadTillActionCompletes
// DIA: idAnimator_TurretNxN<3>::`scalar deleting destructor'
// DIA: idAnimator_TurretNxN<3>::SetParameters
// DIA: idEntityPtr<idEntitySpriteAnimator>::GetEntity
// DIA: idEntityPtr<idTarget_AlarmSystem>::GetEntity
// DIA: idInterpolate<int>::GetCurrentValue
// DIA: idHashTableT<int,idAIAction * __ptr64>::Clear
// DIA: idPresentable::SetModelFade
// DIA: idEntity::GetPresentableLeftDir
std::intptr_t idEntity::GetPresentableLeftDir(...) { return {}; }
// DIA: idEntity::GetPresentableUpDir
std::intptr_t idEntity::GetPresentableUpDir(...) { return {}; }
// DIA: idActor::SaveDamageGroup
idRecoveredValue idActor::SaveDamageGroup(...) { return {}; }
// DIA: idScenePoint::Disable
void idScenePoint::Disable(int currentTime, float disableSeconds) {
  currentUser = nullptr;
  requestTime = disableSeconds < 0.0F
                    ? -1
                    : currentTime - static_cast<int>(disableSeconds * -1000.0F);
}
// DIA: idScenePoint_Combat::HasEnterAnimation
bool idScenePoint_Combat::HasEnterAnimation() const {
  return !enterAnimation.IsEmpty() || extraEnterAnimationCount > 0;
}
// DIA: idEncounterGroup::GetCurrentCombatStage
idCombatStage* idEncounterGroup::GetCurrentCombatStage() {
  return curCombatStage < 0 || curCombatStage >= combatStages.Num()
             ? nullptr
             : &combatStages[curCombatStage];
}
const idCombatStage* idEncounterGroup::GetCurrentCombatStage() const {
  return curCombatStage < 0 || curCombatStage >= combatStages.Num()
             ? nullptr
             : &combatStages[curCombatStage];
}
// DIA: idAIEditable::idAIActionSettings::Init
// DIA: idAI2::GetCurrentCombatStage
const idCombatStage* idAI2::GetCurrentCombatStage() const {
  const idEncounterGroup* group = GetEncounterGroup();
  return group != nullptr ? group->GetCurrentCombatStage() : nullptr;
}
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::SetDeadFlag
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::Damage_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::Kill_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::Reset_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::HealComponent_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::GetTotalCurHealth_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::GetTotalMaxHealth_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::ApplyDamageWithOverflow
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::ApplyDamageAndClampToZero
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::DamageComponent_Impl
// DIA: idAIEditable::idAIActionSettings::idAICustomRobot::idAICustomRobot
// DIA: idAIDeathInfo::~idAIDeathInfo
// DIA: idAI2::SetRobotPowerOut
void idAI2::SetRobotPowerOut() {
  if (robotState.isCharging) return;
  if (robotState.powerOutTriggerValid) {
    ++robotState.powerOutTriggerActivations;
  }
  robotState.cooldownEffectActive = true;
  robotState.cooldownEffectStartTime = gameLocal->GetGameTimeMs();
  robotState.currentStageHealth =
      robotState.chargeHealthFraction * robotState.stageHealth;
  if (!robotState.autoRecharge) {
    coreState.perceptionFlags |= 1U;
    if (fireControl != nullptr) {
      fireControl->SetFireMode(this, FIREMODE_HOLDFIRE);
    }
  }
  robotState.usesLookScenePoints = false;
  robotState.chargeDoneTime = -1;
  robotState.isCharging = true;
  if (robotState.useRechargeAnimation && endBlindedTime == 0 &&
      endStunnedTime == 0) {
    robotState.chargingAnimationActive = true;
  }
}
// DIA: idAI2::SetRobotChargingDone
void idAI2::SetRobotChargingDone() {
  if (!robotState.isCharging) return;
  if (robotState.chargingDoneTriggerValid) {
    ++robotState.chargingDoneTriggerActivations;
  }
  robotState.cooldownEffectActive = false;
  robotState.cooldownEffectStopTime = gameLocal->GetGameTimeMs();
  if (!robotState.autoRecharge) coreState.perceptionFlags &= ~1ULL;
  robotState.usesLookScenePoints = true;
  robotState.currentStageHealth = robotState.stageHealth;
  robotState.chargeStartTime = -1;
  robotState.chargeDoneTime = -1;
  robotState.isCharging = false;
  if (robotState.useRechargeAnimation && !animEventState.dead) {
    robotState.chargingAnimationActive = false;
  }
}
// DIA: idAI2::SetEMPStunned
idRecoveredValue idAI2::SetEMPStunned(...) { return RecordCoreCall("SetEMPStunned"); }
// DIA: idAI2::InhibitRunning
idRecoveredValue idAI2::InhibitRunning(...) { return RecordCoreCall("InhibitRunning"); }
// DIA: idAI2::InitFSMs
idRecoveredValue idAI2::InitFSMs(...) { return RecordCoreCall("InitFSMs"); }
// DIA: idAI2::UpdateMeleeCapabilities
idRecoveredValue idAI2::UpdateMeleeCapabilities(...) { return RecordCoreCall("UpdateMeleeCapabilities"); }
// DIA: idAI2::EquipItem
idRecoveredValue idAI2::EquipItem(...) { return RecordCoreCall("EquipItem"); }
// DIA: idAI2::UnequipItem
idRecoveredValue idAI2::UnequipItem(...) { return RecordCoreCall("UnequipItem"); }
// DIA: idAI2::SetAccuracy
idRecoveredValue idAI2::SetAccuracy(...) { return RecordCoreCall("SetAccuracy"); }
// DIA: idAI2::InitPhysics
idRecoveredValue idAI2::InitPhysics(...) { return RecordCoreCall("InitPhysics"); }
// DIA: `idAI2::InitPhysics'::`1'::dtor$0
// DIA: `idAI2::InitPhysics'::`1'::dtor$1
// DIA: idAI2::InitPerception
idRecoveredValue idAI2::InitPerception(...) { return RecordCoreCall("InitPerception"); }
// DIA: idAI2::OnHide
idRecoveredValue idAI2::OnHide(...) { return RecordCoreCall("OnHide"); }
// DIA: idAI2::UpdateSpriteAnimator
idRecoveredValue idAI2::UpdateSpriteAnimator(...) { return RecordCoreCall("UpdateSpriteAnimator"); }
// DIA: idAI2::OnIgnoreDamage
idRecoveredValue idAI2::OnIgnoreDamage(...) { return RecordCoreCall("OnIgnoreDamage"); }
// DIA: idAI2::UpdateDrop
idRecoveredValue idAI2::UpdateDrop(...) { return RecordCoreCall("UpdateDrop"); }
// DIA: idAI2::PlayAdditivePain
idRecoveredValue idAI2::PlayAdditivePain(...) { return RecordCoreCall("PlayAdditivePain"); }
// DIA: idAI2::PushFromCollision
idRecoveredValue idAI2::PushFromCollision(...) { return RecordCoreCall("PushFromCollision"); }
// DIA: idAI2::CheckDeathCollision
idRecoveredValue idAI2::CheckDeathCollision(...) { return RecordCoreCall("CheckDeathCollision"); }
// DIA: idAI2::DoMeleeDamage
idRecoveredValue idAI2::DoMeleeDamage(idRecoveredOverloadTag<1>) { return RecordCoreCall("DoMeleeDamage"); }
// DIA: idAI2::GetMeleeDirection
idRecoveredValue idAI2::GetMeleeDirection(...) { return RecordCoreCall("GetMeleeDirection"); }
// DIA: idAI2::IsFacingMe
idRecoveredValue idAI2::IsFacingMe(...) { return RecordCoreCall("IsFacingMe"); }
// DIA: idAI2::StartReloading
idRecoveredValue idAI2::StartReloading(...) { return RecordCoreCall("StartReloading"); }
// DIA: `idAI2::StartReloading'::`1'::dtor$0
// DIA: `idAI2::StartReloading'::`1'::dtor$1
// DIA: idAI2::IsDowned
idRecoveredValue idAI2::IsDowned(...) { return RecordCoreCall("IsDowned"); }
// DIA: idAI2::CanBeTakenDown
idRecoveredValue idAI2::CanBeTakenDown(...) { return RecordCoreCall("CanBeTakenDown"); }
// DIA: idAI2::SetTakeDownWindow
idRecoveredValue idAI2::SetTakeDownWindow(...) { return RecordCoreCall("SetTakeDownWindow"); }
// DIA: idAI2::GetUsableState
idRecoveredValue idAI2::GetUsableState(...) { return RecordCoreCall("GetUsableState"); }
// DIA: idAI2::GetOnlineUsableState
idRecoveredValue idAI2::GetOnlineUsableState(...) { return RecordCoreCall("GetOnlineUsableState"); }
// DIA: idAI2::UpdateEnemyInAAS
idRecoveredValue idAI2::UpdateEnemyInAAS(...) { return RecordCoreCall("UpdateEnemyInAAS"); }
// DIA: idAI2::SetAnimWebEvent
idRecoveredValue idAI2::SetAnimWebEvent(...) { return RecordCoreCall("SetAnimWebEvent"); }
// DIA: idAI2::ClearAnimWebEvent
idRecoveredValue idAI2::ClearAnimWebEvent(...) { return RecordCoreCall("ClearAnimWebEvent"); }
// DIA: idAI2::ClearAllAnimWebEvents
idRecoveredValue idAI2::ClearAllAnimWebEvents(...) { return RecordCoreCall("ClearAllAnimWebEvents"); }
// DIA: idAI2::ClearWaitForOverrideAnim
void idAI2::ClearWaitForOverrideAnim(idAnimWebEvent::priority_t priority) {
  (void)priority;
  animWebEventPending = false;
}
// DIA: idAI2::AnimEventReceived
bool idAI2::AnimEventReceived(idAnimWebEvent::priority_t priority) const {
  (void)priority;
  return !animWebEventPending;
}
// DIA: idAI2::WaitForAnimToEnd
idRecoveredValue idAI2::WaitForAnimToEnd(...) { return RecordCoreCall("WaitForAnimToEnd"); }
// DIA: idAI2::WaitForAnimToStart
idRecoveredValue idAI2::WaitForAnimToStart(...) { return RecordCoreCall("WaitForAnimToStart"); }
// DIA: idAI2::WaitForAnimToStartVia
idRecoveredValue idAI2::WaitForAnimToStartVia(...) { return RecordCoreCall("WaitForAnimToStartVia"); }
// DIA: idAI2::WaitForAnimToStartVia
idRecoveredValue idAI2::WaitForAnimToStartVia(idRecoveredOverloadTag<1>) { return RecordCoreCall("WaitForAnimToStartVia"); }
// DIA: idAI2::UpdateExtendedClipModel
idRecoveredValue idAI2::UpdateExtendedClipModel(...) { return RecordCoreCall("UpdateExtendedClipModel"); }
// DIA: idAI2::GetVoiceController
idRecoveredValue idAI2::GetVoiceController(...) { return RecordCoreCall("GetVoiceController"); }
// DIA: idAI2::GetDefaultSpeedByCurrentWalkState
float idAI2::GetDefaultSpeedByCurrentWalkState() const {
  return GetWalkState() == WALKSTATE_RUNNING ? maximumMovementSpeed
                                              : minimumMovementSpeed;
}
// DIA: idAI2::DormantBegin
idRecoveredValue idAI2::DormantBegin(...) { return RecordCoreCall("DormantBegin"); }
// DIA: idAI2::FinishFireWeapon
idRecoveredValue idAI2::FinishFireWeapon(...) { return RecordCoreCall("FinishFireWeapon"); }
// DIA: idAI2::GetSpotLostFireTime
float idAI2::GetSpotLostFireTime() const {
  const int blindedTimeRemaining = endBlindedTime - aiEvaluationTime;
  const float blindedSeconds =
      blindedTimeRemaining > 0
          ? static_cast<float>(blindedTimeRemaining) * 0.001F
          : 0.0F;
  return std::max(spotLostFireTime, blindedSeconds);
}
// DIA: idAI2::GetSpotGainedFireTime
float idAI2::GetSpotGainedFireTime() const {
  const int blindedTimeRemaining = endBlindedTime - aiEvaluationTime;
  const float blindedSeconds =
      blindedTimeRemaining > 0
          ? static_cast<float>(blindedTimeRemaining) * 0.001F
          : 0.0F;
  return std::max(spotGainedFireTime, blindedSeconds);
}
// DIA: idAI2::IsRecentlySpotted
bool idAI2::IsRecentlySpotted(const idAIEntityState* entityState,
                              float time) const {
  if (entityState == nullptr || !entityState->active) return false;
  const aiSenseState_t* sight = entityState->senses[1];
  return sight->lastUnconfirmedStimTime >= 0 &&
         (time < 0.0F ||
          gameLocal->GetGameTimeMs() - sight->lastConfirmedStimTime <
              static_cast<int>(time * 1000.0F));
}
// DIA: idAI2::IsRecentlyHeard
bool idAI2::IsRecentlyHeard(const idAIEntityState* entityState,
                            float time) const {
  if (entityState == nullptr || !entityState->active) return false;
  const aiSenseState_t* hearing = entityState->senses[0];
  return hearing->lastUnconfirmedStimTime >= 0 &&
         (time < 0.0F ||
          gameLocal->GetGameTimeMs() - hearing->lastConfirmedStimTime <
              static_cast<int>(time * 1000.0F));
}
// DIA: idAI2::GetViewTransform
idRecoveredValue idAI2::GetViewTransform(...) { return RecordCoreCall("GetViewTransform"); }
// DIA: idAI2::GetEyeOffset
idRecoveredValue idAI2::GetEyeOffset(...) { return RecordCoreCall("GetEyeOffset"); }
// DIA: idAI2::TeleportPush
idRecoveredValue idAI2::TeleportPush(...) { return RecordCoreCall("TeleportPush"); }
// DIA: idAI2::SpawnFlareEffect
idRecoveredValue idAI2::SpawnFlareEffect(...) { return RecordCoreCall("SpawnFlareEffect"); }
// DIA: idCheckSurroundingsState::TraceVector
// DIA: idCheckSurroundingsState::UpdateDirectionDistanceExponent
// DIA: idAI2::UpdateActions
idRecoveredValue idAI2::UpdateActions(...) { return RecordCoreCall("UpdateActions"); }
// DIA: idAI2::SetAlertCycle
idRecoveredValue idAI2::SetAlertCycle(...) { return RecordCoreCall("SetAlertCycle"); }
// DIA: idAI2::DoEmergencyCoverExit
idRecoveredValue idAI2::DoEmergencyCoverExit(...) { return RecordCoreCall("DoEmergencyCoverExit"); }
// DIA: `idAI2::DoEmergencyCoverExit'::`1'::dtor$0
// DIA: `idAI2::DoEmergencyCoverExit'::`1'::dtor$1
// DIA: idAI2::GetAnimationPathReachability
idRecoveredValue idAI2::GetAnimationPathReachability(...) { return RecordCoreCall("GetAnimationPathReachability"); }
// DIA: idAI2::GetAnimationDuration
idRecoveredValue idAI2::GetAnimationDuration(...) { return RecordCoreCall("GetAnimationDuration"); }
// DIA: idAI2::GetAnimationDelta
idRecoveredValue idAI2::GetAnimationDelta(idRecoveredOverloadTag<1>) { return RecordCoreCall("GetAnimationDelta"); }
// DIA: idAI2::GetAnimationRot
idRecoveredValue idAI2::GetAnimationRot(...) { return RecordCoreCall("GetAnimationRot"); }
// DIA: idAI2::PlayAnimWebPath
idRecoveredValue idAI2::PlayAnimWebPath(...) { return RecordCoreCall("PlayAnimWebPath"); }
// DIA: idAI2::GetCoverTransitionRadius
idRecoveredValue idAI2::GetCoverTransitionRadius(...) { return RecordCoreCall("GetCoverTransitionRadius"); }
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$0
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$1
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$2
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$3
// DIA: idAI2::ShouldScramble
idRecoveredValue idAI2::ShouldScramble(...) { return RecordCoreCall("ShouldScramble"); }
// DIA: idAI2::StartRagdoll
idRecoveredValue idAI2::StartRagdoll(idRecoveredOverloadTag<1>) { return RecordCoreCall("StartRagdoll"); }
// DIA: idAI2::UpdateGoreLevel
idRecoveredValue idAI2::UpdateGoreLevel(...) { return RecordCoreCall("UpdateGoreLevel"); }
// DIA: idAIScriptProxy::PauseCurrentThreadTillActionCompletes
// DIA: idAIScriptProxy::PauseCurrentFiberTillActionCompletes
// DIA: idEntityPtr<idTarget_FakeEnemy>::operator idTarget_FakeEnemy * __ptr64
// DIA: idLeakyIntegrator<0,1>::Update
// DIA: idEntityPtr<idVolume_CustomDeath>::operator->
// DIA: idHashTableT<int,idAIAction * __ptr64>::~idHashTableT<int,idAIAction * __ptr64>
// DIA: idAIEvent::GetOriginator
// DIA: idEntity::GetTarget
std::intptr_t idEntity::GetTarget(...) { return {}; }
// DIA: idAIDamageInfo::GetInflictor
// DIA: idAIDamageInfo::GetAttacker
// DIA: idAIMemory::GetEnemy
idRecoveredValue idAIMemory::GetEnemy(...) { return {}; }
// DIA: idAIMemory::GetFriendly
idRecoveredValue idAIMemory::GetFriendly(...) { return {}; }
// DIA: idAIMemory::GetFriendlyBlocker
idRecoveredValue idAIMemory::GetFriendlyBlocker(...) { return {}; }
// DIA: idAIMemory::GetLastAttacker
idRecoveredValue idAIMemory::GetLastAttacker(...) { return {}; }
// DIA: idFocusInfo::HasFocus
// DIA: idAIDeathInfo::idAIDeathInfo
// DIA: idAI2::HasFocus
idRecoveredValue idAI2::HasFocus(...) { return RecordCoreCall("HasFocus"); }
// DIA: idAI2::GetControllingPlayer
idPlayer* idAI2::GetControllingPlayer() const { return controllingPlayer; }
// DIA: idAI2::IsPlayerControlled
bool idAI2::IsPlayerControlled() const { return controllingPlayer != nullptr; }
// DIA: idGameLocal::GetDebugEntity
// DIA: idGameLocal::GetTensionManager
// DIA: idEntityPtr<idProxyMove>::operator idProxyMove * __ptr64
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::WriteComponents_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::ReadComponents_Impl
// DIA: idShieldHealthT<enum aiHealthComponent_t,2,0,1>::HealShield_Impl
// DIA: idShieldHealthT<enum aiHealthComponent_t,2,0,1>::DamageShield_Impl
// DIA: idShieldHealthT<enum aiHealthComponent_t,2,0,1>::Update_Impl
// DIA: idAIDeathInfo::Set
void idAIDeathInfo::Set(idAIDamageInfo* info) {
  attacker = info->GetAttackerEntity();
  inflictor = info->GetInflictorEntity();
  const idDamageImpulse* deathImpulse = info->GetDeathImpulse();
  impulse = deathImpulse != nullptr ? *deathImpulse : idDamageImpulse{};
  startDeathTime = gameLocal->GetGameTimeMs();
}
// DIA: idAIEditable::idInteractions::idInteractions
// DIA: idAIEditable::idInteractions::~idInteractions
// DIA: idAIEditable::idAIActionSettings::idAIFireSettings::idAIFireSettings
// DIA: idAI2::idAnimWebDefaults::idAnimWebDefaults
idAI2::idAnimWebDefaults::idAnimWebDefaults() = default;
// DIA: idAI2::idAIVolatile::idAIFocusInfo::idAIFocusInfo
idAI2::idAIVolatile::idAIFocusInfo::idAIFocusInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIFocusInfo::idAIFocusInfo'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::idAIDeath::idAIDeath
idAI2::idAIVolatile::idAIDeath::idAIDeath() = default;
// DIA: `idAI2::idAIVolatile::idAIDeath::idAIDeath'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::idAIDeath::~idAIDeath
idAI2::idAIVolatile::idAIDeath::~idAIDeath() = default;
// DIA: idAI2::idAnimWebDefaults::~idAnimWebDefaults
idAI2::idAnimWebDefaults::~idAnimWebDefaults() = default;
// DIA: idAI2::RobotBatteryExplodes
idRecoveredValue idAI2::RobotBatteryExplodes(...) { return RecordCoreCall("RobotBatteryExplodes"); }
// DIA: idAI2::CanFireRobot
bool idAI2::CanFireRobot(int currentTime) {
  if (!robotState.usesRobot) return true;
  if (robotState.isCharging || robotState.noFireTime >= currentTime ||
      (robotState.usesTesla &&
       (teslaDischargeTime >= 0 || teslaExplodeTime >= 0))) {
    robotState.lastShootTime = -1;
    return false;
  }
  if (robotState.shootHealthDrain <= 0.0F) return true;

  const int lastShootTime = robotState.lastShootTime;
  if (lastShootTime <= 0 ||
      (robotState.currentStageHealth -=
           static_cast<float>(currentTime - lastShootTime) * 0.001F *
           robotState.stageHealth * robotState.shootHealthDrain,
       robotState.chargeHealthFraction <
           robotState.currentStageHealth / robotState.stageHealth)) {
    robotState.lastShootTime = currentTime;
    robotState.bossHealth =
        (static_cast<float>(robotState.numberOfStages -
                            robotState.currentStage - 1) +
         robotState.currentStageHealth / robotState.stageHealth) /
        static_cast<float>(robotState.numberOfStages);
    return true;
  }

  SetRobotPowerOut();
  if (robotState.usesRobot) robotState.lastShootTime = -1;
  robotState.bossHealth =
      (static_cast<float>(robotState.numberOfStages - robotState.currentStage -
                          1) +
       robotState.currentStageHealth / robotState.stageHealth) /
      static_cast<float>(robotState.numberOfStages);
  return false;
}
// DIA: idAI2::StopFireRobot
void idAI2::StopFireRobot(int currentTime) {
  if (!robotState.usesRobot) return;
  const int lastShootTime = robotState.lastShootTime;
  if (lastShootTime > 0 && !robotState.isCharging &&
      robotState.shootHealthDrain > 0.0F) {
    const float stageHealth = robotState.stageHealth;
    robotState.currentStageHealth -=
        static_cast<float>(currentTime - lastShootTime) * 0.001F *
        stageHealth * robotState.shootHealthDrain;
    if (robotState.chargeHealthFraction >=
        robotState.currentStageHealth / stageHealth) {
      SetRobotPowerOut();
      robotState.currentStageHealth =
          stageHealth * robotState.chargeHealthFraction;
    }
    robotState.bossHealth =
        (static_cast<float>(robotState.numberOfStages -
                            robotState.currentStage - 1) +
         robotState.currentStageHealth / stageHealth) /
        static_cast<float>(robotState.numberOfStages);
  }
  if (robotState.usesRobot) robotState.lastShootTime = -1;
}
// DIA: idAI2::CanTeslaBlindFire
bool idAI2::CanTeslaBlindFire(idAIEntityState* enemyState, int duration) {
  return teslaBlindFireDuration > 0.0F && enemyState != nullptr &&
         teslaBlindFireTimeStamp != 0 &&
         gameLocal->GetGameTimeMs() <= duration + teslaBlindFireTimeStamp &&
         !IsRecentlySpotted(enemyState, spotLostFireTime) &&
         aiEventState.teslaBlindFirePositions.Num() >= 1;
}
// DIA: idAI2::SpawnTeslaBomb
idRecoveredValue idAI2::SpawnTeslaBomb(...) { return RecordCoreCall("SpawnTeslaBomb"); }
// DIA: idAI2::StopTesla
idRecoveredValue idAI2::StopTesla(...) { return RecordCoreCall("StopTesla"); }
// DIA: idAI2::LKWBlind
idRecoveredValue idAI2::LKWBlind(...) { return RecordCoreCall("LKWBlind"); }
// DIA: idAI2::UpdateGrappleActivation
idRecoveredValue idAI2::UpdateGrappleActivation(...) { return RecordCoreCall("UpdateGrappleActivation"); }
// DIA: idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb
// DIA: `idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb'::`1'::dtor$0
// DIA: `idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb'::`1'::dtor$1
// DIA: `idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb'::`1'::dtor$2
// DIA: idAI2::Remove
idRecoveredValue idAI2::Remove(...) { return RecordCoreCall("Remove"); }
// DIA: idAI2::ShouldEnterDormancy
idRecoveredValue idAI2::ShouldEnterDormancy(...) { return RecordCoreCall("ShouldEnterDormancy"); }
// DIA: idAI2::UpdateFSMs
idRecoveredValue idAI2::UpdateFSMs(...) { return RecordCoreCall("UpdateFSMs"); }
// DIA: idAI2::UpdateMovement
idRecoveredValue idAI2::UpdateMovement(...) { return RecordCoreCall("UpdateMovement"); }
// DIA: idAI2::UpdateDeathMovement
idRecoveredValue idAI2::UpdateDeathMovement(...) { return RecordCoreCall("UpdateDeathMovement"); }
// DIA: idAI2::UpdatePathToEnemy
idRecoveredValue idAI2::UpdatePathToEnemy(...) { return RecordCoreCall("UpdatePathToEnemy"); }
// DIA: idAI2::SendDeathTrigger
idRecoveredValue idAI2::SendDeathTrigger(...) { return RecordCoreCall("SendDeathTrigger"); }
// DIA: idAI2::ApplyDeathAnimationLogic
idRecoveredValue idAI2::ApplyDeathAnimationLogic(...) { return RecordCoreCall("ApplyDeathAnimationLogic"); }
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$0
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$1
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$2
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$3
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$4
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$5
// DIA: idAI2::DamageFeedback
idRecoveredValue idAI2::DamageFeedback(...) { return RecordCoreCall("DamageFeedback"); }
// DIA: idAI2::GetMeleeTypeForTarget
idRecoveredValue idAI2::GetMeleeTypeForTarget(...) { return RecordCoreCall("GetMeleeTypeForTarget"); }
// DIA: idAI2::HandleAffirmativeResponse
idRecoveredValue idAI2::HandleAffirmativeResponse(...) { return RecordCoreCall("HandleAffirmativeResponse"); }
// DIA: idAI2::GetSuspectedEnemyPos_PushedIntoAAS
idRecoveredValue idAI2::GetSuspectedEnemyPos_PushedIntoAAS(...) { return RecordCoreCall("GetSuspectedEnemyPos_PushedIntoAAS"); }
// DIA: idAI2::AwareOfEnemy
bool idAI2::AwareOfEnemy() const { return currentEnemy != nullptr; }
// DIA: idAI2::IsBlockedByFriendly
idRecoveredValue idAI2::IsBlockedByFriendly(...) { return RecordCoreCall("IsBlockedByFriendly"); }
// DIA: idAI2::IsAASBlocked
idRecoveredValue idAI2::IsAASBlocked(...) { return RecordCoreCall("IsAASBlocked"); }
// DIA: idAI2::GetFakeEnemyWeight
idRecoveredValue idAI2::GetFakeEnemyWeight(...) { return RecordCoreCall("GetFakeEnemyWeight"); }
// DIA: idAI2::OnAIEvent_Investigate
idRecoveredValue idAI2::OnAIEvent_Investigate(...) { return RecordCoreCall("OnAIEvent_Investigate"); }
// DIA: idAI2::OnAIEvent_Projectile
idRecoveredValue idAI2::OnAIEvent_Projectile(...) { return RecordCoreCall("OnAIEvent_Projectile"); }
// DIA: idAI2::OnAIEvent_PlayerAim
idRecoveredValue idAI2::OnAIEvent_PlayerAim(...) { return RecordCoreCall("OnAIEvent_PlayerAim"); }
// DIA: idAI2::ForceStopOverrideAnim
idRecoveredValue idAI2::ForceStopOverrideAnim(...) { return RecordCoreCall("ForceStopOverrideAnim"); }
// DIA: idAI2::ClearSyncedAnimData
void idAI2::ClearSyncedAnimData() {
  syncedOverrideAnimation = 0;
  syncedTargetSpawnId = 0x1FFF;
}
// DIA: idAI2::IsOverrideAnimSupported
idRecoveredValue idAI2::IsOverrideAnimSupported(...) { return RecordCoreCall("IsOverrideAnimSupported"); }
// DIA: idAI2::WaitForOverrideAnim
idRecoveredValue idAI2::WaitForOverrideAnim(...) { return RecordCoreCall("WaitForOverrideAnim"); }
// DIA: idAI2::SetLookFocusEntity
idRecoveredValue idAI2::SetLookFocusEntity(...) { return RecordCoreCall("SetLookFocusEntity"); }
// DIA: idAI2::TestFireWeapon
idRecoveredValue idAI2::TestFireWeapon(...) { return RecordCoreCall("TestFireWeapon"); }
// DIA: idAI2::TestForBlockedTrace
idRecoveredValue idAI2::TestForBlockedTrace(...) { return RecordCoreCall("TestForBlockedTrace"); }
// DIA: idAI2::GetFireAtLastKnownDuration
void idAI2::GetFireAtLastKnownDuration(const idWeapon* weapon, int* minimum,
                                       int* maximum) const {
  (void)weapon;
  const int duration = static_cast<int>(GetSpotLostFireTime() * 1000.0F);
  *minimum = duration;
  *maximum = duration;
}
// DIA: idAI2::GetEnemySenseState
idRecoveredValue idAI2::GetEnemySenseState(...) { return RecordCoreCall("GetEnemySenseState"); }
// DIA: idAI2::IsRecentlySpotted
idRecoveredValue idAI2::IsRecentlySpotted(idRecoveredOverloadTag<1>) { return RecordCoreCall("IsRecentlySpotted"); }
// DIA: idAI2::UpdateAimFocus
idRecoveredValue idAI2::UpdateAimFocus(...) { return RecordCoreCall("UpdateAimFocus"); }
// DIA: idAI2::SetupRobotDead
idRecoveredValue idAI2::SetupRobotDead(...) { return RecordCoreCall("SetupRobotDead"); }
// DIA: idAI2::FindClosestFocus
idRecoveredValue idAI2::FindClosestFocus(...) { return RecordCoreCall("FindClosestFocus"); }
// DIA: `idAI2::FindClosestFocus'::`1'::dtor$0
// DIA: idAI2::GetAlertCycle
idRecoveredValue idAI2::GetAlertCycle(...) { return RecordCoreCall("GetAlertCycle"); }
// DIA: idAI2::GetAlertCycleDuration
idRecoveredValue idAI2::GetAlertCycleDuration(...) { return RecordCoreCall("GetAlertCycleDuration"); }
// DIA: idAI2::LeaveActionNode
idRecoveredValue idAI2::LeaveActionNode(...) { return RecordCoreCall("LeaveActionNode"); }
// DIA: idAI2::UsesSmoothAnimations
idRecoveredValue idAI2::UsesSmoothAnimations(...) { return RecordCoreCall("UsesSmoothAnimations"); }
// DIA: idAI2::ActivateStealthKillTrigger
idRecoveredValue idAI2::ActivateStealthKillTrigger(...) { return RecordCoreCall("ActivateStealthKillTrigger"); }
// DIA: idAI2::GetAnimationPathReachability
idRecoveredValue idAI2::GetAnimationPathReachability(idRecoveredOverloadTag<1>) { return RecordCoreCall("GetAnimationPathReachability"); }
// DIA: `idAI2::GetAnimationPathReachability'::`1'::dtor$0
// DIA: `idAI2::GetAnimationPathReachability'::`1'::dtor$1
// DIA: idAI2::GetAnimationDelta
idRecoveredValue idAI2::GetAnimationDelta(idRecoveredOverloadTag<2>) { return RecordCoreCall("GetAnimationDelta"); }
// DIA: idAI2::GetAnimationRot
idRecoveredValue idAI2::GetAnimationRot(idRecoveredOverloadTag<1>) { return RecordCoreCall("GetAnimationRot"); }
// DIA: idList<idPonytail,5>::~idList<idPonytail,5>
// DIA: idList<idCrosshairCustomIcon,5>::Resize
// DIA: idAnimatorParms_AnimWeb::idAnimatorParms_AnimWeb
// DIA: idFSMManager::idFSMStack::idFSMStack
// DIA: idActor::SetLastDamageInfo
idRecoveredValue idActor::SetLastDamageInfo(...) { return {}; }
// DIA: idPlayer::GetProxyMove
idRecoveredValue idPlayer::GetProxyMove(...) { return {}; }
// DIA: idProp_Moveable::SetOwner
idRecoveredValue idProp_Moveable::SetOwner(...) { return {}; }
// DIA: idAIMemory::ShouldAvoid
idRecoveredValue idAIMemory::ShouldAvoid(...) { return {}; }
// DIA: idAIMemory::SetAvoidEntity
idRecoveredValue idAIMemory::SetAvoidEntity(...) { return {}; }
// DIA: idAIMemory::SetSpottedEnemyTrigger
idRecoveredValue idAIMemory::SetSpottedEnemyTrigger(...) { return {}; }
// DIA: idAIMemory::GetSpottedEnemyTrigger
idRecoveredValue idAIMemory::GetSpottedEnemyTrigger(...) { return {}; }
// DIA: idAIMemory::SetDetectedEnemyTrigger
idRecoveredValue idAIMemory::SetDetectedEnemyTrigger(...) { return {}; }
// DIA: idAIMemory::GetDetectedEnemyTrigger
idRecoveredValue idAIMemory::GetDetectedEnemyTrigger(...) { return {}; }
// DIA: idAIMemory::SetTriggeredInteraction
idRecoveredValue idAIMemory::SetTriggeredInteraction(...) { return {}; }
// DIA: idSpawnFocusWithRequirement::idSpawnFocusWithRequirement
// DIA: idAIEditable::idAIActionSettings::Clear
// DIA: idAI2::GetFakeEnemy
idRecoveredValue idAI2::GetFakeEnemy(...) { return RecordCoreCall("GetFakeEnemy"); }
// DIA: idAIEditable::idAIActionSettings::~idAIActionSettings
// DIA: `idAIEditable::idAIActionSettings::~idAIActionSettings'::`1'::dtor$0
// DIA: idAIEditable::idAIDeath::idAIDeath
// DIA: `idAIEditable::idAIDeath::idAIDeath'::`1'::dtor$0
// DIA: `idAIEditable::idAIDeath::idAIDeath'::`1'::dtor$1
// DIA: `idAIEditable::idAIDeath::idAIDeath'::`1'::dtor$2
// DIA: idAIEditable::idAIDeath::~idAIDeath
// DIA: idAI2::UpdateGroupStatusConditions
idRecoveredValue idAI2::UpdateGroupStatusConditions(...) { return RecordCoreCall("UpdateGroupStatusConditions"); }
// DIA: idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo
idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$6
// DIA: idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo
idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$6
// DIA: idAI2::idAIVolatile::idAIScriptInfo::idAIScriptInfo
idAI2::idAIVolatile::idAIScriptInfo::idAIScriptInfo() = default;
// DIA: idAI2::idAIVolatile::idAIScriptInfo::~idAIScriptInfo
idAI2::idAIVolatile::idAIScriptInfo::~idAIScriptInfo() = default;
// DIA: idAIDamageInfo::~idAIDamageInfo
// DIA: idAI2::idAIConstant::idAIConstant
idAI2::idAIConstant::idAIConstant() = default;
// DIA: `idAI2::idAIConstant::idAIConstant'::`1'::dtor$0
// DIA: `idAI2::idAIConstant::idAIConstant'::`1'::dtor$3
// DIA: idAI2::idAIConstant::~idAIConstant
idAI2::idAIConstant::~idAIConstant() = default;
// DIA: `idAI2::idAIConstant::~idAIConstant'::`1'::dtor$0
// DIA: `idAI2::idAIConstant::~idAIConstant'::`1'::dtor$2
// DIA: idAI2::DamageRobot
idRecoveredValue idAI2::DamageRobot(...) { return RecordCoreCall("DamageRobot"); }
// DIA: idAI2::ThinkRobot
idRecoveredValue idAI2::ThinkRobot(...) { return RecordCoreCall("ThinkRobot"); }
// DIA: idAI2::TurretShouldFire
idRecoveredValue idAI2::TurretShouldFire(...) { return RecordCoreCall("TurretShouldFire"); }
// DIA: idAI2::InitAnimation
idRecoveredValue idAI2::InitAnimation(...) { return RecordCoreCall("InitAnimation"); }
// DIA: `idAI2::InitAnimation'::`1'::dtor$0
// DIA: `idAI2::InitAnimation'::`1'::dtor$1
// DIA: `idAI2::InitAnimation'::`1'::dtor$11
// DIA: `idAI2::InitAnimation'::`1'::dtor$12
// DIA: `idAI2::InitAnimation'::`1'::dtor$13
// DIA: `idAI2::InitAnimation'::`1'::dtor$2
// DIA: `idAI2::InitAnimation'::`1'::dtor$16
// DIA: `idAI2::InitAnimation'::`1'::dtor$17
// DIA: `idAI2::InitAnimation'::`1'::dtor$18
// DIA: `idAI2::InitAnimation'::`1'::dtor$3
// DIA: `idAI2::InitAnimation'::`1'::dtor$4
// DIA: `idAI2::InitAnimation'::`1'::dtor$5
// DIA: `idAI2::InitAnimation'::`1'::dtor$6
// DIA: `idAI2::InitAnimation'::`1'::dtor$7
// DIA: `idAI2::InitAnimation'::`1'::dtor$8
// DIA: `idAI2::InitAnimation'::`1'::dtor$9
// DIA: `idAI2::InitAnimation'::`1'::dtor$10
// DIA: idAI2::UpdateTravelFlags
idRecoveredValue idAI2::UpdateTravelFlags(...) { return RecordCoreCall("UpdateTravelFlags"); }
// DIA: `idAI2::UpdateTravelFlags'::`1'::dtor$0
// DIA: idAI2::ShouldDeadThink
idRecoveredValue idAI2::ShouldDeadThink(...) { return RecordCoreCall("ShouldDeadThink"); }
// DIA: idAI2::UpdateEffects
idRecoveredValue idAI2::UpdateEffects(...) { return RecordCoreCall("UpdateEffects"); }
// DIA: idAI2::UpdateBodyOrientationWhileMoving
idRecoveredValue idAI2::UpdateBodyOrientationWhileMoving(...) { return RecordCoreCall("UpdateBodyOrientationWhileMoving"); }
// DIA: idAI2::StartStaggeringPain
idRecoveredValue idAI2::StartStaggeringPain(...) { return RecordCoreCall("StartStaggeringPain"); }
// DIA: idAI2::StartDeathSystem
idRecoveredValue idAI2::StartDeathSystem(...) { return RecordCoreCall("StartDeathSystem"); }
// DIA: `idAI2::StartDeathSystem'::`1'::dtor$2
// DIA: idAI2::KilledNotification
idRecoveredValue idAI2::KilledNotification(...) { return RecordCoreCall("KilledNotification"); }
// DIA: idAI2::Takedown
idRecoveredValue idAI2::Takedown(...) { return RecordCoreCall("Takedown"); }
// DIA: `idAI2::Takedown'::`1'::dtor$0
// DIA: `idAI2::Takedown'::`1'::dtor$1
// DIA: `idAI2::Takedown'::`1'::dtor$2
// DIA: `idAI2::Takedown'::`1'::dtor$3
// DIA: `idAI2::Takedown'::`1'::dtor$4
// DIA: `idAI2::Takedown'::`1'::dtor$5
// DIA: `idAI2::Takedown'::`1'::dtor$6
// DIA: `idAI2::Takedown'::`1'::dtor$7
// DIA: `idAI2::Takedown'::`1'::dtor$8
// DIA: `idAI2::Takedown'::`1'::dtor$9
// DIA: idAI2::Use
idRecoveredValue idAI2::Use(...) { return RecordCoreCall("Use"); }
// DIA: idAI2::InteractionCameraDeactivateCallback
idRecoveredValue idAI2::InteractionCameraDeactivateCallback(...) { return RecordCoreCall("InteractionCameraDeactivateCallback"); }
// DIA: idAI2::SetActionNodeGroupName
idRecoveredValue idAI2::SetActionNodeGroupName(...) { return RecordCoreCall("SetActionNodeGroupName"); }
// DIA: idAI2::UpdateAvoidCharacter
idRecoveredValue idAI2::UpdateAvoidCharacter(...) { return RecordCoreCall("UpdateAvoidCharacter"); }
// DIA: idAI2::ShouldHoldFire
idRecoveredValue idAI2::ShouldHoldFire(...) { return RecordCoreCall("ShouldHoldFire"); }
// DIA: idAI2::SetFakeEnemy
idRecoveredValue idAI2::SetFakeEnemy(...) { return RecordCoreCall("SetFakeEnemy"); }
// DIA: idAI2::ClearFakeEnemy
idRecoveredValue idAI2::ClearFakeEnemy(...) { return RecordCoreCall("ClearFakeEnemy"); }
// DIA: idAI2::ClearWorldState
idRecoveredValue idAI2::ClearWorldState(...) { return RecordCoreCall("ClearWorldState"); }
// DIA: idAI2::SetupFakePosition
idRecoveredValue idAI2::SetupFakePosition(...) { return RecordCoreCall("SetupFakePosition"); }
// DIA: idAI2::OnAIEvent_Voice
idRecoveredValue idAI2::OnAIEvent_Voice(...) { return RecordCoreCall("OnAIEvent_Voice"); }
// DIA: idAI2::OnAIEvent_Footstep
idRecoveredValue idAI2::OnAIEvent_Footstep(...) { return RecordCoreCall("OnAIEvent_Footstep"); }
// DIA: idAI2::OnAIEvent_Gunfire
idRecoveredValue idAI2::OnAIEvent_Gunfire(...) { return RecordCoreCall("OnAIEvent_Gunfire"); }
// DIA: `idAI2::OnAIEvent_Gunfire'::`1'::dtor$0
// DIA: `idAI2::OnAIEvent_Gunfire'::`1'::dtor$1
// DIA: idAI2::OnAIEvent_Explosion
idRecoveredValue idAI2::OnAIEvent_Explosion(...) { return RecordCoreCall("OnAIEvent_Explosion"); }
// DIA: idAI2::OnAIEvent_ProjectileImpact
idRecoveredValue idAI2::OnAIEvent_ProjectileImpact(...) { return RecordCoreCall("OnAIEvent_ProjectileImpact"); }
// DIA: idAI2::OnAIEvent_DeadBodyLand
idRecoveredValue idAI2::OnAIEvent_DeadBodyLand(...) { return RecordCoreCall("OnAIEvent_DeadBodyLand"); }
// DIA: idAI2::OnAIEvent_DoorMotion
idRecoveredValue idAI2::OnAIEvent_DoorMotion(...) { return RecordCoreCall("OnAIEvent_DoorMotion"); }
// DIA: idAI2::OnAIEvent_DeploySound
idRecoveredValue idAI2::OnAIEvent_DeploySound(...) { return RecordCoreCall("OnAIEvent_DeploySound"); }
// DIA: idAI2::OnAIEvent_Grenade
idRecoveredValue idAI2::OnAIEvent_Grenade(...) { return RecordCoreCall("OnAIEvent_Grenade"); }
// DIA: idAI2::OnAIEvent_Vehicle
idRecoveredValue idAI2::OnAIEvent_Vehicle(...) { return RecordCoreCall("OnAIEvent_Vehicle"); }
// DIA: idAI2::UpdateAnimationControllers
idRecoveredValue idAI2::UpdateAnimationControllers(...) { return RecordCoreCall("UpdateAnimationControllers"); }
// DIA: idAI2::Debug_Perception
idRecoveredValue idAI2::Debug_Perception(...) { return RecordCoreCall("Debug_Perception"); }
// DIA: idAI2::idVisibilityObscurity::Update
idRecoveredValue idAI2::idVisibilityObscurity::Update(...) { return {}; }
// DIA: idAI2::ShouldReloadWeapon
idRecoveredValue idAI2::ShouldReloadWeapon(...) { return RecordCoreCall("ShouldReloadWeapon"); }
// DIA: idAI2::CheckForSuspectedToTrailUpdate
idRecoveredValue idAI2::CheckForSuspectedToTrailUpdate(...) { return RecordCoreCall("CheckForSuspectedToTrailUpdate"); }
// DIA: idAI2::CalcMovementTurnRate
idRecoveredValue idAI2::CalcMovementTurnRate(...) { return RecordCoreCall("CalcMovementTurnRate"); }
// DIA: idCheckSurroundingsState::CanUse
// DIA: idCheckSurroundingsState::FindBestLookInterestPoint
// DIA: idCheckSurroundingsState::Update
// DIA: idAI2::FindAutoFocusEntity
idRecoveredValue idAI2::FindAutoFocusEntity(...) { return RecordCoreCall("FindAutoFocusEntity"); }
// DIA: `idAI2::FindAutoFocusEntity'::`1'::dtor$0
// DIA: `idAI2::FindAutoFocusEntity'::`1'::dtor$1
// DIA: idAI2::FindAutoFocus
idRecoveredValue idAI2::FindAutoFocus(...) { return RecordCoreCall("FindAutoFocus"); }
// DIA: idAI2::StartActionScript
idRecoveredValue idAI2::StartActionScript(...) { return RecordCoreCall("StartActionScript"); }
// DIA: idAI2::GiveAwarenessOfEntity
idRecoveredValue idAI2::GiveAwarenessOfEntity(...) { return RecordCoreCall("GiveAwarenessOfEntity"); }
// DIA: idAI2::DoSpawnSettings
idRecoveredValue idAI2::DoSpawnSettings(...) { return RecordCoreCall("DoSpawnSettings"); }
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$5
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$0
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$1
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$2
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$3
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$4
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$9
// DIA: idAI2::SpawnCorpse
idRecoveredValue idAI2::SpawnCorpse(...) { return RecordCoreCall("SpawnCorpse"); }
// DIA: `idAI2::SpawnCorpse'::`1'::dtor$0
// DIA: `idAI2::SpawnCorpse'::`1'::dtor$1
// DIA: idAI2::GetAnimationDeltaAndRot
idRecoveredValue idAI2::GetAnimationDeltaAndRot(...) { return RecordCoreCall("GetAnimationDeltaAndRot"); }
// DIA: idAIScriptProxy::ForceFSMIntoExecutingScript
// DIA: idList<idHandle<int,enum invalidAIEvent_t,-1>,5>::Append
// DIA: idList<idHandle<int,enum invalidAIEvent_t,-1>,5>::Insert
// DIA: idList<eboltControlNode_t,5>::Append
// DIA: idList<idCrosshairCustomIcon,5>::SetNum
// DIA: idAIMoveState::~idAIMoveState
// DIA: idAIMoveState::GetMoveStatus
aiMoveStatus_t idAIMoveState::GetMoveStatus() const { return moveStatus; }
// DIA: idAIMoveState::GetFinalPoint
const idVec3& idAIMoveState::GetFinalPoint() const { return finalPoint; }
// DIA: idAIMoveState::GetNextPoint
// DIA: idAIMoveState::IsPlayingTraversalAnim
bool idAIMoveState::IsPlayingTraversalAnim() const {
  return traversalPlaying;
}
// DIA: idAIMoveState::SetTranslationDeltaScale
void idAIMoveState::SetTranslationDeltaScale(const idVec3& scale) {
  translationScale = scale;
}
// DIA: idAIMoveState::`scalar deleting destructor'
// DIA: idAI2::idAIVolatile::idAIPhysics::~idAIPhysics
idAI2::idAIVolatile::idAIPhysics::~idAIPhysics() = default;
// DIA: `idAI2::idAIVolatile::idAIPhysics::~idAIPhysics'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::idAIPain::~idAIPain
idAI2::idAIVolatile::idAIPain::~idAIPain() = default;
// DIA: idAI2::UpdateTesla
idRecoveredValue idAI2::UpdateTesla(...) { return RecordCoreCall("UpdateTesla"); }
// DIA: `idAI2::UpdateTesla'::`1'::dtor$0
// DIA: idAI2::DamageGroupPopOffArmor
idRecoveredValue idAI2::DamageGroupPopOffArmor(...) { return RecordCoreCall("DamageGroupPopOffArmor"); }
// DIA: `idAI2::DamageGroupPopOffArmor'::`1'::dtor$0
// DIA: idAI2::CheckForGore
idRecoveredValue idAI2::CheckForGore(...) { return RecordCoreCall("CheckForGore"); }
// DIA: idAI2::UpdateDeaths
idRecoveredValue idAI2::UpdateDeaths(...) { return RecordCoreCall("UpdateDeaths"); }
// DIA: idAI2::AlarmNotification
idRecoveredValue idAI2::AlarmNotification(...) { return RecordCoreCall("AlarmNotification"); }
// DIA: idAI2::RelinquishControl
idRecoveredValue idAI2::RelinquishControl(...) { return RecordCoreCall("RelinquishControl"); }
// DIA: `idAI2::RelinquishControl'::`1'::dtor$0
// DIA: idAI2::ModifyCrosshairInfo
idRecoveredValue idAI2::ModifyCrosshairInfo(...) { return RecordCoreCall("ModifyCrosshairInfo"); }
// DIA: idAI2::UpdateWorldState
idRecoveredValue idAI2::UpdateWorldState(...) { return RecordCoreCall("UpdateWorldState"); }
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$0
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$1
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$2
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$3
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$4
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$5
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$6
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$7
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$8
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$9
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$10
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$11
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$12
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$15
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$16
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$20
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$21
// DIA: idAI2::OnAIEvent
idRecoveredValue idAI2::OnAIEvent(...) { return RecordCoreCall("OnAIEvent"); }
// DIA: idAI2::HandlePendingAIEvents
idRecoveredValue idAI2::HandlePendingAIEvents(...) { return RecordCoreCall("HandlePendingAIEvents"); }
// DIA: idAI2::WaitForTraversal
idRecoveredValue idAI2::WaitForTraversal(...) { return RecordCoreCall("WaitForTraversal"); }
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$0
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$1
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$2
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$3
// DIA: idAI2::UpdateLookFocus
idRecoveredValue idAI2::UpdateLookFocus(...) { return RecordCoreCall("UpdateLookFocus"); }
// DIA: `idAI2::UpdateLookFocus'::`1'::dtor$0
// DIA: `idAI2::UpdateLookFocus'::`1'::dtor$1
// DIA: `idAI2::UpdateLookFocus'::`1'::dtor$2
// DIA: idAI2::Think_PlayerControl
idRecoveredValue idAI2::Think_PlayerControl(...) { return RecordCoreCall("Think_PlayerControl"); }
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$0
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$1
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$2
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$5
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$6
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$7
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$10
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$11
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$12
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$13
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$16
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$17
// DIA: idAIScriptProxy::InternalCallEvent
// DIA: idLegsFSM::~idLegsFSM
// DIA: idAI2::Spawn
idRecoveredValue idAI2::Spawn(...) { return RecordCoreCall("Spawn"); }
// DIA: `idAI2::Spawn'::`1'::dtor$0
// DIA: `idAI2::Spawn'::`1'::dtor$1
// DIA: `idAI2::Spawn'::`1'::dtor$2
// DIA: `idAI2::Spawn'::`1'::dtor$3
// DIA: idAI2::OnShow
idRecoveredValue idAI2::OnShow(...) { return RecordCoreCall("OnShow"); }
// DIA: idAI2::Think_AIControl
idRecoveredValue idAI2::Think_AIControl(...) { return RecordCoreCall("Think_AIControl"); }
// DIA: idAI2::Damage
idRecoveredValue idAI2::Damage(...) { return RecordCoreCall("Damage"); }
// DIA: `idAI2::Damage'::`1'::dtor$0
// DIA: `idAI2::Damage'::`1'::dtor$1
// DIA: `idAI2::Damage'::`1'::dtor$6
// DIA: `idAI2::Damage'::`1'::dtor$7
// DIA: `idAI2::Damage'::`1'::dtor$8
// DIA: idAI2::DormantEnd
idRecoveredValue idAI2::DormantEnd(...) { return RecordCoreCall("DormantEnd"); }
// DIA: idAI2::FirstThink
idRecoveredValue idAI2::FirstThink(...) { return RecordCoreCall("FirstThink"); }
// DIA: idAIActionManager::~idAIActionManager
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$0
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$1
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$2
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$3
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$4
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$5
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$6
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$7
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$8
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$9
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$10
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$11
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$12
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$13
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$14
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$15
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$16
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$17
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$18
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$19
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$20
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$21
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$22
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$23
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$24
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$25
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$26
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$27
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$28
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$29
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$30
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$31
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$32
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$33
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$34
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$35
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$36
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$37
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$38
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$39
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$40
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$41
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$42
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$43
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$44
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$45
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$46
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$47
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$48
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$49
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$50
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$51
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$52
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$53
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$54
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$55
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$56
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$57
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$58
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$59
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$60
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$61
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$62
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$63
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$64
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$65
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$66
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$67
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$68
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$69
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$70
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$71
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$72
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$73
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$74
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$75
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$76
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$77
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$78
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$79
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$80
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$81
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$82
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$83
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$84
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$85
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$86
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$87
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$88
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$89
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$90
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$91
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$94
// DIA: idAI2::Think_Impl
idRecoveredValue idAI2::Think_Impl(...) { return RecordCoreCall("Think_Impl"); }
// DIA: `idAI2::Think_Impl'::`1'::dtor$0
// DIA: `idAI2::Think_Impl'::`1'::dtor$1
// DIA: `idAI2::Think_Impl'::`1'::dtor$2
// DIA: `idAI2::Think_Impl'::`1'::dtor$3
// DIA: `idAI2::Think_Impl'::`1'::dtor$5
// DIA: `idAI2::Think_Impl'::`1'::dtor$6
// DIA: `idAI2::Think_Impl'::`1'::dtor$8
// DIA: idAI2::idAIVolatile::idAIFSMInfo::~idAIFSMInfo
idAI2::idAIVolatile::idAIFSMInfo::~idAIFSMInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIFSMInfo::~idAIFSMInfo'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::~idAIVolatile
idAI2::idAIVolatile::~idAIVolatile() = default;
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$6
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$7
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$8
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$9
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$10
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$11
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$12
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$13
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$14
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$15
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$22
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$25
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$30
// DIA: idAI2::Think
idRecoveredValue idAI2::Think(...) { return RecordCoreCall("Think"); }
// DIA: `idAI2::Think'::`1'::dtor$0
// DIA: idAIEditable::idAISpawn::idAISpawn
// DIA: `idAIEditable::idAISpawn::idAISpawn'::`1'::dtor$0
// DIA: `idAIEditable::idAISpawn::idAISpawn'::`1'::dtor$1
// DIA: idAIEditable::idAISpawn::~idAISpawn
// DIA: idAI2::idAIVolatile::idAIVolatile
idAI2::idAIVolatile::idAIVolatile() = default;
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$17
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$21
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$6
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$7
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$8
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$9
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$10
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$11
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$12
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$13
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$14
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$15
// DIA: idAIEditable::~idAIEditable
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$0
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$1
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$2
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$3
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$4
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$5
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$6
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$7
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$8
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$9
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$11
// DIA: idAI2::~idAI2
// DIA: `idAI2::~idAI2'::`1'::dtor$0
// DIA: `idAI2::~idAI2'::`1'::dtor$1
// DIA: `idAI2::~idAI2'::`1'::dtor$2
// DIA: `idAI2::~idAI2'::`1'::dtor$3
// DIA: `idAI2::~idAI2'::`1'::dtor$4
// DIA: `idAI2::~idAI2'::`1'::dtor$5
// DIA: `idAI2::~idAI2'::`1'::dtor$6
// DIA: `idAI2::~idAI2'::`1'::dtor$7
// DIA: `idAI2::~idAI2'::`1'::dtor$8
// DIA: `idAI2::~idAI2'::`1'::dtor$9
// DIA: `idAI2::~idAI2'::`1'::dtor$12
// DIA: `idAI2::~idAI2'::`1'::dtor$14
// DIA: `idAI2::~idAI2'::`1'::dtor$16
// DIA: idActor::GetAF
idRecoveredValue idActor::GetAF(...) { return {}; }
// DIA: idActor::GetLastEyePos
idRecoveredValue idActor::GetLastEyePos(...) { return {}; }
// DIA: idActor::GetEyeAxis
idRecoveredValue idActor::GetEyeAxis(...) { return {}; }
// DIA: idActor::SetEyeAxis
idRecoveredValue idActor::SetEyeAxis(...) { return {}; }
// DIA: idActor::GetInventory
idRecoveredValue idActor::GetInventory(...) { return {}; }
// DIA: idActor::GetAttachments
idRecoveredValue idActor::GetAttachments(...) { return {}; }
// DIA: idActor::GetTriggerTouchList
idRecoveredValue idActor::GetTriggerTouchList(...) { return {}; }
// DIA: idActor::GetDisplayName
idRecoveredValue idActor::GetDisplayName(...) { return {}; }
// DIA: idActor::GetPosture
idRecoveredValue idActor::GetPosture(...) { return {}; }
// DIA: idActor::GetLeanState
idRecoveredValue idActor::GetLeanState(...) { return {}; }
// DIA: idActor::SetLeanState
idRecoveredValue idActor::SetLeanState(...) { return {}; }
// DIA: idActor::GetWeaponReadyState
idRecoveredValue idActor::GetWeaponReadyState(...) { return {}; }
// DIA: idActor::SetWeaponReadyState
idRecoveredValue idActor::SetWeaponReadyState(...) { return {}; }
// DIA: idAI2::GetAASPositionMgr
idRecoveredValue idAI2::GetAASPositionMgr(...) { return RecordCoreCall("GetAASPositionMgr"); }
// DIA: idAI2::GetHealthComponent
idRecoveredValue idAI2::GetHealthComponent(...) { return RecordCoreCall("GetHealthComponent"); }
// DIA: idAI2::GetComponentTimeLine
idRecoveredValue idAI2::GetComponentTimeLine(...) { return RecordCoreCall("GetComponentTimeLine"); }
// DIA: idAI2::GetAAS
idRecoveredValue idAI2::GetAAS(...) { return RecordCoreCall("GetAAS"); }
// DIA: idAI2::GetWeaponSlot
idRecoveredValue idAI2::GetWeaponSlot(...) { return RecordCoreCall("GetWeaponSlot"); }
// DIA: idAI2::HasSyncedWalks
idRecoveredValue idAI2::HasSyncedWalks(...) { return RecordCoreCall("HasSyncedWalks"); }
// DIA: idAI2::GetInputSettings
idRecoveredValue idAI2::GetInputSettings(...) { return RecordCoreCall("GetInputSettings"); }
// DIA: idAI2::GetDynamicCoverMgr
idRecoveredValue idAI2::GetDynamicCoverMgr(...) { return RecordCoreCall("GetDynamicCoverMgr"); }
// DIA: idAI2::GetAnimatorAnimWeb
idRecoveredValue idAI2::GetAnimatorAnimWeb(...) { return RecordCoreCall("GetAnimatorAnimWeb"); }
// DIA: idAI2::IsDead
idRecoveredValue idAI2::IsDead(...) { return RecordCoreCall("IsDead"); }
// DIA: idAI2::IsDying
idRecoveredValue idAI2::IsDying(...) { return RecordCoreCall("IsDying"); }
// DIA: idAIEditable::idAIEditable
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$0
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$12
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$1
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$2
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$3
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$4
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$5
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$6
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$7
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$8
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$9
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$10
// DIA: idAI2::idAI2
// DIA: `idAI2::idAI2'::`1'::dtor$0
// DIA: `idAI2::idAI2'::`1'::dtor$1
// DIA: `idAI2::idAI2'::`1'::dtor$2
// DIA: `idAI2::idAI2'::`1'::dtor$3
// DIA: `idAI2::idAI2'::`1'::dtor$4
// DIA: `idAI2::idAI2'::`1'::dtor$5
// DIA: `idAI2::idAI2'::`1'::dtor$6
// DIA: `idAI2::idAI2'::`1'::dtor$11
// DIA: `idAI2::idAI2'::`1'::dtor$7
// DIA: `idAI2::idAI2'::`1'::dtor$8
// DIA: `idAI2::idAI2'::`1'::dtor$9
// DIA: `idAI2::idAI2'::`1'::dtor$15
// DIA: `idAI2::idAI2'::`1'::dtor$16
// DIA: `idAI2::idAI2'::`1'::dtor$10
// DIA: `idAI2::idAI2'::`1'::dtor$18
// DIA: idAI2::`scalar deleting destructor'
// DIA: `dynamic initializer for 'ai_limitFSMUpdates''
idCVar ai_limitFSMUpdates("ai_limitFSMUpdates", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLevel''
idCVar ai_debugLevel("ai_debugLevel", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTracking''
idCVar ai_debugTracking("ai_debugTracking", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugHealth''
idCVar ai_debugHealth("ai_debugHealth", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEvents''
idCVar ai_debugEvents("ai_debugEvents", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugState''
idCVar ai_debugState("ai_debugState", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugAim''
idCVar ai_debugAim("ai_debugAim", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLook''
idCVar ai_debugLook("ai_debugLook", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugOrientation''
idCVar ai_debugOrientation("ai_debugOrientation", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugCover''
idCVar ai_debugCover("ai_debugCover", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugCoverEntityId''
idCVar ai_debugCoverEntityId("ai_debugCoverEntityId", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEnemy''
idCVar ai_debugEnemy("ai_debugEnemy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_visibilitySenseScale''
idCVar ai_visibilitySenseScale("ai_visibilitySenseScale", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableSenseVolumes''
idCVar ai_enableSenseVolumes("ai_enableSenseVolumes", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugSearch''
idCVar ai_debugSearch("ai_debugSearch", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugFriendly''
idCVar ai_debugFriendly("ai_debugFriendly", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPain''
idCVar ai_debugPain("ai_debugPain", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInjured''
idCVar ai_debugInjured("ai_debugInjured", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugDeath''
idCVar ai_debugDeath("ai_debugDeath", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPushFromCollision''
idCVar ai_debugPushFromCollision("ai_debugPushFromCollision", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugVoice''
idCVar ai_debugVoice("ai_debugVoice", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTension''
idCVar ai_debugTension("ai_debugTension", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugDangerZone''
idCVar ai_debugDangerZone("ai_debugDangerZone", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useCombatSubtitles''
idCVar ai_useCombatSubtitles("ai_useCombatSubtitles", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTargetList''
idCVar ai_debugTargetList("ai_debugTargetList", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEnemyTargetSelector''
idCVar ai_debugEnemyTargetSelector("ai_debugEnemyTargetSelector", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteraction''
idCVar ai_debugInteraction("ai_debugInteraction", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteractionDistance''
idCVar ai_debugInteractionDistance("ai_debugInteractionDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteraction_Approach''
idCVar ai_debugInteraction_Approach("ai_debugInteraction_Approach", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteraction_Departure''
idCVar ai_debugInteraction_Departure("ai_debugInteraction_Departure", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_fastJobInteraction''
idCVar ai_fastJobInteraction("ai_fastJobInteraction", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugmelee''
idCVar ai_debugmelee("ai_debugmelee", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugOpenCombatMove''
idCVar ai_debugOpenCombatMove("ai_debugOpenCombatMove", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLeapAttack''
idCVar ai_debugLeapAttack("ai_debugLeapAttack", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTraversals''
idCVar ai_debugTraversals("ai_debugTraversals", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEyePos''
idCVar ai_debugEyePos("ai_debugEyePos", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEncounterGroupRoles''
idCVar ai_debugEncounterGroupRoles("ai_debugEncounterGroupRoles", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugCoverTransitions''
idCVar ai_debugCoverTransitions("ai_debugCoverTransitions", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_showAASName''
idCVar ai_showAASName("ai_showAASName", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPathToEnemy''
idCVar ai_debugPathToEnemy("ai_debugPathToEnemy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugAnimDeltaScale''
idCVar ai_debugAnimDeltaScale("ai_debugAnimDeltaScale", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugMissTime''
idCVar ai_debugMissTime("ai_debugMissTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLatchedState''
idCVar ai_debugLatchedState("ai_debugLatchedState", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugBlendTree''
idCVar ai_debugBlendTree("ai_debugBlendTree", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_blendTreeName''
idCVar ai_blendTreeName("ai_blendTreeName", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugDodge''
idCVar ai_debugDodge("ai_debugDodge", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useAimTracker''
idCVar ai_useAimTracker("ai_useAimTracker", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useLookTracker''
idCVar ai_useLookTracker("ai_useLookTracker", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useWanderingAim''
idCVar ai_useWanderingAim("ai_useWanderingAim", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useWeaponSpread''
idCVar ai_useWeaponSpread("ai_useWeaponSpread", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useTransitions''
idCVar ai_useTransitions("ai_useTransitions", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useCoverTransitions''
idCVar ai_useCoverTransitions("ai_useCoverTransitions", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enable''
idCVar ai_enable("ai_enable", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableWorldState''
idCVar ai_enableWorldState("ai_enableWorldState", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableFSM''
idCVar ai_enableFSM("ai_enableFSM", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableAnimation''
idCVar ai_enableAnimation("ai_enableAnimation", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enablePhysics''
idCVar ai_enablePhysics("ai_enablePhysics", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableMovement''
idCVar ai_enableMovement("ai_enableMovement", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableDormancy''
idCVar ai_enableDormancy("ai_enableDormancy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableAutoFocus''
idCVar ai_enableAutoFocus("ai_enableAutoFocus", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableInjuredRuns''
idCVar ai_enableInjuredRuns("ai_enableInjuredRuns", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugGore''
idCVar ai_debugGore("ai_debugGore", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableScrambles''
idCVar ai_enableScrambles("ai_enableScrambles", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_scrambleTime''
idCVar ai_scrambleTime("ai_scrambleTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_searchToStimDist''
idCVar ai_searchToStimDist("ai_searchToStimDist", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_mood''
idCVar ai_mood("ai_mood", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_attitude''
idCVar ai_attitude("ai_attitude", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_otherFactionAttitude''
idCVar ai_otherFactionAttitude("ai_otherFactionAttitude", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_accuracy''
idCVar ai_accuracy("ai_accuracy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_lookRate''
idCVar ai_lookRate("ai_lookRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_keepShootingNoSight''
idCVar ai_keepShootingNoSight("ai_keepShootingNoSight", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_showFOV''
idCVar ai_showFOV("ai_showFOV", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_weaponKick''
idCVar ai_weaponKick("ai_weaponKick", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_faceFocus''
idCVar ai_faceFocus("ai_faceFocus", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_lookAtPlayer''
idCVar ai_lookAtPlayer("ai_lookAtPlayer", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_allowStaggerPain''
idCVar ai_allowStaggerPain("ai_allowStaggerPain", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_invulnerable''
idCVar ai_invulnerable("ai_invulnerable", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_ignoreEvents''
idCVar ai_ignoreEvents("ai_ignoreEvents", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_ignoreFireLimits''
idCVar ai_ignoreFireLimits("ai_ignoreFireLimits", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_forceWalk''
idCVar ai_forceWalk("ai_forceWalk", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_walkMod''
idCVar ai_walkMod("ai_walkMod", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_runMod''
idCVar ai_runMod("ai_runMod", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_forceTurnRate''
idCVar ai_forceTurnRate("ai_forceTurnRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_minTurnRate''
idCVar ai_minTurnRate("ai_minTurnRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_maxTurnRate''
idCVar ai_maxTurnRate("ai_maxTurnRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_moveModeLookAhead''
idCVar ai_moveModeLookAhead("ai_moveModeLookAhead", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_flashIfLootable''
idCVar ai_flashIfLootable("ai_flashIfLootable", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_lootDelay''
idCVar ai_lootDelay("ai_lootDelay", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_interactionView_useCamera''
idCVar ai_interactionView_useCamera("ai_interactionView_useCamera", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useStageCoverFilter''
idCVar ai_useStageCoverFilter("ai_useStageCoverFilter", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_disableWanderGreetings''
idCVar ai_disableWanderGreetings("ai_disableWanderGreetings", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_droppedLootDistance''
idCVar ai_droppedLootDistance("ai_droppedLootDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableInfluenceTrails''
idCVar ai_enableInfluenceTrails("ai_enableInfluenceTrails", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugGreetingDistance''
idCVar ai_debugGreetingDistance("ai_debugGreetingDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPerception''
idCVar ai_debugPerception("ai_debugPerception", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugScenePoints''
idCVar ai_debugScenePoints("ai_debugScenePoints", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_allowCollapse''
idCVar ai_allowCollapse("ai_allowCollapse", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_maxDeferDeathTime''
idCVar ai_maxDeferDeathTime("ai_maxDeferDeathTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_forceInjury''
idCVar ai_forceInjury("ai_forceInjury", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_pushVelocityTolerance''
idCVar ai_pushVelocityTolerance("ai_pushVelocityTolerance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_ragdollKillLimit''
idCVar ai_ragdollKillLimit("ai_ragdollKillLimit", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugBleedoutCheck''
idCVar ai_debugBleedoutCheck("ai_debugBleedoutCheck", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_thinkTrace''
idCVar ai_thinkTrace("ai_thinkTrace", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownWindoMaxTime''
idCVar takedownWindoMaxTime("takedownWindoMaxTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownType''
idCVar takedownType("takedownType", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownID''
idCVar takedownID("takedownID", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownWindowDebug''
idCVar takedownWindowDebug("takedownWindowDebug", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'counterBlockTime''
idCVar counterBlockTime("counterBlockTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugArmorPopOff''
idCVar ai_debugArmorPopOff("ai_debugArmorPopOff", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_damageScale''
idCVar ai_damageScale("ai_damageScale", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_syncAF''
idCVar ai_syncAF("ai_syncAF", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_limitTargetUpdates''
idCVar ai_limitTargetUpdates("ai_limitTargetUpdates", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_FadeDelay''
idCVar ai_FadeDelay("ai_FadeDelay", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_CoopFadeOutTime''
idCVar ai_CoopFadeOutTime("ai_CoopFadeOutTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_CoopFadeDelay''
idCVar ai_CoopFadeDelay("ai_CoopFadeDelay", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_NonPlayerOneShotChance''
idCVar ai_NonPlayerOneShotChance("ai_NonPlayerOneShotChance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useAvoidForCoverFilter''
idCVar ai_useAvoidForCoverFilter("ai_useAvoidForCoverFilter", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_controlHeartBeatIntervalsMin''
idCVar ai_controlHeartBeatIntervalsMin("ai_controlHeartBeatIntervalsMin", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_controlHeartBeatIntervalsMax''
idCVar ai_controlHeartBeatIntervalsMax("ai_controlHeartBeatIntervalsMax", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_controlTakeDownDistance''
idCVar ai_controlTakeDownDistance("ai_controlTakeDownDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'rpWetParameters''
idCVar rpWetParameters("rpWetParameters", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'rpLandWetRippleIntensity''
idCVar rpLandWetRippleIntensity("rpLandWetRippleIntensity", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic atexit destructor for 'ai_limitFSMUpdates''
// DIA: `dynamic atexit destructor for 'ai_debugLevel''
// DIA: `dynamic atexit destructor for 'ai_debugTracking''
// DIA: `dynamic atexit destructor for 'ai_debugHealth''
// DIA: `dynamic atexit destructor for 'ai_debugEvents''
// DIA: `dynamic atexit destructor for 'ai_debugState''
// DIA: `dynamic atexit destructor for 'ai_debugAim''
// DIA: `dynamic atexit destructor for 'ai_debugLook''
// DIA: `dynamic atexit destructor for 'ai_debugOrientation''
// DIA: `dynamic atexit destructor for 'ai_debugCover''
// DIA: `dynamic atexit destructor for 'ai_debugCoverEntityId''
// DIA: `dynamic atexit destructor for 'ai_debugEnemy''
// DIA: `dynamic atexit destructor for 'ai_visibilitySenseScale''
// DIA: `dynamic atexit destructor for 'ai_enableSenseVolumes''
// DIA: `dynamic atexit destructor for 'ai_debugSearch''
// DIA: `dynamic atexit destructor for 'ai_debugFriendly''
// DIA: `dynamic atexit destructor for 'ai_debugPain''
// DIA: `dynamic atexit destructor for 'ai_debugInjured''
// DIA: `dynamic atexit destructor for 'ai_debugDeath''
// DIA: `dynamic atexit destructor for 'ai_debugPushFromCollision''
// DIA: `dynamic atexit destructor for 'ai_debugVoice''
// DIA: `dynamic atexit destructor for 'ai_debugTension''
// DIA: `dynamic atexit destructor for 'ai_debugDangerZone''
// DIA: `dynamic atexit destructor for 'ai_useCombatSubtitles''
// DIA: `dynamic atexit destructor for 'ai_debugTargetList''
// DIA: `dynamic atexit destructor for 'ai_debugEnemyTargetSelector''
// DIA: `dynamic atexit destructor for 'ai_debugInteraction''
// DIA: `dynamic atexit destructor for 'ai_debugInteractionDistance''
// DIA: `dynamic atexit destructor for 'ai_debugInteraction_Approach''
// DIA: `dynamic atexit destructor for 'ai_debugInteraction_Departure''
// DIA: `dynamic atexit destructor for 'ai_fastJobInteraction''
// DIA: `dynamic atexit destructor for 'ai_debugmelee''
// DIA: `dynamic atexit destructor for 'ai_debugOpenCombatMove''
// DIA: `dynamic atexit destructor for 'ai_debugLeapAttack''
// DIA: `dynamic atexit destructor for 'ai_debugTraversals''
// DIA: `dynamic atexit destructor for 'ai_debugEyePos''
// DIA: `dynamic atexit destructor for 'ai_debugEncounterGroupRoles''
// DIA: `dynamic atexit destructor for 'ai_debugCoverTransitions''
// DIA: `dynamic atexit destructor for 'ai_showAASName''
// DIA: `dynamic atexit destructor for 'ai_debugPathToEnemy''
// DIA: `dynamic atexit destructor for 'ai_debugAnimDeltaScale''
// DIA: `dynamic atexit destructor for 'ai_debugMissTime''
// DIA: `dynamic atexit destructor for 'ai_debugLatchedState''
// DIA: `dynamic atexit destructor for 'ai_debugBlendTree''
// DIA: `dynamic atexit destructor for 'ai_blendTreeName''
// DIA: `dynamic atexit destructor for 'ai_debugDodge''
// DIA: `dynamic atexit destructor for 'ai_useAimTracker''
// DIA: `dynamic atexit destructor for 'ai_useLookTracker''
// DIA: `dynamic atexit destructor for 'ai_useWanderingAim''
// DIA: `dynamic atexit destructor for 'ai_useWeaponSpread''
// DIA: `dynamic atexit destructor for 'ai_useTransitions''
// DIA: `dynamic atexit destructor for 'ai_useCoverTransitions''
// DIA: `dynamic atexit destructor for 'ai_enable''
// DIA: `dynamic atexit destructor for 'ai_enableWorldState''
// DIA: `dynamic atexit destructor for 'ai_enableFSM''
// DIA: `dynamic atexit destructor for 'ai_enableAnimation''
// DIA: `dynamic atexit destructor for 'ai_enablePhysics''
// DIA: `dynamic atexit destructor for 'ai_enableMovement''
// DIA: `dynamic atexit destructor for 'ai_enableDormancy''
// DIA: `dynamic atexit destructor for 'ai_enableAutoFocus''
// DIA: `dynamic atexit destructor for 'ai_enableInjuredRuns''
// DIA: `dynamic atexit destructor for 'ai_debugGore''
// DIA: `dynamic atexit destructor for 'ai_enableScrambles''
// DIA: `dynamic atexit destructor for 'ai_scrambleTime''
// DIA: `dynamic atexit destructor for 'ai_searchToStimDist''
// DIA: `dynamic atexit destructor for 'ai_mood''
// DIA: `dynamic atexit destructor for 'ai_attitude''
// DIA: `dynamic atexit destructor for 'ai_otherFactionAttitude''
// DIA: `dynamic atexit destructor for 'ai_accuracy''
// DIA: `dynamic atexit destructor for 'ai_lookRate''
// DIA: `dynamic atexit destructor for 'ai_keepShootingNoSight''
// DIA: `dynamic atexit destructor for 'ai_showFOV''
// DIA: `dynamic atexit destructor for 'ai_weaponKick''
// DIA: `dynamic atexit destructor for 'ai_faceFocus''
// DIA: `dynamic atexit destructor for 'ai_lookAtPlayer''
// DIA: `dynamic atexit destructor for 'ai_allowStaggerPain''
// DIA: `dynamic atexit destructor for 'ai_invulnerable''
// DIA: `dynamic atexit destructor for 'ai_ignoreEvents''
// DIA: `dynamic atexit destructor for 'ai_ignoreFireLimits''
// DIA: `dynamic atexit destructor for 'ai_forceWalk''
// DIA: `dynamic atexit destructor for 'ai_runMod''
// DIA: `dynamic atexit destructor for 'ai_walkMod''
// DIA: `dynamic atexit destructor for 'ai_forceTurnRate''
// DIA: `dynamic atexit destructor for 'ai_minTurnRate''
// DIA: `dynamic atexit destructor for 'ai_maxTurnRate''
// DIA: `dynamic atexit destructor for 'ai_moveModeLookAhead''
// DIA: `dynamic atexit destructor for 'ai_flashIfLootable''
// DIA: `dynamic atexit destructor for 'ai_lootDelay''
// DIA: `dynamic atexit destructor for 'ai_interactionView_useCamera''
// DIA: `dynamic atexit destructor for 'ai_useStageCoverFilter''
// DIA: `dynamic atexit destructor for 'ai_disableWanderGreetings''
// DIA: `dynamic atexit destructor for 'ai_droppedLootDistance''
// DIA: `dynamic atexit destructor for 'ai_enableInfluenceTrails''
// DIA: `dynamic atexit destructor for 'ai_debugGreetingDistance''
// DIA: `dynamic atexit destructor for 'ai_debugPerception''
// DIA: `dynamic atexit destructor for 'ai_debugScenePoints''
// DIA: `dynamic atexit destructor for 'ai_allowCollapse''
// DIA: `dynamic atexit destructor for 'ai_maxDeferDeathTime''
// DIA: `dynamic atexit destructor for 'ai_forceInjury''
// DIA: `dynamic atexit destructor for 'ai_pushVelocityTolerance''
// DIA: `dynamic atexit destructor for 'ai_ragdollKillLimit''
// DIA: `dynamic atexit destructor for 'ai_debugBleedoutCheck''
// DIA: `dynamic atexit destructor for 'ai_thinkTrace''
// DIA: `dynamic atexit destructor for 'takedownWindoMaxTime''
// DIA: `dynamic atexit destructor for 'takedownType''
// DIA: `dynamic atexit destructor for 'takedownID''
// DIA: `dynamic atexit destructor for 'takedownWindowDebug''
// DIA: `dynamic atexit destructor for 'counterBlockTime''
// DIA: `dynamic atexit destructor for 'ai_debugArmorPopOff''
// DIA: `dynamic atexit destructor for 'ai_damageScale''
// DIA: `dynamic atexit destructor for 'ai_syncAF''
// DIA: `dynamic atexit destructor for 'ai_limitTargetUpdates''
// DIA: `dynamic atexit destructor for 'ai_FadeDelay''
// DIA: `dynamic atexit destructor for 'ai_CoopFadeOutTime''
// DIA: `dynamic atexit destructor for 'ai_CoopFadeDelay''
// DIA: `dynamic atexit destructor for 'ai_NonPlayerOneShotChance''
// DIA: `dynamic atexit destructor for 'ai_useAvoidForCoverFilter''
// DIA: `dynamic atexit destructor for 'ai_controlHeartBeatIntervalsMin''
// DIA: `dynamic atexit destructor for 'ai_controlHeartBeatIntervalsMax''
// DIA: `dynamic atexit destructor for 'ai_controlTakeDownDistance''

#include "tungsten/game/ai/ai2_core_support.inl"
