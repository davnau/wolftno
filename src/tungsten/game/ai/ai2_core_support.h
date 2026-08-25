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
#include <cstddef>
#include <cstring>

enum invalidAliasHandle_t : int { INVALID_ALIAS_HANDLE = -1 };

template <typename T, typename InvalidEnum, T InvalidValue>
struct idHandle {
  T value{InvalidValue};
  bool operator==(const idHandle& other) const { return value == other.value; }
};

class idAnimator_Channel {
public:
  void InitializeForAI(const char* channelName, int weightGroupValue,
                       int filterGroupValue, float alphaValue,
                       unsigned int flagsValue) {
    initializedName = channelName != nullptr ? channelName : "";
    initializedWeightGroup = weightGroupValue;
    initializedFilterGroup = filterGroupValue;
    initializedAlpha = alphaValue;
    initializedFlags = flagsValue;
    ++initializeCount;
  }
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
  int GetInitializeCountForTest() const { return initializeCount; }
  const char* GetInitializedNameForTest() const {
    return initializedName.data;
  }
  int GetInitializedWeightGroupForTest() const {
    return initializedWeightGroup;
  }
  int GetInitializedFilterGroupForTest() const {
    return initializedFilterGroup;
  }
  float GetInitializedAlphaForTest() const { return initializedAlpha; }
  unsigned int GetInitializedFlagsForTest() const {
    return initializedFlags;
  }

private:
  idStr initializedName;
  int initializeCount{0};
  int initializedWeightGroup{0};
  int initializedFilterGroup{0};
  float initializedAlpha{0.0F};
  unsigned int initializedFlags{0};
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
  void SetStartDeathTime(int value) { startDeathTime = value; }
  unsigned int GetDeathFlags() const { return deathFlags; }
  void SetDeathFlags(unsigned int value) { deathFlags = value; }
  bool GetDeathImmediateRagdoll() const { return deathImmediateRagdoll; }
  void SetDeathImmediateRagdoll(bool value) {
    deathImmediateRagdoll = value;
  }
  const char* GetDebugDeathAnimState() const {
    return debugDeathAnimState.data;
  }
  void SetDebugDeathAnimState(const char* value) {
    debugDeathAnimState = value != nullptr ? value : "";
  }
  const idDeclAnimWebNode* GetDeathAnimationNode() const {
    return deathAnimationNode;
  }
  void SetDeathAnimationNode(const idDeclAnimWebNode* value) {
    deathAnimationNode = value;
  }
  unsigned int GetAnimationFlags() const { return animationFlags; }
  void AddAnimationFlags(unsigned int value) { animationFlags |= value; }
  void ConfigureForTest(const idEntity* attackerValue,
                        const idEntity* inflictorValue,
                        const idDamageImpulse& impulseValue,
                        int startDeathTimeValue,
                        unsigned int deathFlagsValue) {
    attacker = attackerValue;
    inflictor = inflictorValue;
    impulse = impulseValue;
    startDeathTime = startDeathTimeValue;
    deathFlags = deathFlagsValue;
  }

private:
  const idEntity* attacker{nullptr};
  const idEntity* inflictor{nullptr};
  idDamageImpulse impulse{};
  int startDeathTime{-1};
  unsigned int deathFlags{0};
  bool deathImmediateRagdoll{true};
  idStr debugDeathAnimState;
  const idDeclAnimWebNode* deathAnimationNode{nullptr};
  unsigned int animationFlags{0};
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

class idAIHealth {
public:
  void ClearDeadFlag() {
    dead = false;
    ++clearDeadFlagCount;
  }
  void SetCurBaseHealth_Impl(float health) {
    currentBaseHealth = health;
    ++setCurBaseHealthCount;
  }
  float GetBaseHealthRatio_Impl() const {
    return maximumBaseHealth > 0.0F
               ? currentBaseHealth / maximumBaseHealth
               : 0.0F;
  }
  float GetMaximumBaseHealthForTest() const { return maximumBaseHealth; }
  void SetMaximumBaseHealthForTest(float health) {
    maximumBaseHealth = health;
  }
  void Kill_Impl(float killingBlowDamage = 0.0F) {
    currentBaseHealth = 0.0F;
    lastKillingBlowDamage = killingBlowDamage;
  }
  void SetDeadFlag() { dead = true; }

  float GetTotalCurHealth_Impl() const { return currentBaseHealth; }
  float GetTotalMaxHealth_Impl() const { return maximumBaseHealth; }

