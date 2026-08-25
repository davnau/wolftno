#pragma once

#include "tungsten/game/ai/ai2_actions.h"
#include "tungsten/game/ai/aibehaviors.h"
#include "tungsten/game/ai/aidamageinfo.h"
#include "tungsten/game/ai/aimemory.h"
#include "tungsten/game/ai/aimovestate.h"
#include "tungsten/game/ai/rotationtracker.h"
#include "tungsten/game/clientgame/presentable/presentable.h"
#include "engine/decls/animwebs/blendparms.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/filesystem/file.h"

#include <algorithm>
#include <cstring>

enum invalidAliasHandle_t : int { INVALID_ALIAS_HANDLE = -1 };

template <typename T, typename InvalidEnum, T InvalidValue>
struct idHandle {
  T value{InvalidValue};
  bool operator==(const idHandle& other) const { return value == other.value; }
};

class idAnimator_Channel {
public:
  bool PlayAnim(unsigned short aliasHandle, int currentTime, float rateScale,
                int blendDurationMS, int blendOutDurationMS, float weight) {
    lastAliasHandle = aliasHandle;
    lastCurrentTime = currentTime;
    lastRateScale = rateScale;
    lastBlendDurationMS = blendDurationMS;
    lastBlendOutDurationMS = blendOutDurationMS;
    lastWeight = weight;
    ++playCount;
    return playResult;
  }
  void SetPlayResultForTest(bool value) { playResult = value; }
  int GetPlayCountForTest() const { return playCount; }
  unsigned short GetLastAliasHandleForTest() const { return lastAliasHandle; }
  int GetLastCurrentTimeForTest() const { return lastCurrentTime; }
  float GetLastRateScaleForTest() const { return lastRateScale; }
  int GetLastBlendDurationMSForTest() const { return lastBlendDurationMS; }
  int GetLastBlendOutDurationMSForTest() const {
    return lastBlendOutDurationMS;
  }
  float GetLastWeightForTest() const { return lastWeight; }

private:
  bool playResult{true};
  int playCount{0};
  unsigned short lastAliasHandle{0xFFFFU};
  int lastCurrentTime{0};
  float lastRateScale{0.0F};
  int lastBlendDurationMS{0};
  int lastBlendOutDurationMS{0};
  float lastWeight{0.0F};
};

class idAction_MoveToPoint {
public:
  ~idAction_MoveToPoint();
};

class idAIActionManager {
public:
  ~idAIActionManager();
  void Init(idAI2* value) {
    owner = value;
    ++initCount;
  }
  idAI2* GetOwnerForTest() const { return owner; }
  int GetInitCountForTest() const { return initCount; }

private:
  idAI2* owner{nullptr};
  int initCount{0};
};

class idAIDeathInfo {
public:
  idAIDeathInfo();
  ~idAIDeathInfo();
  void Set(idAIDamageInfo* info);
  const idEntity* GetAttacker() const { return attacker; }
  const idEntity* GetInflictor() const { return inflictor; }
  const idDamageImpulse& GetImpulse() const { return impulse; }
  int GetStartDeathTime() const { return startDeathTime; }

private:
  const idEntity* attacker{nullptr};
  const idEntity* inflictor{nullptr};
  idDamageImpulse impulse{};
  int startDeathTime{0};
};

class idGoreComponent {
public:
  bool CheckForGoreByName(const idAtomicString* goreName) const {
    if (goreName == nullptr || goreName->str == nullptr) return false;
    return std::find(goreNames.begin(), goreNames.end(), goreName->str) !=
           goreNames.end();
  }
  void SetGoreAvailable(const char* goreName, bool available) {
    if (goreName == nullptr) return;
    const auto found = std::find(goreNames.begin(), goreNames.end(), goreName);
    if (available && found == goreNames.end()) goreNames.emplace_back(goreName);
    if (!available && found != goreNames.end()) goreNames.erase(found);
  }

private:
  std::vector<std::string> goreNames;
};

class idAIHealth {};

class idComponentActionScript {
public:
  void Update() { ++updateCount; }
  void Start(const idEntity* owner) {
    lastStartOwner = owner;
    ++startCount;
  }
  int GetUpdateCount() const { return updateCount; }
  int GetStartCountForTest() const { return startCount; }
  const idEntity* GetLastStartOwnerForTest() const { return lastStartOwner; }
  int NumScriptActions(const idEntity* owner) const {
    (void)owner;
    return scriptActionCount;
  }
  void ClearActionScript(const idEntity* owner) {
    (void)owner;
    scriptActionCount = 0;
    ++clearCount;
  }
  void SetActionScript(const idEntity* owner,
                       const idList<idScriptAction, 5>* script,
                       idEntity* scriptExecutor, idEntity* activator,
                       bool inherited) {
    lastOwner = owner;
    lastScript = script;
    lastScriptExecutor = scriptExecutor;
    lastActivator = activator;
    lastInherited = inherited;
    scriptActionCount = script != nullptr ? script->Num() : 0;
    ++setCount;
  }
  void SetScriptActionCountForTest(int value) { scriptActionCount = value; }
  int GetClearCountForTest() const { return clearCount; }
  int GetSetCountForTest() const { return setCount; }
  const idEntity* GetLastOwnerForTest() const { return lastOwner; }
  const idList<idScriptAction, 5>* GetLastScriptForTest() const {
    return lastScript;
  }
  idEntity* GetLastScriptExecutorForTest() const {
    return lastScriptExecutor;
  }
  idEntity* GetLastActivatorForTest() const { return lastActivator; }
  bool GetLastInheritedForTest() const { return lastInherited; }

private:
  int updateCount{0};
  int startCount{0};
  int scriptActionCount{0};
  int clearCount{0};
  int setCount{0};
  const idEntity* lastOwner{nullptr};
  const idEntity* lastStartOwner{nullptr};
  const idList<idScriptAction, 5>* lastScript{nullptr};
  idEntity* lastScriptExecutor{nullptr};
  idEntity* lastActivator{nullptr};
  bool lastInherited{false};
};

class idFormationCoverMgr {};

struct idAnimWebRoute {
  std::array<idAnimWebIndex, 32> path{};
  int num{0};
  int cost{0};
};