  bool IsDead_Impl() const { return dead; }
  float GetCurBaseHealthForTest() const { return currentBaseHealth; }
  float GetLastKillingBlowDamageForTest() const {
    return lastKillingBlowDamage;
  }
  int GetClearDeadFlagCountForTest() const { return clearDeadFlagCount; }
  int GetSetCurBaseHealthCountForTest() const {
    return setCurBaseHealthCount;
  }
  void ResetCallCountsForTest() {
    clearDeadFlagCount = 0;
    setCurBaseHealthCount = 0;
  }

private:
  float currentBaseHealth{0.0F};
  float maximumBaseHealth{100.0F};
  float lastKillingBlowDamage{0.0F};
  bool dead{false};
  int clearDeadFlagCount{0};
  int setCurBaseHealthCount{0};
};

class idComponentActionScript {
public:
  void Update() { ++updateCount; }
  void Start(const idEntity* owner) {
    lastStartOwner = owner;
    running = true;
    ++startCount;
  }
  bool RunningActionScript(const idEntity* owner) const {
    lastRunningQueryOwner = owner;
    ++runningQueryCount;
    return running;
  }
  void SetRunningForTest(bool value) { running = value; }
  int GetRunningQueryCountForTest() const { return runningQueryCount; }
  const idEntity* GetLastRunningQueryOwnerForTest() const {
    return lastRunningQueryOwner;
  }
  void ResetRunningQueryCountForTest() const {
    runningQueryCount = 0;
    lastRunningQueryOwner = nullptr;
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
    running = false;
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
    running = scriptActionCount > 0;
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
  bool running{false};
  mutable int runningQueryCount{0};
  mutable const idEntity* lastRunningQueryOwner{nullptr};
};

class idFormationCoverMgr {};

struct idAnimWebRoute {
  std::array<idAnimWebIndex, 32> path{};
  int num{0};
  int cost{0};
};

class idAnimator_AnimWeb {
public:
  void InitializeForAI(idAI2* ownerValue, const char* animatorName,
                       const idDeclAnimWeb* declarationValue,
                       const char* initialSubWeb, const char* initialState,
                       const std::vector<std::string>& activeLayerValues,
                       bool enabledValue, int currentTime) {
    owner = ownerValue;
    name = animatorName != nullptr ? animatorName : "";
    declaration = declarationValue;
    initialSubWebName = initialSubWeb != nullptr ? initialSubWeb : "";
    initialStateName = initialState != nullptr ? initialState : "";
    activeLayers = activeLayerValues;
    enabled = enabledValue;
    initTime = currentTime;
    ++initializeCount;
  }
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
  void SetSelect01ForTest(float value) { select01 = value; }
  float GetSelect01ForTest() const { return select01; }
  void SetMeleeAttackForTest(float value) { meleeAttack = value; }
  float GetMeleeAttackForTest() const { return meleeAttack; }
  idAI2* GetOwnerForTest() const { return owner; }
  const char* GetInitializedNameForTest() const { return name.data; }
  const char* GetInitialSubWebNameForTest() const {
    return initialSubWebName.data;
  }
  const char* GetInitialStateNameForTest() const {
    return initialStateName.data;
  }
  const std::vector<std::string>& GetActiveLayersForTest() const {
    return activeLayers;
  }
  bool IsEnabledForTest() const { return enabled; }
  int GetInitializeCountForTest() const { return initializeCount; }
  int GetInitTimeForTest() const { return initTime; }

private:
  idAI2* owner{nullptr};
  idStr name;
  idStr initialSubWebName;
  idStr initialStateName;
  std::vector<std::string> activeLayers;
  int initializeCount{0};
  int initTime{0};
  bool enabled{false};
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
  float select01{0.0F};
  float meleeAttack{0.0F};
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

class idDeclMD6;
class idDeclEntityDef;
class idSoundShader;
class idDamageGroup;
class idDeclElectricBolt;
class idDeclGore;
class idDeclVoiceOver;

class idSpawnFocusWithRequirement {
public:
  idSpawnFocusWithRequirement();
  bool RequirementFilled(const idInventoryCollection* inventory) const;
  void ConfigureForTest(const idDeclEntityDef* definition,
                        std::initializer_list<const char*> requirementNames) {
    giveEnt = definition;
    requirements.Clear();
    for (const char* name : requirementNames) {
      requirements.Append(idAtomicString(name != nullptr ? name : ""));
    }
  }
  const idList<idAtomicString, 5>& GetRequirementsForTest() const {
    return requirements;
  }
  const idDeclEntityDef* GetGiveEntityDefinitionForTest() const {
    return giveEnt;
  }

private:
  const idDeclEntityDef* giveEnt{nullptr};
  idList<idAtomicString, 5> requirements;
};
static_assert(sizeof(idSpawnFocusWithRequirement) == 0x20,
              "idSpawnFocusWithRequirement ABI size drift");

struct idAIDeathSound {
  const idDeclDamage* damageType{nullptr};
  const idDeclVoiceOver* voiceOver{nullptr};
  const idSoundShader* soundShader{nullptr};
};
static_assert(sizeof(idAIDeathSound) == 0x18,
              "idAIDeathSound ABI size drift");

struct idSpawnEntityReference {
  int value{0x1FFF};
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

    int ignoreDamageType;
    idSpawnEntityReference trigger;
    idSpawnEntityReference triggerOnDying;
    int removeTime;
    float fadeOutTime;
    float bodyFadeTime;
    bool bAvoidDangerZone;
    bool bReportDangerZone;
    float dangerZoneFactor_Death;
    bool bCorpseDetection;
    idAnimWebPath deathAnim;
    idList<idAIDeathSound, 5> deathSounds;
    const idDeclGore* goreDef;
    bool removeHealthWhenDamaged : 1;
    bool shouldDropLootBox : 1;
    bool useArmorSurfTypeOnDG : 1;
    bool noBodyRemoval : 1;
    bool useRagdoll : 1;
    bool manuallyStartedRagdoll : 1;
    bool useDeadPhys : 1;
    bool slideAwayFromCollision : 1;
    bool instantRagdoll : 1;
    float disintigrateDuration;
    int deadEffectDuration;
    int deathAnimAWResponseFlags;
    bool canOnlyBeKilledByPlayer : 1;
    idList<idStr, 5> canOnlyBeKilledDamageGroups;
    idSpawnFocusWithRequirement focusOverride;
    const idDeclEntityDef* deathSpawnEntityDef;
  };
  struct idAIActionSettings {
    using idSpawnEntityReference = ::idSpawnEntityReference;