class idAnimator_AnimWeb {
public:
  void SetDeclarationForTest(const idDeclAnimWeb* value) {
    declaration = value;
  }
  const idDeclAnimWeb* GetDeclaration() const { return declaration; }
  void SetEvent(animWebEvent_t eventType) {
    pendingEvents[static_cast<std::size_t>(eventType)] = true;
  }
  void SetEvent(animWebEvent_t eventType, idAnimWebIndex subWebIndex,
                idAnimWebIndex stateIndex,
                const idEventDef* eventDefinition,
                idAnimWebEvent::priority_t priority) {
    pendingEvents[static_cast<std::size_t>(eventType)] = true;
    lastEventType = eventType;
    lastSubWebIndex = subWebIndex;
    lastStateIndex = stateIndex;
    lastEventDefinition = eventDefinition;
    lastPriority = priority;
    ++setEventCount;
  }
  void ClearEvent(animWebEvent_t eventType) {
    pendingEvents[static_cast<std::size_t>(eventType)] = false;
  }
  void ClearEvents() { pendingEvents.fill(false); }
  bool IsEventPending(animWebEvent_t eventType) const {
    return pendingEvents[static_cast<std::size_t>(eventType)];
  }
  bool IsAnyEventPending() const {
    return std::any_of(pendingEvents.begin(), pendingEvents.end(),
                       [](bool pending) { return pending; });
  }
  bool GetDestinationIndices(idAnimWebIndex* subWebIndex,
                             idAnimWebIndex* stateIndex) const {
    if (!destinationIndicesValid) {
      return false;
    }
    *subWebIndex = destinationSubWebIndex;
    *stateIndex = destinationStateIndex;
    return true;
  }
  void ForceState(idAnimWebIndex subWebIndex, idAnimWebIndex stateIndex,
                  const blendParms_t& blendParms, int flags) {
    lastForcedSubWebIndex = subWebIndex;
    lastForcedStateIndex = stateIndex;
    lastForcedDuration = blendParms.parms.destDuration;
    lastForcedDestStartFrame = blendParms.parms.destStartFrame;
    lastForcedOriginBlend = blendParms.parms.originBlend;
    lastForceStateFlags = flags;
    ++forceStateCount;
  }
  int ChangeStateVia(idAnimWebIndex destinationSubWeb,
                     idAnimWebIndex destinationState,
                     idAnimWebIndex viaSubWeb, idAnimWebIndex viaState,
                     int interruptPath, int interruptBlend, int flags) {
    lastChangeViaDestinationSubWeb = destinationSubWeb;
    lastChangeViaDestinationState = destinationState;
    lastChangeViaSubWeb = viaSubWeb;
    lastChangeViaState = viaState;
    lastChangeViaInterruptPath = interruptPath;
    lastChangeViaInterruptBlend = interruptBlend;
    lastChangeViaFlags = flags;
    ++changeStateViaCount;
    return changeStateViaResult;
  }
  int ChangeState(idAnimWebIndex destinationSubWeb,
                  idAnimWebIndex destinationState, int interruptPath,
                  int interruptBlend, int flags) {
    lastChangeDestinationSubWeb = destinationSubWeb;
    lastChangeDestinationState = destinationState;
    lastChangeInterruptPath = interruptPath;
    lastChangeInterruptBlend = interruptBlend;
    lastChangeFlags = flags;
    ++changeStateCount;
    return changeStateResult;
  }
  void SetDestinationIndicesForTest(idAnimWebIndex subWebIndex,
                                    idAnimWebIndex stateIndex,
                                    bool valid = true) {
    destinationSubWebIndex = subWebIndex;
    destinationStateIndex = stateIndex;
    destinationIndicesValid = valid;
  }
  bool FindPath(idAnimWebIndex fromNodeIndex,
                idAnimWebIndex targetStateIndex,
                idAnimWebIndex targetSubWebIndex,
                idAnimWebIndex excludedSubWebIndex,
                idAnimWebRoute* route) {
    lastFindPathCurrentNodeIndex = fromNodeIndex;
    lastFindPathDestinationStateIndex = targetStateIndex;
    lastFindPathDestinationSubWebIndex = targetSubWebIndex;
    lastFindPathExcludedSubWebIndex = excludedSubWebIndex;
    lastFindPathInputRoute = *route;
    ++findPathCount;
    return findPathResult;
  }
  idAnimWebIndex GetCurrentNodeIndex() const { return currentNodeIndex; }
  void SetCurrentNodeIndexForTest(idAnimWebIndex value) {
    currentNodeIndex = value;
  }
  void SetCurrentStateIndicesForTest(idAnimWebIndex subWeb,
                                     idAnimWebIndex state) {
    currentSubWebIndex = subWeb;
    currentStateIndex = state;
  }
  idAnimWebIndex GetCurrentSubWebIndex() const {
    return currentSubWebIndex;
  }
  idAnimWebIndex GetCurrentStateIndex() const { return currentStateIndex; }
  void SetFindPathResultForTest(bool value) { findPathResult = value; }
  void SetChangeStateViaResultForTest(int value) {
    changeStateViaResult = value;
  }
  void SetChangeStateResultForTest(int value) { changeStateResult = value; }
  int GetSetEventCount() const { return setEventCount; }
  animWebEvent_t GetLastEventType() const { return lastEventType; }
  idAnimWebIndex GetLastSubWebIndex() const { return lastSubWebIndex; }
  idAnimWebIndex GetLastStateIndex() const { return lastStateIndex; }
  const idEventDef* GetLastEventDefinition() const {
    return lastEventDefinition;
  }
  idAnimWebEvent::priority_t GetLastPriority() const { return lastPriority; }
  int GetForceStateCountForTest() const { return forceStateCount; }
  idAnimWebIndex GetLastForcedSubWebIndexForTest() const {
    return lastForcedSubWebIndex;
  }
  idAnimWebIndex GetLastForcedStateIndexForTest() const {
    return lastForcedStateIndex;
  }
  int GetLastForcedDurationForTest() const { return lastForcedDuration; }
  int GetLastForcedDestStartFrameForTest() const {
    return lastForcedDestStartFrame;
  }
  int GetLastForcedOriginBlendForTest() const {
    return lastForcedOriginBlend;
  }
  int GetLastForceStateFlagsForTest() const { return lastForceStateFlags; }
  int GetFindPathCountForTest() const { return findPathCount; }
  idAnimWebIndex GetLastFindPathCurrentNodeIndexForTest() const {
    return lastFindPathCurrentNodeIndex;
  }
  idAnimWebIndex GetLastFindPathDestinationStateIndexForTest() const {
    return lastFindPathDestinationStateIndex;
  }
  idAnimWebIndex GetLastFindPathDestinationSubWebIndexForTest() const {
    return lastFindPathDestinationSubWebIndex;
  }
  idAnimWebIndex GetLastFindPathExcludedSubWebIndexForTest() const {
    return lastFindPathExcludedSubWebIndex;
  }
  const idAnimWebRoute& GetLastFindPathInputRouteForTest() const {
    return lastFindPathInputRoute;
  }
  int GetChangeStateViaCountForTest() const { return changeStateViaCount; }
  idAnimWebIndex GetLastChangeViaDestinationSubWebForTest() const {
    return lastChangeViaDestinationSubWeb;
  }
  idAnimWebIndex GetLastChangeViaDestinationStateForTest() const {
    return lastChangeViaDestinationState;
  }
  idAnimWebIndex GetLastChangeViaSubWebForTest() const {
    return lastChangeViaSubWeb;
  }
  idAnimWebIndex GetLastChangeViaStateForTest() const {
    return lastChangeViaState;
  }
  int GetLastChangeViaInterruptPathForTest() const {
    return lastChangeViaInterruptPath;
  }
  int GetLastChangeViaInterruptBlendForTest() const {
    return lastChangeViaInterruptBlend;
  }
  int GetLastChangeViaFlagsForTest() const { return lastChangeViaFlags; }
  int GetChangeStateCountForTest() const { return changeStateCount; }
  idAnimWebIndex GetLastChangeDestinationSubWebForTest() const {
    return lastChangeDestinationSubWeb;
  }
  idAnimWebIndex GetLastChangeDestinationStateForTest() const {
    return lastChangeDestinationState;
  }
  int GetLastChangeInterruptPathForTest() const {
    return lastChangeInterruptPath;
  }
  int GetLastChangeInterruptBlendForTest() const {
    return lastChangeInterruptBlend;
  }
  int GetLastChangeFlagsForTest() const { return lastChangeFlags; }

private:
  const idDeclAnimWeb* declaration{nullptr};
  std::array<bool, 2> pendingEvents{{false, false}};
  int setEventCount{0};
  animWebEvent_t lastEventType{ANIMWEB_EVENT_DEST_CLAMPED};
  idAnimWebIndex lastSubWebIndex{};
  idAnimWebIndex lastStateIndex{};
  const idEventDef* lastEventDefinition{nullptr};
  idAnimWebEvent::priority_t lastPriority{idAnimWebEvent::PRIORITY_LOW};
  bool destinationIndicesValid{false};
  idAnimWebIndex destinationSubWebIndex{-1};
  idAnimWebIndex destinationStateIndex{-1};
  int forceStateCount{0};
  idAnimWebIndex lastForcedSubWebIndex{-1};
  idAnimWebIndex lastForcedStateIndex{-1};
  int lastForcedDuration{0};
  int lastForcedDestStartFrame{-1};
  int lastForcedOriginBlend{-1};
  int lastForceStateFlags{-1};
  idAnimWebIndex currentNodeIndex{-1};
  idAnimWebIndex currentSubWebIndex{-1};
  idAnimWebIndex currentStateIndex{-1};
  bool findPathResult{false};
  int findPathCount{0};
  idAnimWebIndex lastFindPathCurrentNodeIndex{-1};
  idAnimWebIndex lastFindPathDestinationStateIndex{-1};
  idAnimWebIndex lastFindPathDestinationSubWebIndex{-1};
  idAnimWebIndex lastFindPathExcludedSubWebIndex{-1};
  idAnimWebRoute lastFindPathInputRoute{};
  int changeStateViaResult{0};
  int changeStateViaCount{0};
  idAnimWebIndex lastChangeViaDestinationSubWeb{-1};
  idAnimWebIndex lastChangeViaDestinationState{-1};
  idAnimWebIndex lastChangeViaSubWeb{-1};
  idAnimWebIndex lastChangeViaState{-1};
  int lastChangeViaInterruptPath{-1};
  int lastChangeViaInterruptBlend{-1};
  int lastChangeViaFlags{-1};
  int changeStateResult{0};
  int changeStateCount{0};
  idAnimWebIndex lastChangeDestinationSubWeb{-1};
  idAnimWebIndex lastChangeDestinationState{-1};
  int lastChangeInterruptPath{-1};
  int lastChangeInterruptBlend{-1};
  int lastChangeFlags{-1};
};