    struct idAICoverSettings {
      bool usesCombatScenePoints{true};
      bool usesClose{true};
      bool usesIdle{true};
      bool usesEmergencyExit{false};
      bool dodgeMovingToScenepoint{false};
      int scenePointSelectionFlags{3};
      int coverFlags{0x36};
      float coverMinRange{50.0F};
      float coverMaxRange{1000.0F};
      float coverTargetMinRange{200.0F};
      float coverRandomWeight{0.1F};
      float coverHistoryWeight{1.0F};
      float coverVerticalWeight{1.0F};
      float coverTacticalWeight{0.25F};
      float coverTargetTimeout{-1.0F};
      float returnToCoverDelay{1.0F};
    };

    struct idAIMeleeSettings {
      float meleeMaxRange{75.0F};
      float meleeLongMaxRange{0.0F};
      float playerTakedownRange{0.0F};
      float playerTakedownMaxHealth{0.0F};
      const idDeclEntityDef* playerTakedownDecl{nullptr};
      bool instantHitExcludeFromTrace{false};
    };

    struct idAISearchSettings {
      bool usesSearchScenePoints{true};
      float searchMinRange{50.0F};
      float searchMaxRange{-1.0F};
    };

    struct idAIRoamSettings {
      bool usesRoamScenePoints{true};
      float roamMinRange{50.0F};
      float roamMaxRange{-1.0F};
      float idleTalkPeriod{-1.0F};
    };

    struct idAILookSettings {
      bool usesLookScenePoints{true};
      float lookMinRange{50.0F};
      float lookMaxRange{-1.0F};
      float minLookSPPeriod{5.0F};
      float combatTargetDelayLook{-1.0F};
    };

    struct idAIFireSettings {
      using animationHandle_t =
          idHandle<unsigned short, invalidAliasHandle_t, 65535>;

      struct idAIFireTiming {
        bool bUseFireTiming{false};
        float minBurstDuration{1.0F};
        float maxBurstDuration{3.0F};
        float minBurstPause{0.2F};
        float maxBurstPause{1.0F};
        float missTime{0.0F};
        float missTimeMinRange{300.0F};
        bool missTrackAlongGround{false};
        float spotLostFire{2.0F};
        float spotGainedFire{0.0F};
        float minSpotLostRange{75.0F};
        bool predictTargetMovement{false};
        float focusCompensationTime{-1.0F};
      };

      idAIFireSettings();
      const idList<animationHandle_t, 5>& GetAdditiveFireAnimations() const {
        return additiveFireAnims;
      }
      const idList<animationHandle_t, 5>&
      GetCrouchingAdditiveFireAnimations() const {
        return additiveFireAnimsCrouch;
      }

      float aimAngle{15.0F};
      float gangstaPoseFraction{0.0F};
      float gangstaDuration{3.0F};
      float maxFireRange{-1.0F};
      bool usesGrenades{false};
      const idDeclMD6* fakeGrenadeMD6{nullptr};
      const idDeclEntityDef* fakeGrenadeExplosion{nullptr};
      float secondaryFireRange{-1.0F};
      bool useReloadAnimWhenMoving{false};
      idList<animationHandle_t, 5> additiveFireAnims;
      idList<animationHandle_t, 5> additiveFireAnimsCrouch;
      float additiveFireAnimBlendTime{0.0F};
      float freeAmmoDistance{-1.0F};
      float goNutsRange{0.0F};
      float safeReloadFraction{0.0F};
      float aimDot{0.0F};
      float aimDotUpsideDown{0.0F};
      float gangstaAimDot{0.0F};
      idAIFireTiming fireTiming;
    };

    struct idAICloseSettings {
      struct idPeriodicVoice {
        bool use{false};
        int minTime{1000};
        int maxTime{1000};
      };

      int closeFlags{1};
      float closeMinRange{0.0F};
      float closeMaxRange{-1.0F};
      float stopOnSpotRange{500.0F};
      float noSpotTimeBeforeWalking{3.0F};
      float maxRunRange{-1.0F};
      bool useEvasiveManeuvers{false};
      bool noStopOnSpotWhenBoreBlocked{false};
      int evadeInterval{1000};
      int evadeMinDistance{300};
      bool useOutOfReach{false};
      float outOfReachGentlemanRange{0.0F};
      idPeriodicVoice periodicVoice;
    };

    struct idAIDodgeSettings {
      int dodgeFlags{0xF6};
      float dodgeMinRange{100.0F};
      float dodgeMaxRange{-1.0F};
      float tauntMinRange{100.0F};
      float dodgePeriod{2.0F};
      float dodgeAnimPeriod{4.0F};
      bool detectGrenadesInAir{false};
    };

    struct idAITauntSettings {
      float tauntMinInterval{4.0F};
      float tauntMaxInterval{7.0F};
    };

    struct idAISquadSettings {
      idSpawnEntityReference squadLeader;
      bool playerIsLeader{false};
      float leaderRange{200.0F};
      float scenePointRange{500.0F};
      bool squadActive{true};
    };

    struct idAIOutOfReachSettings {
      float distance{32.0F};
    };