struct idAI2::idAIVolatile::idAIDeath {
  struct disintegrateTime_t {
    int startTime{0};
    float duration{0.0F};
    float currentTime{0.0F};
    float currentValue{0.0F};
    float endValue{0.0F};
    float startValue{0.0F};
  };

  idAIDeath();
  ~idAIDeath();

  const disintegrateTime_t& GetDisintegrateTime() const {
    return disintegrateTime;
  }
  const idEntityPtr<idVolume_CustomDeath>& GetCustomDeathVolume() const {
    return customDeathVolume;
  }
  const idEntityPtr<idEntity>& GetFocusOverrideEntity() const {
    return focusOverrideEnt;
  }

private:
  idGoreComponent goreComponent;
  idAIDeathInfo deathInfo;
  disintegrateTime_t disintegrateTime;
  idEntityPtr<idVolume_CustomDeath> customDeathVolume;
  idEntityPtr<idEntity> focusOverrideEnt;
};

class idAIEditable {
public:
  struct idAIPerception { ~idAIPerception(); };
  struct idAISpawn {
    idAISpawn();
    ~idAISpawn();
  };
  struct idInteractions {
    idInteractions();
    ~idInteractions();

    struct interactiveCameraVars_t {
      idVec3 targetOffset{0.0F, 0.0F, -5.0F};
      idAngles angleOffset{15.0F, -23.0F, 0.0F};
      float distance{55.0F};
      float fov{80.0F};
      bool towardsPlayer{true};
    };

    const idList<idEntityPtr<idInteractionCamera>, 5>& GetCameras() const {
      return interactionCameras;
    }
    const interactiveCameraVars_t& GetCameraVariables() const {
      return interactiveCameraVars;
    }
    const idStr& GetActionNodeGroup() const { return actionNodeGroup; }