    struct idAICustomRobot {
      idAICustomRobot();
      bool usesRobot{false};
      bool isBoss{true};
      idAtomicString stageGroup;
      float stageHealth{300.0F};
      float curStageHealth{300.0F};
      float bossHealth{1.0F};
      float chargeHealthFract{0.15000001F};
      float chargeTime{5.0F};
      float shootHealthDrain{0.0F};
      bool autoRecharge{false};
      bool useRechargeAnim{false};
      bool isCharging{false};
      int chargeStartTime{-1};
      int chargeDoneTime{-1};
      int numStages{1};
      idSpawnEntityReference triggerOnPowerOut;
      idSpawnEntityReference triggerOnChargingDone;
      idList<idSpawnEntityReference, 5> stageTriggers;
      int curStage{0};
      int noFireTime{-1};
      int immuneDoneTime{-1};
      const idSoundShader* turnfastSound{nullptr};
      idVec3 lastBodyDir{1.0F, 0.0F, 0.0F};
      int nextTurnfastTime{-1};
      int lastShootTime{-1};
      int nextMTSPCheck{-1};
    };

    struct idAIRobotBlind {
      bool bUsesBlindness{false};
      float blindedDuration{-1.0F};
      float reblindCooldown{0.0F};
      int endBlindedTime{0};
      int endReblindCooldown{0};
      idDamageGroup* blindedDamageGroup{nullptr};
    };

    struct idAIEMPStun {
      bool bUsesEMPStun{false};
      float stunDuration{-1.0F};
      bool isEMPMagnetic{false};
      bool isNeverEMPMagnetic{false};
      int endStunnedTime{0};
    };

    struct idAISleeping {
      bool bUsesSleeping{false};
      float sleepRadius{100.0F};
      bool isSleeping{false};
    };

    struct idRobotDeath {
      float timeToCritical{-1.0F};
      float timeCriticalToExplode{-1.0F};
      const idDeclDamage* explodeDamage{nullptr};
      float stopEffectsDelay{1.0F};
      int criticalTimestamp{-1};
      int explodeTimestamp{-1};
      int deadTimeStamp{-1};
      bool deathSetup{false};
    };

    struct idAITesla {
      bool usesTesla{false};
      bool useTeslaAnim{true};
      bool isHeavyGuardGun{false};
      float teslaMeleeRange{0.0F};
      float teslaAASRange{0.0F};
      float teslaMinRange{0.0F};
      float teslaRange{500.0F};
      float teslaChargeTime{5.0F};
      float teslaCoolDownTime{5.0F};
      float teslaExplodeDelay{0.25F};
      float teslaMaxRangeDamageScale{1.0F};
      bool useTeslaWhileBlinded{false};
      bool useTeslaBombGroundEffects{false};
      bool spawnTeslaBombInFOV{false};
      float teslaTargetOffset{64.0F};
      float inhibitFireTime{0.0F};
      const idDeclDamage* teslaDamageDecl{nullptr};
      const idDeclDamage* teslaBombDamageDecl{nullptr};
      float teslaBombDangerZone{0.0F};
      const idDeclElectricBolt* teslaBoltDecl{nullptr};
      float teslaBoltFlickerInterval{0.5F};
      float teslaBlindFireDuration{0.0F};
      const idDeclEntityDef* teslaBombFxDecl{nullptr};
    };

    ~idAIActionSettings();
    void Clear();
    idRecoveredValue Init(...);

    bool usesTurrets{false};
    bool stationaryMode{false};
    bool noLowerBodyTurns{false};
    float enemyTrailUpdatePeriod{4.0F};
    bool isElite{false};
    bool isDualWielder{false};
    bool isCommander{false};
    bool isHeavy{false};
    bool isKampHund{false};
    bool isSuperSoldat{false};
    idAICoverSettings cover;
    idAIMeleeSettings melee;
    idAISearchSettings search;
    idAIRoamSettings roam;
    idAILookSettings look;
    idAIFireSettings fire;
    idAICloseSettings close;
    idAIDodgeSettings dodge;
    idAITauntSettings taunt;
    idAISquadSettings squad;
    idAIOutOfReachSettings outOfReach;
    idAICustomRobot customRobot;
    idAIRobotBlind robotBlinding;
    idAIEMPStun EMPStun;
    idAISleeping sleeping;
    idRobotDeath robotDeath;
    idAITesla tesla;
  };