  private:
    idList<idEntityPtr<idInteractionCamera>, 5> interactionCameras;
    interactiveCameraVars_t interactiveCameraVars;
    idStr actionNodeGroup;
  };
  struct idAIDeath {
    idAIDeath();
    ~idAIDeath();
  };
  struct idAIActionSettings {
    struct idAICustomRobot { idAICustomRobot(); };
    struct idAIFireSettings {
      using animationHandle_t =
          idHandle<unsigned short, invalidAliasHandle_t, 65535>;

      idAIFireSettings();
      const idList<animationHandle_t, 5>& GetAdditiveFireAnimations() const {
        return additiveFireAnims;
      }
      const idList<animationHandle_t, 5>&
      GetCrouchingAdditiveFireAnimations() const {
        return additiveFireAnimsCrouch;
      }

    private:
      idList<animationHandle_t, 5> additiveFireAnims;
      idList<animationHandle_t, 5> additiveFireAnimsCrouch;
    };
    ~idAIActionSettings();
    idRecoveredValue Clear(...);
    idRecoveredValue Init(...);
  };

  idAIEditable();
  ~idAIEditable();
};

class idAIEvent {
public:
  explicit idAIEvent(unsigned int eventClassValue = 0)
      : eventClass(eventClassValue) {}
  unsigned int GetEventClass() const { return eventClass; }
  void SetEventClass(unsigned int value) { eventClass = value; }
  idRecoveredValue GetOriginator(...);

private:
  unsigned int eventClass{0};
};

class idAIEventSphere : public idAIEvent {
public:
  using idAIEvent::idAIEvent;
};

class idAIFSMCallback {
public:
  virtual ~idAIFSMCallback();
  idRecoveredValue OnError(...);
  idRecoveredValue OnRestart(...);
  idRecoveredValue OnTransition(...);
};

class idAnimatedEntity : public idEntity {
public:
  virtual void JobSync() { ++jobSyncCount; }
  idTreeAnimator* GetTreeAnimatorFromPresentable() const;
  idRecoveredValue SetModelByName(...);
  void SetPresentable(idPresentable* value) { presentable = value; }
  void SetAimPoint(const idVec3& value) { aimPoint = value; }
  const idVec3& GetAimPoint() const { return aimPoint; }
  void SetHeadTrackBlendTime(int value) { headTrackBlendTime = value; }
  int GetHeadTrackBlendTime() const { return headTrackBlendTime; }
  int GetAnimatedEntityJobSyncCount() const { return jobSyncCount; }

private:
  idVec3 aimPoint{};
  int headTrackBlendTime{500};
  int jobSyncCount{0};
  idPresentable* presentable{nullptr};
};

class idAnimator_Drone {
public:
  ~idAnimator_Drone();
};

class idAnimatorParms_AnimWeb {
public:
  idAnimatorParms_AnimWeb();
  ~idAnimatorParms_AnimWeb();
};

class idBaseHealth {
public:
  virtual ~idBaseHealth() = default;
  void SetCurBaseHealth(float health);
  bool DamageShield_Impl(float damage, bool allowDeath, bool overflow,
                         float* remainingDamage, bool ignoreArmor);

protected:
  virtual void ClearDeadFlag() = 0;
  virtual void SetCurBaseHealth_Impl(float health) = 0;
  virtual bool Damage_Impl(float damage, bool allowDeath,
                           float* remainingDamage, bool ignoreArmor) = 0;
};

class idCheckSurroundingsState {
public:
  idRecoveredValue CanUse(...);
  idRecoveredValue FindBestLookInterestPoint(...);
  idRecoveredValue FindNetDirectionInterest(...);
  idRecoveredValue GetLookedAtElapsedTime(...);
  idRecoveredValue SetLastLookedAtTime(...);
  idRecoveredValue TraceVector(...);
  idRecoveredValue Update(...);
  idRecoveredValue UpdateDirectionDistanceExponent(...);
  idRecoveredValue UpdateLookInterests(...);
};

using idRotationTrackerRange = idRotationTracker::range_t;

struct idFocusTrackingParms {
  idAngles rate{360.0F, 360.0F, 0.0F};
  float blendScale{1.0F};
};

struct idDeclTrackingParms {
  idList<idFocusTrackingParms, 5> parms;
};

struct idHeadTrackGroup {
  idStr name;
  int blendTime{500};
  std::int16_t referenceJoint{-1};
};

class idAnimator_TrackerBase {
public:
  virtual ~idAnimator_TrackerBase() = default;
  bool ValidAsParent(int curTime, int msPerFrame) const;
  void Start(int time, int blendTime) {
    contributing = true;
    blendStartTime = time;
    lastBlendTime = std::max(0, blendTime);
    lastValidTime = time;
  }
  void End(int time, int blendTime) {
    contributing = false;
    blendStartTime = time;
    lastBlendTime = std::max(0, blendTime);
  }
  bool IsContributing() const { return contributing; }

  int trackGroupIndex{-1};
  std::int16_t referenceJointOverride{-1};
  int lastValidTime{-1};
  int blendStartTime{0};
  int lastBlendTime{0};
  bool lockTracking{false};
  idVec3 trackedPoint{};

private:
  bool contributing{false};
};

class idAimTracker : public idRotationTracker {
public:
  virtual ~idAimTracker();
  void Update(const idVec3& referenceDirection, const idVec3& idealDirection,
              const idAngles& rate, float elapsedSeconds,
              bool projectWhenBehind, bool useAcceleration);
};

struct idWanderingAim {
  void ConvergeTo(float rate, const idVec3& target);
  void AddWeaponFireKickbackImpulse(idAI2* ai, const idWeapon* weapon);
  void SmoothWander(float scale, float maxDivergence);
  idWeapon* GetEffectiveWeapon(const idAI2* ai) const;
  void Diverge(idAI2* ai);
  void Update(idAI2* ai, const idVec3& eyePosition,
              const idVec3& directionToTarget, float distanceToTarget,
              idVec3* newFocusPoint, bool showDebug);

  idVec3 error{};
  idVec3 kickback{};
  int kickCount{0};
  int lastTime{0};
  int convergeTime{0};
  unsigned int randomSeed{0xA341316CU};
};

class idFocusInfo {
public:
  idFocusInfo();
  ~idFocusInfo();