  idAIEditable();
  ~idAIEditable();
};

static_assert(sizeof(idAIEditable::idAIActionSettings) == 0x310,
              "idAIActionSettings ABI size drift");
static_assert(offsetof(idAIEditable::idAIActionSettings, cover) == 0x10);
static_assert(offsetof(idAIEditable::idAIActionSettings, melee) == 0x48);
static_assert(offsetof(idAIEditable::idAIActionSettings, search) == 0x68);
static_assert(offsetof(idAIEditable::idAIActionSettings, roam) == 0x74);
static_assert(offsetof(idAIEditable::idAIActionSettings, look) == 0x84);
static_assert(offsetof(idAIEditable::idAIActionSettings, fire) == 0x98);
static_assert(offsetof(idAIEditable::idAIActionSettings, close) == 0x148);
static_assert(offsetof(idAIEditable::idAIActionSettings, dodge) == 0x180);
static_assert(offsetof(idAIEditable::idAIActionSettings, taunt) == 0x19C);
static_assert(offsetof(idAIEditable::idAIActionSettings, squad) == 0x1A4);
static_assert(offsetof(idAIEditable::idAIActionSettings, outOfReach) == 0x1B8);
static_assert(offsetof(idAIEditable::idAIActionSettings, customRobot) == 0x1C0);
static_assert(offsetof(idAIEditable::idAIActionSettings, robotBlinding) ==
              0x248);
static_assert(offsetof(idAIEditable::idAIActionSettings, EMPStun) == 0x268);
static_assert(offsetof(idAIEditable::idAIActionSettings, sleeping) == 0x278);
static_assert(offsetof(idAIEditable::idAIActionSettings, robotDeath) == 0x288);
static_assert(offsetof(idAIEditable::idAIActionSettings, tesla) == 0x2B0);
static_assert(sizeof(idAIEditable::idAIDeath) == 0xC8,
              "idAIDeath ABI size drift");
static_assert(offsetof(idAIEditable::idAIDeath, trigger) == 0x4);
static_assert(offsetof(idAIEditable::idAIDeath, triggerOnDying) == 0x8);
static_assert(offsetof(idAIEditable::idAIDeath, deathAnim) == 0x28);
static_assert(offsetof(idAIEditable::idAIDeath, deathSounds) == 0x50);
static_assert(offsetof(idAIEditable::idAIDeath, canOnlyBeKilledDamageGroups) ==
              0x88);
static_assert(offsetof(idAIEditable::idAIDeath, focusOverride) == 0xA0);
static_assert(offsetof(idAIEditable::idAIDeath, deathSpawnEntityDef) == 0xC0);

class idAIEvent {
public:
  explicit idAIEvent(unsigned int eventClassValue = 0)
      : eventClass(eventClassValue) {}
  unsigned int GetEventClass() const { return eventClass; }
  void SetEventClass(unsigned int value) { eventClass = value; }
  const idEntity* GetOriginator() const { return originator; }
  const idEntity* GetInstigator() const { return instigator; }
  const idVec3& GetOrigin() const { return origin; }
  int GetHandle() const { return handle; }
  void ConfigureEventForTest(const idEntity* eventOriginator,
                             const idEntity* eventInstigator,
                             const idVec3& eventOrigin) {
    originator = eventOriginator;
    instigator = eventInstigator;
    origin = eventOrigin;
  }
  void SetHandleForTest(int value) { handle = value; }
  idRecoveredValue GetOriginator(...);

private:
  unsigned int eventClass{0};
  const idEntity* originator{nullptr};
  const idEntity* instigator{nullptr};
  idVec3 origin{};
  int handle{-1};
};

class idAIEventSphere : public idAIEvent {
public:
  using idAIEvent::idAIEvent;
  float GetRadius() const {
    ++radiusQueryCount;
    return radius;
  }
  void SetRadiusForTest(float value) { radius = value; }
  int GetRadiusQueryCountForTest() const { return radiusQueryCount; }
  void ResetRadiusQueryCountForTest() const { radiusQueryCount = 0; }

private:
  float radius{0.0F};
  mutable int radiusQueryCount{0};
};

class idAIEventProjectedSphere : public idAIEventSphere {
public:
  bool HasProjection() const { return hasProjection; }
  const idVec3& GetProjectedOrigin() const { return projectedOrigin; }
  void ConfigureProjectionForTest(bool projected,
                                  const idVec3& projectedPosition) {
    hasProjection = projected;
    projectedOrigin = projectedPosition;
  }

private:
  idVec3 projectedOrigin{};
  bool hasProjection{false};
};

class idAIEvent_Vehicle : public idAIEvent {
public:
  float GetRadius() const { return radius; }
  void SetRadiusForTest(float value) { radius = value; }

private:
  float radius{0.0F};
};

class idAIEventSound : public idAIEventSphere {
public:
  float GetIntensity(const idEntity* entity) const {
    ++intensityQueryCount;
    lastIntensityEntity = entity;
    return intensity;
  }
  bool IsSoundType() const { return soundType; }
  void SetIntensityForTest(float value) { intensity = value; }
  void SetSoundTypeForTest(bool value) { soundType = value; }
  int GetIntensityQueryCountForTest() const { return intensityQueryCount; }
  const idEntity* GetLastIntensityEntityForTest() const {
    return lastIntensityEntity;
  }
  void ResetIntensityObservationsForTest() const {
    intensityQueryCount = 0;
    lastIntensityEntity = nullptr;
  }

private:
  float intensity{0.0F};
  bool soundType{true};
  mutable int intensityQueryCount{0};
  mutable const idEntity* lastIntensityEntity{nullptr};
};

class idAIEventVoice : public idAIEventSound {
public:
  int GetVoiceMessageType() const {
    ++voiceMessageQueryCount;
    return voiceMessageType;
  }
  void SetVoiceMessageTypeForTest(int value) { voiceMessageType = value; }
  int GetVoiceMessageQueryCountForTest() const {
    return voiceMessageQueryCount;
  }
  void ResetVoiceMessageQueryCountForTest() const {
    voiceMessageQueryCount = 0;
  }

private:
  int voiceMessageType{7};
  mutable int voiceMessageQueryCount{0};
};

class idVehicle : public idEntity {
public:
  idEntity* GetDriver() const { return driver; }
  void SetDriver(idEntity* value) { driver = value; }

private:
  idEntity* driver{nullptr};
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
  bool ShouldHoldFire(idAI2* ai, const idEntity* enemy,
                      aiHoldFireReason_t* holdFireReason) {
    ++holdFireQueryCount;
    lastHoldFireAI = ai;
    lastHoldFireEnemy = enemy;
    if (ai == nullptr) return false;

    const int currentTime =
        gameLocal != nullptr ? gameLocal->GetGameTimeMs() : 0;
    if (launchDoneTime > 0 && currentTime < launchDoneTime) return true;

    if (currentFireWindupTick > 0) {
      const int windupFinishedTime =
          currentFireWindupTick +
          static_cast<int>(ai->GetSpotGainedFireTime() * 1000.0F);
      if (currentTime <= windupFinishedTime) {
        if (holdFireReason != nullptr) {
          *holdFireReason = AI_HOLDFIRE_REASON_NOREGAIN;
        }
        return true;
      }
      keepFiringStopTick = (std::max)(
          keepFiringStopTick,
          currentTime +
              static_cast<int>(ai->GetSpotLostFireTime() * 1000.0F));
      currentFireWindupTick = 0;
      if (holdFireReason != nullptr) {
        *holdFireReason = AI_HOLDFIRE_REASON_NONE;
      }
      return false;
    }

    if (keepFiringStopTick > 0) {
      if (ai->GetSpottedDuration(enemy, currentTime) >= 0) {
        keepFiringStopTick = (std::max)(
            keepFiringStopTick,
            currentTime +
                static_cast<int>(ai->GetSpotLostFireTime() * 1000.0F));
      }
      if (currentTime >= keepFiringStopTick) {
        nextFireWindupTick = currentTime + 500;
        keepFiringStopTick = 0;
      }
      if (holdFireReason != nullptr) {
        *holdFireReason = AI_HOLDFIRE_REASON_NONE;
      }
      return false;
    }

    if (noFireMoving && goalRange > 200.0F) {
      if (holdFireReason != nullptr) {
        *holdFireReason = AI_HOLDFIRE_REASON_NOTRACK;
      }
      return true;
    }
    return false;
  }
  void Update(idAI2* ai, const idMat3& axis, const idVec3& velocity) {
    lastAI = ai;
    lastAxis = axis;
    lastVelocity = velocity;
    ++updateCount;
  }
  int GetUpdateCountForTest() const { return updateCount; }
  idAI2* GetLastAIForTest() const { return lastAI; }
  const idMat3& GetLastAxisForTest() const { return lastAxis; }
  const idVec3& GetLastVelocityForTest() const { return lastVelocity; }
  void ConfigureHoldFireForTest(int launchDone, int fireWindup,
                                int keepFiringStop, bool inhibitWhileMoving,
                                float range) {
    launchDoneTime = launchDone;
    currentFireWindupTick = fireWindup;
    keepFiringStopTick = keepFiringStop;
    noFireMoving = inhibitWhileMoving;
    goalRange = range;
  }
  int GetHoldFireQueryCountForTest() const { return holdFireQueryCount; }
  idAI2* GetLastHoldFireAIForTest() const { return lastHoldFireAI; }
  const idEntity* GetLastHoldFireEnemyForTest() const {
    return lastHoldFireEnemy;
  }
  int GetNextFireWindupTickForTest() const { return nextFireWindupTick; }

private:
  idAI2* lastAI{nullptr};
  idMat3 lastAxis{};
  idVec3 lastVelocity{};
  int updateCount{0};
  idAI2* lastHoldFireAI{nullptr};
  const idEntity* lastHoldFireEnemy{nullptr};
  int holdFireQueryCount{0};
  int launchDoneTime{0};
  int currentFireWindupTick{0};
  int keepFiringStopTick{0};
  int nextFireWindupTick{0};
  bool noFireMoving{false};
  float goalRange{0.0F};
};

class idAnimStack;

class idMD6Blend {
public:
  enum blendOp_t : int {
    BLENDOP_REPLACE = 0,
    BLENDOP_ADD = 1,
  };
  enum originBlend_t : int {
    ORIGINBLEND_DEFAULT = 0,
    ORIGINBLEND_SOURCE = 1,
    ORIGINBLEND_DEST = 2,
  };
};

class idAnimatorParms_AnimWeb {
public:
  idAnimatorParms_AnimWeb(idAnimStack* animStack, const char* name,
                          idMD6Blend::blendOp_t blendOp,
                          idMD6Blend::originBlend_t originBlend,
                          md6WeightGroup_t weightGroup,
                          md6WeightGroup_t filterGroup, float alpha,
                          const idDeclAnimWeb* webDecl,
                          const char* initialSubWebName,
                          const char* initialStateName, int curTime);
  ~idAnimatorParms_AnimWeb();

  idAnimStack* animStack{nullptr};
  idStr name;
  idMD6Blend::blendOp_t blendOp{idMD6Blend::BLENDOP_REPLACE};
  idMD6Blend::originBlend_t originBlend{idMD6Blend::ORIGINBLEND_DEFAULT};
  md6WeightGroup_t weightGroup{MD6_WEIGHTGROUP_ALL};
  md6WeightGroup_t filterGroup{MD6_WEIGHTGROUP_ALL};
  float alpha{0.0F};
  const idDeclAnimWeb* webDecl{nullptr};
  idStr initialStateName;
  idStr initialSubWebName;
  int curTime{0};
  idList<idAtomicString, 5> activeLayers;
};

class idBaseHealth {
public:
  virtual ~idBaseHealth() = default;
  void SetCurBaseHealth(float health) {
    if (health > 0.0F) {
      ClearDeadFlag();
      SetCurBaseHealth_Impl(health);
    }
  }
  bool DamageShield_Impl(float damage, bool allowDeath, bool overflow,
                         float* remainingDamage, bool ignoreArmor) {
    (void)overflow;
    return Damage_Impl(damage, allowDeath, remainingDamage, ignoreArmor);
  }

protected:
  virtual void ClearDeadFlag() = 0;
  virtual void SetCurBaseHealth_Impl(float health) = 0;
  virtual bool Damage_Impl(float damage, bool allowDeath,
                           float* remainingDamage, bool ignoreArmor) = 0;
};

class idCheckSurroundingsState {
public:
  bool CanUse(const idAI2* ai) const;
  float FindBestLookInterestPoint(const idAI2* ai, idVec3& lookTo) const;
  int GetLookedAtElapsedTime(const idEntity* entity) const;
  void SetLastLookedAtTime(const idEntity* entity, int setTime);
  void Update(const idAI2* ai);
  void SetDistanceExponentForTest(int index, int value) {
    distanceExponent[static_cast<std::size_t>(index)] = value;
  }
  void SetLookInterestForTest(int index, float value) {
    lookInterest[static_cast<std::size_t>(index)] = value;
  }
  float FindNetDirectionInterestForTest(
      const idAI2* ai, const idVec3& testDirection) const {
    return FindNetDirectionInterest(ai, testDirection);
  }
  bool TraceVectorForTest(const idAI2* ai, const idVec3& from,
                          const idVec3& to) const {
    return TraceVector(ai, from, to);
  }
  int UpdateDirectionDistanceExponentForTest(int directionToTrace,
                                             const idAI2* ai) {
    return UpdateDirectionDistanceExponent(directionToTrace, ai);
  }
  void UpdateLookInterestsForTest(const idAI2* ai) {
    UpdateLookInterests(ai);
  }
  int GetDistanceExponentForTest(int index) const {
    return distanceExponent[static_cast<std::size_t>(index)];
  }
  float GetLookInterestForTest(int index) const {
    return lookInterest[static_cast<std::size_t>(index)];
  }
  float GetLookForwardInterestForTest() const {
    return lookForwardInterest;
  }
  void SetLookForwardInterestForTest(float value) {
    lookForwardInterest = value;
  }
  int GetNextDirectionToTraceForTest() const {
    return nextDirectionToTrace;
  }
  void SetNextDirectionToTraceForTest(int value) {
    nextDirectionToTrace = value;
  }

private:
  bool TraceVector(const idAI2* ai, const idVec3& from,
                   const idVec3& to) const;
  int UpdateDirectionDistanceExponent(int directionToTrace,
                                      const idAI2* ai);
  void UpdateLookInterests(const idAI2* ai);
  float FindNetDirectionInterest(const idAI2* ai,
                                 const idVec3& testDirection) const;