  bool HasFocus();
  bool HasActiveFocus() const;
  void Shutdown(idAnimatedEntity* owner);
  void SetConstraints(const idRotationTrackerRange* constraints);
  void AddWeaponFireKickback(const idAnimatedEntity* owner,
                             const idWeapon* weapon);
  void SetEnableTracking(const idAnimatedEntity* owner, bool enable);
  void SetDeclTrackingParms(const idDeclTrackingParms* decl);
  void SetTempFocusPoint(idAnimatedEntity* owner, const idVec3& focusPoint,
                         aiFocus_t focusType);
  void SetYawOnlyThreshold(float angle);
  void SetLockTracking(bool lock);
  void SetTrackingParmsIndex(int index);
  void Init(idAnimatedEntity* owner, aiFoci_t fociTypeValue);
  const idHeadTrackGroup* GetCurHeadTrackGroup(
      idAnimatedEntity* owner) const;
  void SwapTrackers(idAnimatedEntity* owner, int blendTime);
  void SetReferenceJointOverride(idAnimatedEntity* owner,
                                 std::int16_t jointIndex);
  void SetExplicitFocusPoint(idAnimatedEntity* owner,
                             const idVec3& focusPoint,
                             bool keepInView, int duration,
                             aiFocus_t focusTypeValue);
  void Clear(idAnimatedEntity* owner);
  void BlendOut(idAnimatedEntity* owner, int blendTime);
  void BlendIn(idAnimatedEntity* owner, int blendTime);
  void ShowTrackingInfo(idAnimatedEntity* owner, const char* label,
                        const idVec3& eyePosition, const idVec3& debugOffset);
  void Update(idAnimatedEntity* owner, idAnimator_TrackerBase* parentTracker);
  void SetCurHeadTrackGroup(idAnimatedEntity* owner, int trackGroupIndex);
  void SetFocusEntity(idAnimatedEntity* owner, idAnimatedEntity* entity,
                      bool keepInView, aimPoint_t point, int duration);

  bool IsInitialized() const { return initialized; }
  bool IsTrackingEnabled() const { return enableTracking; }
  bool IsTrackingLocked() const { return lockTracking; }
  const idVec3& GetIdealFocusPoint() const { return idealFocusPoint; }
  idEntity* GetFocusEntity() const { return focusEntity; }
  int GetFocusTimeout() const { return focusTimeout; }
  int GetCurrentAnimatorIndex() const { return curAnimatorIndex; }
  int GetTrackingParmsIndex() const { return trackingParmsIndex; }
  int GetUpdateCount() const { return updateCount; }
  const idStr& GetDebugText() const { return debugText; }
  const idAimTracker& GetAimTracker() const { return aimTracker; }
  int GetKickCount() const { return wanderingAim.kickCount; }
  idAnimator_TrackerBase* GetAnimator(int index) const {
    return index >= 0 && index < 2 ? animators[index] : nullptr;
  }
  idList<idHeadTrackGroup, 5>& HeadTrackGroups() { return headTrackGroups; }

private:
  int ResolveBlendTime(idAnimatedEntity* owner, int requested) const;

  idWanderingAim wanderingAim;
  idAimTracker aimTracker;
  aiFoci_t fociType{AIFOCI_NONE};
  aiFocus_t focusType{AIFOCUS_NOTHING};
  aimPoint_t aimPoint{AIMPOINT_EYELEVEL};
  idAnimator_TrackerBase* animators[2]{nullptr, nullptr};
  idList<idHeadTrackGroup, 5> headTrackGroups;
  const idDeclTrackingParms* declTrackingParms{nullptr};
  idEntity* focusEntity{nullptr};
  idVec3 idealFocusPoint{};
  idVec3 offset{};
  int defaultHeadTrackGroup{-1};
  int curAnimatorIndex{1};
  int trackingParmsIndex{0};
  int focusTimeout{-1};
  int updateCount{0};
  bool initialized{false};
  bool enableTracking{true};
  bool lockTracking{false};
  bool hasExplicitFocusPoint{false};
  bool keepFocusInView{false};
  idStr debugText;
};

class idFSMCallback {
public:
  virtual ~idFSMCallback();
};

class idFiniteStateMachine {
public:
  explicit idFiniteStateMachine(const char* fsmName = nullptr)
      : name(fsmName != nullptr ? fsmName : "") {}
  void Save(idFile_String* file, const char* indent) const {
    lastSaveIndent = indent != nullptr ? indent : "";
    ++saveCount;
    file->Printf("%s%s", lastSaveIndent.data, saveText.data);
  }
  bool PostEvent(int eventId, const void* eventData, unsigned int currentTime) {
    lastEventId = eventId;
    lastEventData = eventData;
    lastEventTime = currentTime;
    ++postEventCount;
    return true;
  }
  const char* GetName() const { return name.data; }
  void SetSaveTextForTest(const char* value) {
    saveText = value != nullptr ? value : "";
  }
  int GetSaveCount() const { return saveCount; }
  const char* GetLastSaveIndent() const { return lastSaveIndent.data; }
  int GetPostEventCount() const { return postEventCount; }
  int GetLastEventId() const { return lastEventId; }
  const void* GetLastEventData() const { return lastEventData; }
  unsigned int GetLastEventTime() const { return lastEventTime; }
  void SetCurrentStateForTest(const void* state) { currentState = state; }
  const void* GetCurrentStateForTest() const { return currentState; }
  void SetActionCallerState(const void* state) { actionCallerState = state; }
  const void* GetActionCallerStateForTest() const { return actionCallerState; }
  void ProcessEvent(const idEventDef* eventDefinition, const idEntity* entity) {
    lastProcessedEvent = eventDefinition;
    lastProcessedEntity = entity;
    lastProcessEventArguments[0] = {};
    lastProcessEventArguments[0].type = static_cast<unsigned char>('e');
    lastProcessEventArguments[0].value.pointer = entity;
    lastProcessEventArgumentCount = 1;
    ++processEventCount;
  }
  idEventArg ProcessEvent(const idEventDef* eventDefinition,
                          idEventArg argument0, idEventArg argument1,
                          idEventArg argument2) {
    lastProcessedEvent = eventDefinition;
    lastProcessedEntity =
        argument0.type == static_cast<unsigned char>('e')
            ? static_cast<const idEntity*>(argument0.value.pointer)
            : nullptr;
    lastProcessEventArguments[0] = argument0;
    lastProcessEventArguments[1] = argument1;
    lastProcessEventArguments[2] = argument2;
    lastProcessEventArgumentCount = 3;
    ++processEventCount;
    return {};
  }
  int GetProcessEventCount() const { return processEventCount; }
  const idEventDef* GetLastProcessedEvent() const { return lastProcessedEvent; }
  const idEntity* GetLastProcessedEntity() const { return lastProcessedEntity; }
  int GetLastProcessEventArgumentCountForTest() const {
    return lastProcessEventArgumentCount;
  }
  const idEventArg& GetLastProcessEventArgumentForTest(int index) const {
    return lastProcessEventArguments[static_cast<std::size_t>(index)];
  }
  void SetRespondsToEvent(int eventNumber, bool responds) {
    if (eventNumber >= 0 &&
        eventNumber < static_cast<int>(respondingEvents.size())) {
      respondingEvents[static_cast<std::size_t>(eventNumber)] = responds;
    }
  }
  bool RespondsTo(const idEventDef* eventDefinition) const {
    return eventDefinition != nullptr && eventDefinition->eventnum >= 0 &&
           eventDefinition->eventnum <
               static_cast<int>(respondingEvents.size()) &&
           respondingEvents[static_cast<std::size_t>(
               eventDefinition->eventnum)];
  }
  void SetCallEventResultForTest(const idEventArg& value) {
    callEventResult = value;
  }
  idEventArg* ProcessEventArgPtr(idEventArg* result,
                                 const idEventDef* eventDefinition,
                                 idEventArg* arguments) {
    lastCallEventDefinition = eventDefinition;
    lastCallEventArguments = arguments;
    ++callEventCount;
    *result = callEventResult;
    return result;
  }
  int GetCallEventCountForTest() const { return callEventCount; }
  const idEventDef* GetLastCallEventDefinitionForTest() const {
    return lastCallEventDefinition;
  }
  const idEventArg* GetLastCallEventArgumentsForTest() const {
    return lastCallEventArguments;
  }
  void SetCallback(idAIFSMCallback* value) { callback = value; }
  idAIFSMCallback* GetCallbackForTest() const { return callback; }

private:
  idStr name;
  idStr saveText{"fsm"};
  mutable idStr lastSaveIndent;
  mutable int saveCount{0};
  int postEventCount{0};
  int lastEventId{-1};
  const void* lastEventData{nullptr};
  unsigned int lastEventTime{0};
  const void* currentState{nullptr};
  const void* actionCallerState{nullptr};
  const idEventDef* lastProcessedEvent{nullptr};
  const idEntity* lastProcessedEntity{nullptr};
  int processEventCount{0};
  std::array<idEventArg, 3> lastProcessEventArguments{};
  int lastProcessEventArgumentCount{0};
  std::array<bool, 64> respondingEvents{};
  idEventArg callEventResult{};
  const idEventDef* lastCallEventDefinition{nullptr};
  idEventArg* lastCallEventArguments{nullptr};
  int callEventCount{0};
  idAIFSMCallback* callback{nullptr};
};

class idFSMManager {
public:
  struct idFSMStack {
    idFSMStack();
    void Add(idFiniteStateMachine* fsm, const char* stateName) {
      fsms.Append(fsm);
      states.Append(idStr(stateName != nullptr ? stateName : ""));
    }
    int Num() const { return fsms.Num(); }
    idFiniteStateMachine* GetFSM(int index) const { return fsms[index]; }
    const char* GetStateName(int index) const { return states[index].data; }

  private:
    idList<idFiniteStateMachine*, 5> fsms;
    idList<idStr, 5> states;
  };
  virtual ~idFSMManager();
  idFiniteStateMachine* FindFSM(const char* name) {
    return name != nullptr && std::strcmp(name, alertCycle.GetName()) == 0
               ? &alertCycle
               : nullptr;
  }
  idFiniteStateMachine* FindAlertCycleFSM() { return &alertCycle; }
  void Init(idAI2* value) {
    owner = value;
    ++initCount;
  }
  int NumFSMs() const { return 1; }
  idFiniteStateMachine* GetFSM(int index) {
    return index == 0 ? &alertCycle : nullptr;
  }
  idAI2* GetOwnerForTest() const { return owner; }
  int GetInitCountForTest() const { return initCount; }
  void SetFSMStack(const idFSMStack& value) {
    stack = value;
    ++setStackCount;
  }
  const idFSMStack& GetFSMStackForTest() const { return stack; }
  int GetSetStackCountForTest() const { return setStackCount; }

private:
  idFiniteStateMachine alertCycle{"alertCycle"};
  idAI2* owner{nullptr};
  int initCount{0};
  idFSMStack stack;
  int setStackCount{0};
};

class idFXManager {
public:
  void SetExternalTransform(const idVec3& position, const idMat3& rotation);
  int StartFX(const idVec3* position, const idMat3* axis, int gameTime,
              fxCondition_t condition) {
    lastStartPosition = *position;
    lastStartAxis = *axis;
    lastStartTime = gameTime;
    lastStartCondition = static_cast<int>(condition);
    ++startCount;
    return nextStartHandle++;
  }
  void StopFX(int gameTime, fxCondition_t condition, bool immediate) {
    lastStopTime = gameTime;
    lastStopCondition = static_cast<int>(condition);
    lastStopImmediate = immediate;
    ++stopCount;
  }
  const idVec3& GetExternalPosition() const { return externalPosition; }
  const idMat3& GetExternalRotation() const { return externalRotation; }
  bool HasExternalPositionAndRotation() const {
    return hasExternalPositionAndRotation;
  }
  int GetStartCount() const { return startCount; }
  int GetStopCount() const { return stopCount; }
  const idVec3& GetLastStartPosition() const { return lastStartPosition; }
  const idMat3& GetLastStartAxis() const { return lastStartAxis; }
  int GetLastStartTime() const { return lastStartTime; }
  int GetLastStartCondition() const { return lastStartCondition; }
  int GetLastStopTime() const { return lastStopTime; }
  int GetLastStopCondition() const { return lastStopCondition; }
  bool GetLastStopImmediate() const { return lastStopImmediate; }
  void SetNextStartHandle(int value) { nextStartHandle = value; }

private:
  idMat3 externalRotation{};
  idVec3 externalPosition{};
  bool hasExternalPositionAndRotation{false};
  idVec3 lastStartPosition{};
  idMat3 lastStartAxis{};
  int lastStartTime{-1};
  int lastStartCondition{0};
  int lastStopTime{-1};
  int lastStopCondition{0};
  bool lastStopImmediate{false};
  int startCount{0};
  int stopCount{0};
  int nextStartHandle{1};
};