  float lookForwardInterest{100.0F};
  std::array<int, 8> distanceExponent{};
  std::array<float, 8> lookInterest{};
  int nextDirectionToTrace{0};
  std::array<idSpawnId, 4> lookedAtEntitySpawnIDs{{
      idSpawnId{0x1fff}, idSpawnId{0x1fff}, idSpawnId{0x1fff},
      idSpawnId{0x1fff}}};
  std::array<int, 4> lookedAtEntitiesTime{};
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
  float GetMaximumYawRadiansForBodyOrientation() const {
    constexpr float degreesToRadians = 0.017453292519943295F;
    return std::max(constraints.yawLeft, constraints.yawRight) *
           degreesToRadians;
  }
  float GetMaximumYawDotForBodyOrientation() const {
    return std::cos(GetMaximumYawRadiansForBodyOrientation());
  }
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
  void SetFocusEntity(idAnimatedEntity* owner, const idEntity* entity,
                      bool keepInView, aimPoint_t point, int duration);

  bool IsInitialized() const { return initialized; }
  bool IsTrackingEnabled() const { return enableTracking; }
  bool IsTrackingLocked() const { return lockTracking; }
  const idVec3& GetIdealFocusPoint() const { return idealFocusPoint; }
  const idVec3& GetCurrentFocusPoint() const { return currentFocusPoint; }
  void SetCurrentFocusPointForTest(const idVec3& value) {
    currentFocusPoint = value;
  }
  void SetIdealFocusPoint(const idVec3& value) {
    idealFocusPoint = value;
  }
  void SetIdealFocusPointForTest(const idVec3& value) {
    SetIdealFocusPoint(value);
  }
  idEntity* GetFocusEntity() const { return focusEntity; }
  int GetFocusTimeout() const { return focusTimeout; }
  bool GetKeepFocusInViewForTest() const { return keepFocusInView; }
  aimPoint_t GetAimPointSelection() const { return aimPoint; }
  aimPoint_t GetAimPointForTest() const { return GetAimPointSelection(); }
  aiFocus_t GetFocusType() const { return focusType; }
  aiFocus_t GetFocusTypeForTest() const { return GetFocusType(); }
  aiFoci_t GetFociType() const { return fociType; }
  const idVec3& GetOffset() const { return offset; }
  const idVec3& GetOffsetForTest() const { return GetOffset(); }
  bool HasExplicitFocusPoint() const { return hasExplicitFocusPoint; }
  bool HasExplicitFocusPointForTest() const {
    return HasExplicitFocusPoint();
  }
  void SetFocusType(aiFocus_t value) { focusType = value; }
  void SetOffset(const idVec3& value) { offset = value; }
  int GetCurrentAnimatorIndex() const { return curAnimatorIndex; }
  int GetTrackingParmsIndex() const { return trackingParmsIndex; }
  int GetUpdateCount() const { return updateCount; }
  idAnimator_TrackerBase* GetLastParentTrackerForTest() const {
    return lastParentTracker;
  }
  const idStr& GetDebugText() const { return debugText; }
  const idAimTracker& GetAimTracker() const { return aimTracker; }
  void SetBodyOrientationYawLimitForTest(float radians) {
    aimTracker.SetConstraints(radians, radians);
  }
  int GetKickCount() const { return wanderingAim.kickCount; }
  int GetInitCountForTest() const { return initCount; }
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
  idVec3 currentFocusPoint{};
  idVec3 offset{};
  int defaultHeadTrackGroup{-1};
  int curAnimatorIndex{1};
  int trackingParmsIndex{0};
  int focusTimeout{-1};
  int updateCount{0};
  idAnimator_TrackerBase* lastParentTracker{nullptr};
  int initCount{0};
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
  void ProcessEvent(const idEventDef* eventDefinition) {
    lastProcessedEvent = eventDefinition;
    lastProcessedEntity = nullptr;
    lastProcessEventArgumentCount = 0;
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
  idFiniteStateMachine* FindAlertCycleFSM() {
    return alertCycleAvailable ? &alertCycle : nullptr;
  }
  void SetAlertCycleFSMAvailableForTest(bool value) {
    alertCycleAvailable = value;
  }
  void Init(idAI2* value) {
    owner = value;
    ++initCount;
  }
  int NumFSMs() const { return 1; }
  idFiniteStateMachine* GetFSM(int index) {
    if (index == 0) return &alertCycle;
    return index == 1 ? &combatScenePoint : nullptr;
  }
  idAI2* GetOwnerForTest() const { return owner; }
  int GetInitCountForTest() const { return initCount; }
  void SetFSMStack(const idFSMStack& value) {
    stack = value;
    ++setStackCount;
  }
  const idFSMStack& GetFSMStackForTest() const { return stack; }
  int GetSetStackCountForTest() const { return setStackCount; }
  idFiniteStateMachine* GetCombatScenePointFSMForTest() {
    return &combatScenePoint;
  }

private:
  idFiniteStateMachine alertCycle{"alertCycle"};
  idFiniteStateMachine combatScenePoint{"combatScenePoint"};
  idAI2* owner{nullptr};
  int initCount{0};
  idFSMStack stack;
  int setStackCount{0};
  bool alertCycleAvailable{true};
};

class idFXManager {
public:
  void SetExternalTransform(const idVec3& position,
                            const idMat3& rotation) {
    externalPosition = position;
    externalRotation = rotation;
    hasExternalPositionAndRotation = true;
  }
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
  void StopAllFX(int gameTime, bool immediate) {
    lastStopAllTime = gameTime;
    lastStopAllImmediate = immediate;
    ++stopAllCount;
  }
  const idVec3& GetExternalPosition() const { return externalPosition; }
  const idMat3& GetExternalRotation() const { return externalRotation; }
  bool HasExternalPositionAndRotation() const {
    return hasExternalPositionAndRotation;
  }
  int GetStartCount() const { return startCount; }
  int GetStopCount() const { return stopCount; }
  int GetStopAllCount() const { return stopAllCount; }
  const idVec3& GetLastStartPosition() const { return lastStartPosition; }
  const idMat3& GetLastStartAxis() const { return lastStartAxis; }
  int GetLastStartTime() const { return lastStartTime; }
  int GetLastStartCondition() const { return lastStartCondition; }
  int GetLastStopTime() const { return lastStopTime; }
  int GetLastStopCondition() const { return lastStopCondition; }
  bool GetLastStopImmediate() const { return lastStopImmediate; }
  int GetLastStopAllTime() const { return lastStopAllTime; }
  bool GetLastStopAllImmediate() const { return lastStopAllImmediate; }
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
  int lastStopAllTime{-1};
  bool lastStopAllImmediate{false};
  int startCount{0};
  int stopCount{0};
  int stopAllCount{0};
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

struct eboltControlNode_t { int value{0}; };
class idCrosshairCustomIcon {
public:
  int value{0};
};
class idPonytail {
public:
  int value{0};
};
class idAnimWebAI;
class idAIAction {};
class idEntitySpriteAnimator {};
class idProxyMove : public idEntity {
public:
  const idVec3& GetProxyPlayerOrigin() const { return proxyPlayerOrigin; }
  void SetProxyPlayerOriginForTest(const idVec3& value) {
    proxyPlayerOrigin = value;
  }

private:
  idVec3 proxyPlayerOrigin{};
};
class idTarget_AlarmSystem : public idEntity {};
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
  T GetRatio(T current) const {
    const T range = maxRange - minRange;
    const T clampedCurrent = std::max(T{}, current - minRange);
    return range != T{} ? std::min(T{1}, clampedCurrent / range) : T{};
  }
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