class idLegsFSM {
public:
  ~idLegsFSM();
  int GetStatus() const { return status; }
  void SetStatus(int value) { status = value; }
  void Update(bool allowTransitions, int curTime, const void* eventData) {
    lastAllowTransitions = allowTransitions;
    lastUpdateTime = curTime;
    lastEventData = eventData;
    ++updateCount;
  }
  int GetUpdateCount() const { return updateCount; }
  int GetLastUpdateTime() const { return lastUpdateTime; }
  bool GetLastAllowTransitions() const { return lastAllowTransitions; }
  const void* GetLastEventData() const { return lastEventData; }
  void SetRespondsToEvent(int eventNumber, bool responds) {
    if (eventNumber >= 0 &&
        eventNumber < static_cast<int>(respondingEvents.size())) {
      respondingEvents[static_cast<std::size_t>(eventNumber)] = responds;
    }
  }
  bool RespondsTo(const idEventDef* event) const {
    return event->eventnum >= 0 &&
           event->eventnum < static_cast<int>(respondingEvents.size()) &&
           respondingEvents[static_cast<std::size_t>(event->eventnum)];
  }
  void ProcessMeleeBlocked(idAI2* source) {
    lastMeleeBlockedSource = source;
    ++meleeBlockedEventCount;
  }
  idAI2* GetLastMeleeBlockedSource() const {
    return lastMeleeBlockedSource;
  }
  int GetMeleeBlockedEventCount() const { return meleeBlockedEventCount; }

private:
  int status{0};
  int updateCount{0};
  int lastUpdateTime{0};
  bool lastAllowTransitions{false};
  const void* lastEventData{nullptr};
  std::array<bool, 64> respondingEvents{};
  idAI2* lastMeleeBlockedSource{nullptr};
  int meleeBlockedEventCount{0};
};

class idScriptFunction {
public:
  ~idScriptFunction();
};

class idSpawnFocusWithRequirement {
public:
  idSpawnFocusWithRequirement();
  idRecoveredValue RequirementFilled(...);
};

struct eboltControlNode_t { int value{0}; };
struct idCrosshairCustomIcon { int value{0}; };
struct idPonytail { int value{0}; };
class idAnimWebAI;
class idAIAction {};
class idEntitySpriteAnimator {};
class idProxyMove {};
class idTarget_AlarmSystem {};
class idTarget_FakeEnemy : public idEntity {
public:
  explicit idTarget_FakeEnemy(float weightValue = 0.0F)
      : weight(weightValue) {}
  float weight{0.0F};
};

class idTurret : public idEntity {};
class idVolume_CustomDeath {};

enum invalidAIEvent_t { INVALID_AI_EVENT = -1 };

template <int JointCount>
class idAnimator_TurretNxN {
public:
  virtual ~idAnimator_TurretNxN();
  idRecoveredValue SetParameters(...);
};

template <typename Key, typename Value>
class idHashTableT {
public:
  ~idHashTableT();
  idRecoveredValue Clear(...);
};

enum aiHealthComponent_t { AI_HEALTH_BODY = 0, AI_HEALTH_SHIELD = 1 };

template <typename Component, int Count, int DeadIndex>
class idHealthT {
public:
  struct healthComponent_t {
    Component type{static_cast<Component>(0)};
    float max{0.0F};
    float cur{0.0F};
  };

  idHealthT() {
    for (int index = 0; index < Count; ++index) {
      components[index].type = static_cast<Component>(index);
    }
  }

  void ApplyDamageAndClampToZero(float* damage, float* healthComponent) {
    const float currentHealth = *healthComponent;
    const float incomingDamage = *damage;
    AccumulateDamage(incomingDamage < currentHealth ? incomingDamage
                                                     : currentHealth);
    *healthComponent = incomingDamage < currentHealth
                           ? currentHealth - incomingDamage
                           : 0.0F;
    *damage = 0.0F;
  }
  void ApplyDamageWithOverflow(float* damage, float* healthComponent) {
    const float incomingDamage = *damage;
    const float currentHealth = *healthComponent;
    if (incomingDamage < currentHealth) {
      *healthComponent = currentHealth - incomingDamage;
      AccumulateDamage(incomingDamage);
      *damage = 0.0F;
    } else {
      *damage = incomingDamage - currentHealth;
      AccumulateDamage(currentHealth);
      *healthComponent = 0.0F;
    }
  }
  bool Damage_Impl(float damage, bool allowDeath, float* remainingDamage) {
    return DamageComponent_Impl(1, damage, allowDeath, true,
                                remainingDamage);
  }
  virtual bool DamageComponent_Impl(int type, float damage, bool allowDeath,
                                    bool overflow, float* remainingDamage) {
    if (type != DeadIndex) {
      *remainingDamage = damage;
      if (overflow) {
        ApplyDamageWithOverflow(remainingDamage, &components[type].cur);
        if (*remainingDamage > 0.0F) {
          return DamageComponent_Impl(type - 1, *remainingDamage, allowDeath,
                                      true, remainingDamage);
        }
      } else {
        ApplyDamageAndClampToZero(remainingDamage, &components[type].cur);
      }
      return false;
    }

    AccumulateDamage(damage);
    *remainingDamage = damage;
    if (!isDead) {
      *remainingDamage =
          std::max(0.0F, std::min(damage, damage - components[type].cur));
      components[type].cur -= damage;
      if (components[type].cur <= 0.0F) {
        if (allowDeath && killThreshold <= damage) {
          Kill_Impl(damage);
          SetDeadFlag();
          return true;
        }
        components[type].cur = 1.0F;
      }
    }
    return false;
  }
  float GetTotalCurHealth_Impl() const {
    float total = 0.0F;
    for (const auto& component : components) total += component.cur;
    return total;
  }
  float GetTotalMaxHealth_Impl() const {
    float total = 0.0F;
    for (const auto& component : components) total += component.max;
    return total;
  }
  virtual void HealComponent_Impl(int type, float health, bool overflow) {
    if (health < 0.0F) return;
    components[type].cur += health;
    if (type == Count - 1) {
      components[type].cur =
          std::min(components[type].cur, components[type].max);
      return;
    }
    const float overflowHealth = components[type].cur - components[type].max;
    isDead = false;
    if (overflowHealth > 0.0F) {
      components[type].cur = components[type].max;
      if (overflow) HealComponent_Impl(type + 1, overflowHealth, true);
    }
  }
  virtual void Kill_Impl(float damage) {
    if (!isDead) {
      killingBlowDamage = damage;
      isDead = true;
    }
  }
  void ReadComponents_Impl(idFile* file) {
    for (auto& component : components) {
      ReadBigEndian32(file, &component.max);
      ReadBigEndian32(file, &component.cur);
    }
    ReadBigEndian32(file, &killingBlowDamage);
    ReadBigEndian32(file, &accumulatedFrameDamage);
    ReadBigEndian32(file, &accumulatedDamageFrame);
    file->Read(&isDead, 1U);
  }
  void Reset_Impl() {
    for (auto& component : components) component.cur = component.max;
    isDead = false;
    killingBlowDamage = 0.0F;
  }
  virtual void SetDeadFlag() { isDead = true; }
  void WriteComponents_Impl(idFile* file) const {
    for (const auto& component : components) {
      WriteBigEndian32(file, &component.max);
      WriteBigEndian32(file, &component.cur);
    }
    WriteBigEndian32(file, &killingBlowDamage);
    WriteBigEndian32(file, &accumulatedFrameDamage);
    WriteBigEndian32(file, &accumulatedDamageFrame);
    const std::uint8_t dead = isDead ? 1U : 0U;
    file->Write(&dead, 1U);
  }

  void SetComponent(int type, float current, float maximum) {
    components[type].cur = current;
    components[type].max = maximum;
  }
  const healthComponent_t& GetComponent(int type) const {
    return components[type];
  }
  void SetKillThreshold(float value) { killThreshold = value; }
  virtual bool IsDead_Impl() const { return isDead; }
  bool IsDead() const { return IsDead_Impl(); }
  float GetKillingBlowDamage() const { return killingBlowDamage; }
  float GetAccumulatedFrameDamage() const { return accumulatedFrameDamage; }
  int GetAccumulatedDamageFrame() const { return accumulatedDamageFrame; }

protected:
  static void WriteBigEndian32(idFile* file, const void* value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, value, sizeof(bits));
    const std::array<std::uint8_t, 4> bytes{{
        static_cast<std::uint8_t>(bits >> 24U),
        static_cast<std::uint8_t>(bits >> 16U),
        static_cast<std::uint8_t>(bits >> 8U),
        static_cast<std::uint8_t>(bits)}};
    file->Write(bytes.data(), bytes.size());
  }
  static void ReadBigEndian32(idFile* file, void* value) {
    std::array<std::uint8_t, 4> bytes{};
    file->Read(bytes.data(), bytes.size());
    const std::uint32_t bits =
        (static_cast<std::uint32_t>(bytes[0]) << 24U) |
        (static_cast<std::uint32_t>(bytes[1]) << 16U) |
        (static_cast<std::uint32_t>(bytes[2]) << 8U) |
        static_cast<std::uint32_t>(bytes[3]);
    std::memcpy(value, &bits, sizeof(bits));
  }

  void AccumulateDamage(float damage) {
    const int frame = gameLocal->GetFrameNum();
    if (frame == accumulatedDamageFrame) {
      accumulatedFrameDamage += damage;
    } else {
      accumulatedFrameDamage = damage;
      accumulatedDamageFrame = frame;
    }
  }

  std::array<healthComponent_t, Count> components{};
  float killThreshold{0.0F};
  float killingBlowDamage{0.0F};
  float accumulatedFrameDamage{0.0F};
  int accumulatedDamageFrame{-1};
  bool isDead{false};
};

template <typename Component, int Count, int DeadIndex, int ShieldIndex>
class idShieldHealthT : public idHealthT<Component, Count, DeadIndex> {
public:
  bool DamageShield_Impl(float damage, bool allowDeath, bool overflow,
                         float* remainingDamage) {
    return this->DamageComponent_Impl(ShieldIndex, damage, allowDeath, overflow,
                                      remainingDamage);
  }
  void HealShield_Impl(float health) {
    this->HealComponent_Impl(ShieldIndex, health, false);
  }
  void Update_Impl(float regenBoost) {
    (void)regenBoost;
    if (this->IsDead_Impl() ||
        this->components[ShieldIndex].cur >=
            this->components[ShieldIndex].max) {
      return;
    }

    constexpr int gameTimeScaled = 1;
    const int frame = gameLocal->GetFrameNum();
    const int gameHz = gameLocal->GetGameHz(gameTimeScaled);
    const float secondsPerFrame = 1.0F / static_cast<float>(gameHz);
    const float secondsSinceDamage =
        static_cast<float>(frame - this->accumulatedDamageFrame) *
        secondsPerFrame;
    if (shieldRegenDelay > secondsSinceDamage) {
      lastShieldIntervalFrame = -1;
      return;
    }

    if (lastShieldIntervalFrame <= 0) {
      lastShieldIntervalFrame =
          frame - static_cast<int>(static_cast<float>(gameHz) *
                                   shieldRegenInterval);
      idLib::Printf("Regen started %.2f seconds after last damage.\n",
                    secondsSinceDamage);
    }
    if (lastShieldIntervalFrame >= 0) {
      const float intervalSeconds =
          static_cast<float>(frame - lastShieldIntervalFrame) *
          secondsPerFrame;
      if (intervalSeconds >= shieldRegenInterval) {
        lastShieldIntervalFrame = frame;
        this->HealComponent_Impl(ShieldIndex,
                                 intervalSeconds * shieldRegenRate, false);
      }
    }
  }

  void ConfigureShieldRegeneration(float delay, float rate, float interval) {
    shieldRegenDelay = delay;
    shieldRegenRate = rate;
    shieldRegenInterval = interval;
  }
  int GetLastShieldIntervalFrame() const { return lastShieldIntervalFrame; }

private:
  int lastShieldIntervalFrame{-1};
  float shieldRegenDelay{0.0F};
  float shieldRegenRate{0.0F};
  float shieldRegenInterval{0.0F};
};

template <typename T>
class idInterpolate {
public:
  idRecoveredValue GetCurrentValue(...) { return {}; }
};

template <typename T>
class idRange {
public:
  T GetRatio(T current) const;
  T minRange{};
  T maxRange{};
};

class idAutoComplete {};
extern const char* aiWalkModifierNames[7];

class idCmdSystem {
public:
  template <const char** Values, int Count>
  static idRecoveredValue ArgCompletion_String(...);
};
