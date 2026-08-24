#pragma once

#include "tungsten/game/ai/aitypes.h"
#include "tungsten/game/game_runtime.h"
#include "tungsten/game/declvoiceover.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/containers/staticlist.h"
#include "shared/idlib/text/atomicstring.h"
#include "shared/idlib/text/str.h"

#include <array>
#include <algorithm>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

struct idScriptAction;
class idAIVoiceController;
class idAIOrientation;
class idFaceMgr;
class idPresentable;
class idAnimWebPath;
class idMD6Anim;
class idWeapon;
class idAIFireControl;
class idAIMoveState;
class idFocusInfo;
class idAIEvent;
class idLegsFSM;
class idDeclFacialAnimationSet;
class idVolume_CustomDeath;
struct idAIVoiceProfile;
struct idDeclAIBehavior;
class idInteractionCamera;
class idAIEntityState;

template <typename T>
class idEntityPtr {
public:
  idEntityPtr() = default;
  explicit idEntityPtr(T* value) : entity(value) {}

  void Set(T* value) { entity = value; }
  operator T*() const;
  T* GetEntity() const;
  T* operator->() const;

private:
  T* entity{nullptr};
};

class idDeclInventory {
public:
  const char* declTypeName{"inventoryItem"};
  bool resourceValid{true};
};

class idDeclAmmo {
public:
  explicit idDeclAmmo(const char* declName = nullptr)
      : name(declName != nullptr ? declName : "") {
    registry.push_back(this);
  }
  ~idDeclAmmo() {
    registry.erase(std::remove(registry.begin(), registry.end(), this),
                   registry.end());
  }

  static const idDeclAmmo* FindWithInheritance(const char* declName) {
    if (declName == nullptr) {
      return nullptr;
    }
    for (const idDeclAmmo* declaration : registry) {
      if (declaration->resourceValid &&
          std::strcmp(declaration->GetName(), declName) == 0) {
        return declaration;
      }
    }
    return nullptr;
  }
  const char* GetName() const { return name.data; }
  bool resourceValid{true};

private:
  idStr name;
  inline static std::vector<const idDeclAmmo*> registry;
};

class idInventoryCollection {};

struct eventVoid {
  bool accepted{false};
  bool completed{false};
};

struct eventInt {
  int value{0};
};

struct eventVector {
  idVec3 value{};
};

struct eventBool {
  bool value{false};
};

struct eventDecl {
  const void* value{nullptr};
};

struct eventEntity {
  idEntity* value{nullptr};
};

struct idRecoveredValue {
  std::intptr_t value{0};

  template <typename T>
  operator T() const {
    if constexpr (std::is_pointer_v<T>) {
      return reinterpret_cast<T>(value);
    } else {
      return static_cast<T>(value);
    }
  }
};

enum aiActionBool_t {
  AIACTION_FALSE = 0,
  AIACTION_TRUE = 1,
};

enum coverAction_t {
  COVERACTION_NONE = 0,
  COVERACTION_BLINDFIRE_OVER = 6,
  COVERACTION_BLINDFIRE_LEFT = 7,
  COVERACTION_BLINDFIRE_RIGHT = 8,
  COVERACTION_HIDE = 12,
};
enum painType_t { PAINTYPE_NONE = 0 };
enum aiDirection_t { AIDIRECTION_NONE = 0 };
enum aiAnimWait_t { AIANIMWAIT_DONT_WAIT = 0, AIANIMWAIT_WAIT = 1 };
enum walkState_t : int {
  WALKSTATE_DEFAULT = 0,
  WALKSTATE_WALKING = 1,
  WALKSTATE_RUNNING = 2,
};
enum aiFoci_t { AIFOCI_AIM, AIFOCI_LOOK, AIFOCI_NONE };
enum aiFocus_t { AIFOCUS_NOTHING, AIFOCUS_POINT, AIFOCUS_ENTITY };
enum runIndexType_t {
  ANIMWEBAI_RUNINDEXTYPE_NORMAL = 0,
  ANIMWEBAI_RUNINDEXTYPE_INJURED = 1,
  ANIMWEBAI_RUNINDEXTYPE_SCRAMBLE = 2,
  ANIMWEBAI_RUNINDEXTYPE_FAR = 3,
  ANIMWEBAI_RUNINDEXTYPE_GUNDOWN = 4,
  ANIMWEBAI_RUNINDEXTYPE_NARROW = 5,
  ANIMWEBAI_RUNINDEXTYPE_UP_STAIRS = 6,
  ANIMWEBAI_RUNINDEXTYPE_DOWN_STAIRS = 7,
  ANIMWEBAI_RUNINDEXTYPE_DUCK = 8,
  ANIMWEBAI_RUNINDEXTYPE_MAX = 9,
};
struct idAnimWebEvent {
  enum priority_t { PRIORITY_LOW = 0, PRIORITY_NORMAL = 1, PRIORITY_HIGH = 2 };
};
using aiModeRestricFlag_t = int;

bool OkToFocus(const idVec3* to, const idVec3* from,
               const idVec3* actualPosition);

struct idAIScriptEventArg {
  enum type_t { ENTITY, INTEGER, FLOAT, VECTOR, STRING, DECL };

  static idAIScriptEventArg Entity(const idEntity* value) {
    idAIScriptEventArg arg; arg.type = ENTITY; arg.entity = value; return arg;
  }
  static idAIScriptEventArg Integer(int value) {
    idAIScriptEventArg arg; arg.type = INTEGER; arg.integer = value; return arg;
  }
  static idAIScriptEventArg Float(float value) {
    idAIScriptEventArg arg; arg.type = FLOAT; arg.real = value; return arg;
  }
  static idAIScriptEventArg Vector(const idVec3& value) {
    idAIScriptEventArg arg; arg.type = VECTOR; arg.vector = value; return arg;
  }
  static idAIScriptEventArg String(const char* value) {
    idAIScriptEventArg arg; arg.type = STRING;
    arg.string = value != nullptr ? value : ""; return arg;
  }
  static idAIScriptEventArg Decl(const void* value) {
    idAIScriptEventArg arg; arg.type = DECL; arg.decl = value; return arg;
  }

  type_t type{INTEGER};
  const idEntity* entity{nullptr};
  const void* decl{nullptr};
  idVec3 vector{};
  std::string string;
  int integer{0};
  float real{0.0F};
};

class idAIScriptProxy {
public:
  struct invocation_t {
    std::string event;
    std::vector<idAIScriptEventArg> arguments;
  };

  void ProcessEvent(const char* event,
                    std::initializer_list<idAIScriptEventArg> arguments) {
    lastInvocation.event = event != nullptr ? event : "";
    lastInvocation.arguments.assign(arguments.begin(), arguments.end());
  }
  const invocation_t& GetLastInvocation() const { return lastInvocation; }
  idRecoveredValue ForceFSMIntoExecutingScript(...);
  idRecoveredValue InternalCallEvent(...);
  idRecoveredValue PauseCurrentFiberTillActionCompletes(...);
  idRecoveredValue PauseCurrentThreadTillActionCompletes(...);
  idRecoveredValue PauseFiberTillActionCompletes(...);
  idRecoveredValue PauseThreadTillActionCompletes(...);
  bool blocking{false};

private:
  invocation_t lastInvocation;
};

template <int Index>
struct idRecoveredOverloadTag {};

struct idFirstPersonEvent {
  int eventId{0};
};

class idActor : public idEntity {
public:
  eventVoid* AnimEvent_DropWeapons(eventVoid* result);
  bool WereWeaponsDropped() const { return weaponsDropped; }
  walkState_t GetWalkState() const;
  void SetWalkState(walkState_t state);
  idRecoveredValue SaveDamageGroup(...);
  idRecoveredValue SetLastDamageInfo(...);
  idRecoveredValue GetAF(...);
  idRecoveredValue GetLastEyePos(...);
  idRecoveredValue GetEyeAxis(...);
  idRecoveredValue SetEyeAxis(...);
  idRecoveredValue GetInventory(...);
  idRecoveredValue GetAttachments(...);
  idRecoveredValue GetTriggerTouchList(...);
  idRecoveredValue GetDisplayName(...);
  idRecoveredValue GetPosture(...);
  idRecoveredValue GetLeanState(...);
  idRecoveredValue SetLeanState(...);
  idRecoveredValue GetWeaponReadyState(...);
  idRecoveredValue SetWeaponReadyState(...);
  idWeapon* GetEquippedWeapon(unsigned int slot) const {
    return slot < equippedWeapons.size() ? equippedWeapons[slot] : nullptr;
  }
  void SetEquippedWeapon(unsigned int slot, idWeapon* weapon) {
    if (slot < equippedWeapons.size()) {
      equippedWeapons[slot] = weapon;
    }
  }
  idInventoryCollection* GetInventoryCollection() { return &inventory; }

protected:
  bool weaponsDropped{false};
  walkState_t walkState{WALKSTATE_DEFAULT};
  std::array<idWeapon*, 15> equippedWeapons{};
  idInventoryCollection inventory;
};

class idPlayer : public idActor {
public:
  idFirstPersonEvent* GetCurFirstPersonEvent();
  void SetCurFirstPersonEvent(idFirstPersonEvent* value) {
    currentFirstPersonEvent = value;
  }
  idRecoveredValue GetUCmdTracker1(...);
  idRecoveredValue GetIsSprinting(...);
  idRecoveredValue GetFocusTracker(...);
  idRecoveredValue IsZoomedIn(...);
  idRecoveredValue GetProxyMove(...);
  void SetNoclip(bool value);
  bool IsNoclip() const { return noclip; }
  void SetAIAimState(const idVec3& originValue, const idVec3& directionValue,
                     float distanceValue) {
    aiAimOrigin = originValue;
    aiAimDirection = directionValue;
    aiAimDistance = std::max(0.0F, distanceValue);
  }
  const idVec3& GetAIAimOrigin() const { return aiAimOrigin; }
  const idVec3& GetAIAimDirection() const { return aiAimDirection; }
  float GetAIAimDistance() const { return aiAimDistance; }
  bool CanGrapple() const;
  void SetCanGrapple(bool value) { canGrapple = value; }
  float GetTotalCurHealth() const { return totalCurHealth; }
  void SetTotalCurHealth(float value) { totalCurHealth = value; }
  bool IsSpectating() const { return spectating; }
  void SetSpectating(bool value) { spectating = value; }

private:
  idFirstPersonEvent* currentFirstPersonEvent{nullptr};
  bool noclip{false};
  idVec3 aiAimOrigin{};
  bool canGrapple{true};
  idVec3 aiAimDirection{1.0F, 0.0F, 0.0F};
  float aiAimDistance{4096.0F};
  float totalCurHealth{100.0F};
  bool spectating{false};
};

class idTreeAnimator {
public:
  const char* GetModelDefName();
  void SetModelDefName(const char* value) { modelDefName = value; }

private:
  idStr modelDefName;
};

class idFaction {
public:
  const char* GetName();
  void SetName(const char* value) { name = value; }

private:
  idStr name;
};

class idAI2;

enum aiGroupMessage_t {
  AIGROUPMSG_NONE,
  AIGROUPMSG_ALERT,
  AIGROUPMSG_SEARCH,
  AIGROUPMSG_COMBAT,
};

enum encounterGroupRole_t {
  ROLE_NONE,
  ROLE_DEFEND,
  ROLE_ASSAULT,
  ROLE_FLANK,
  ROLE_SUPPRESS,
  ROLE_RECHARGE,
};

class idAICombatHint : public idEntity {
public:
  enum combatHintType_t {
    COMBAT_HINT_DEFENSE_POINT,
    COMBAT_HINT_ASSAULT_POINT,
    COMBAT_HINT_FLANK_POINT,
    COMBAT_HINT_FLANK_TARGET_POINT,
    COMBAT_HINT_FLANK_PATH_POINT,
    COMBAT_HINT_GRENADE_POINT,
    COMBAT_HINT_RECHARGE_POINT,
    COMBAT_HINT_SUPPRESSION_FIRE_POINT,
    COMBAT_HINT_MAX,
  };

  enum combatHintClass_t {
    COMBAT_HINT_CLASS_NONE = 0,
    COMBAT_HINT_CLASS_LIGHT = 1,
    COMBAT_HINT_CLASS_HEAVY = 2,
    COMBAT_HINT_CLASS_ANY = COMBAT_HINT_CLASS_LIGHT | COMBAT_HINT_CLASS_HEAVY,
  };

  combatHintType_t type{COMBAT_HINT_DEFENSE_POINT};
  int hintClass{COMBAT_HINT_CLASS_ANY};
  int coverIndex{-1};
  int dontUseUntil{0};
  bool enabled{true};
  idVec3 origin{};
  idVec3 direction{1.0F, 0.0F, 0.0F};
};

class idCombatStage {
public:
  enum combatStage_t {
    COMBAT_STAGE_NO_GOODWILL,
    COMBAT_STAGE_DEFAULT,
    COMBAT_STAGE_ADVANCE,
    COMBAT_STAGE_FALLBACK,
    COMBAT_STAGE_RETREAT,
    COMBAT_STAGE_MAX,
  };

  const idList<int, 5>* GetNodesListByHintType(
      idAICombatHint::combatHintType_t hintType) const;
  const idList<int, 5>* GetNumHintNodes(
      idAICombatHint::combatHintType_t hintType) const;
  const idAICombatHint* GetCombatHintEntity(int index) const;
  const idAICombatHint* GetClosestHintNodePtr(
      idAICombatHint::combatHintType_t hintType, const idVec3& currentPos,
      int hintClass = idAICombatHint::COMBAT_HINT_CLASS_ANY) const;
  const idAICombatHint* GetClosestHintNodePtrByDot(
      idAICombatHint::combatHintType_t hintType, const idVec3& currentPos,
      const idVec3& direction,
      int hintClass = idAICombatHint::COMBAT_HINT_CLASS_ANY) const;
  void CategorizeHintGroupCombatNodes();

  idStr name;
  idList<idAICombatHint*, 5> hints;
  idList<int, 5> defenseNodes;
  idList<int, 5> assaultNodes;
  idList<int, 5> flankNodes;
  idList<int, 5> flankTargetNodes;
  idList<int, 5> flankPathNodes;
  idList<int, 5> grenadeNodes;
  idList<int, 5> rechargeNodes;
  idList<int, 5> suppressionFireNodes;
  idList<encounterGroupRole_t, 5> rolePriorityList;
};

class idEncounterGroup;

class idEncounterGroupExpr {
public:
  enum value_t {
    VALUE_LIVING_MEMBERS,
    VALUE_DEAD_MEMBERS,
    VALUE_PERCENT_ALIVE,
    VALUE_AVERAGE_HEALTH,
    VALUE_LOWEST_TARGET_HEALTH,
    VALUE_HIGHEST_ALERT_CYCLE,
  };
  enum comparison_t { COMPARE_LESS, COMPARE_LESS_EQUAL, COMPARE_EQUAL,
                      COMPARE_GREATER_EQUAL, COMPARE_GREATER };

  float Value(idEncounterGroup* group) const;
  void TriggerTarget(idEncounterGroup* group);
  bool Eval(idEncounterGroup* owner) const;
  void CheckTrigger(idEncounterGroup* group);

  value_t value{VALUE_LIVING_MEMBERS};
  comparison_t comparison{COMPARE_GREATER_EQUAL};
  float threshold{0.0F};
  bool repeat{false};
  bool triggered{false};
  idEntity* target{nullptr};
  std::function<void(idEncounterGroup&)> callback;
};

struct idEncounterState {
  idEncounterState();

  int alertCycle{0};
  idEntity* target{nullptr};
  idVec3 lastKnownPosition{};
  int lastSeenTime{-1};
};

struct idTargetList {
  struct entry_t {
    idEntity* entity{nullptr};
    idPlayer* player{nullptr};
    float health{100.0F};
    float maxHealth{100.0F};
  };
  idList<entry_t, 5> viableTargets;
};

class idEncounterGroup {
public:
  struct member_t {
    idAI2* ai{nullptr};
    float maxHealth{100.0F};
    float enemyHealthRatio{1.0F};
    int alertCycle{0};
    int lastSeenEnemyTime{-1};
    encounterGroupRole_t role{ROLE_NONE};
    bool active{true};
    bool firing{false};
  };

  struct target_t {
    idEntity* target{nullptr};
    idPlayer* player{nullptr};
    float health{100.0F};
    float maxHealth{100.0F};
    int frame{0};
  };

  idEncounterGroup();
  ~idEncounterGroup();

  void ResetSearchPoints();
  int GetResetCount() const { return resetCount; }
  idCombatStage* GetCurrentCombatStage();
  const idCombatStage* GetCurrentCombatStage() const;

  void SetAnyMemberHasSeenDeadBody(bool seen);
  bool GetAnyMemberHasSeenDeadBody() const;
  bool CanAddFirer() const;
  void AddFirer();
  void RemoveFirer();
  int NumLivingMembers() const;
  int NumDeadMembers() const;
  int GetNumTargets() const;
  float LowestPlayerHealth() const;
  float HighestPlayerHealth() const;
  int AnyMemberHighestAlertCycle() const;
  void SetGroupEnemyUpdateTimeStamp(const idEntity* enemy, int time);
  bool IsRecentlyGroupSpotted(const idEntity* enemy, float seconds) const;
  const idPlayer* GetPlayerWithLowestHealth() const;
  const idPlayer* GetPlayerWithHighestHealth() const;
  int FindIndex(idAI2* ai) const;
  void Remove(idAI2* ai);
  float PercentMembersAlive() const;
  float AverageHealth() const;
  float AverageEnemyHealthRatio() const;
  float LowestTargetPlayerHealth() const;
  float HighestTargetPlayerHealth() const;
  int ActiveMemberHighestAlertCycle() const;
  idAI2* GetRandomMember();
  idAI2* GetRandomDeadMember();
  const idEntity* GetRandomEnemy();
  const idPlayer* GetTargetedPlayerWithLowestHealth() const;
  const idPlayer* GetTargetedPlayerWithHighestHealth() const;
  bool SetPendingCombatStage(int index, int delay);
  void UpdateAnyMemberHighestAlertCycle();
  idAI2* GetLeader() const;
  void MemberKilled(idAI2* ai, const idEntity* attacker,
                    const idEntity* inflictor);
  void UpdateGroupAmbush(idAI2* member, const idVec3& enemyPos);
  idAI2* FindSpeaker(const idEntity* enemy, bool mustBeNearLastVisPos,
                     bool mustBeVisible, const idAI2* excludeAI) const;
  bool SetPendingCombatStageByName(const idStr& name, int delay);
  bool SuggestEncounterGroupRole(
      idAI2* ai, encounterGroupRole_t role,
      idCombatStage::combatStage_t combatStageChange,
      bool previouslyPlayedOrderVo);
  void DebugDrawCombatStage(const idCombatStage* stage, int duration);
  void OnGroupMessage(aiGroupMessage_t message, idEntity* entity);
  void GetCurrentCombatStageCoverIndices(idList<int, 5>* output) const;
  void GetLivingMembers(idStaticList<idAI2*, 64>* output) const;
  idAI2* GetRandomLiveMember();
  idAI2* GetClosestActiveGroupMemberToPoint(const idVec3& point) const;
  void ResolveChildReferences();
  int LastLivingMemberSeenEnemytime() const;
  void UpdateFiringConditions();
  bool UpdateGroupMemberRoles(idCombatStage::combatStage_t combatStageChange);
  void UpdateLivingMemberCount();
  bool SetCurrentCombatStage(int index);
  void Init(const char* nameValue, std::int16_t handleValue);
  void Add(idAI2* ai);
  void CheckGroupTriggers();
  void Think();
  void UpdateTargetList(const idTargetList* targets);

  void SetLivingMembers(int value) {
    livingMembers = value;
    numLivingMembers = value;
  }
  void SetActiveCoverIndices(const idList<int, 5>& value) {
    activeCoverIndices = value;
  }
  const idStr& GetGroupName() const { return groupName; }
  std::int16_t GetHandle() const { return handle; }
  int GetThinkCount() const { return thinkCount; }
  int GetLastMessage() const { return lastMessage; }
  idEntity* GetLastMessageEntity() const { return lastMessageEntity; }
  int GetCurrentCombatStageIndex() const { return curCombatStage; }
  int GetPendingCombatStageIndex() const { return pendingCombatStage; }
  int GetRecentlyKilledMembers() const { return recentlyKilledMembers; }

  idStr groupName;
  std::int16_t handle{-1};
  bool initialized{false};
  int livingMembers{0};
  int thinkCount{0};
  int lastMessage{-1};
  int messageCount{0};
  idEntity* lastMessageEntity{nullptr};
  idList<int, 5> activeCoverIndices;
  int resetCount{0};
  idList<member_t, 5> members;
  idList<idEncounterGroup*, 5> childGroups;
  idList<idStr, 5> unresolvedChildNames;
  idList<target_t, 5> targetList;
  idList<idCombatStage, 5> combatStages;
  idList<idEncounterGroupExpr, 5> expressions;
  idEncounterState ownGroupAlertState;
  idEncounterState otherGroupAlertState;
  int numDeadMembers{0};
  int numLivingMembers{0};
  int maxSimultaneousFirers{-1};
  int numSimultaneousFiring{0};
  int anyMemberHighestAlertCycle{0};
  int groupEnemyUpdateTimeStamp{-1};
  const idEntity* groupEnemy{nullptr};
  int curCombatStage{-1};
  int pendingCombatStage{-1};
  int pendingCombatStageTime{-1};
  int initialCombatStage{0};
  int recentlyKilledMembers{0};
  int randomSeed{0x12345};
  bool anyMemberHasSeenDeadBody{false};
  bool shouldAmbush{false};
};

struct idAIProjectileInfo {
  const void* itemDecl{nullptr};
  idVec3 actualTargetPos{};
  idVec3 actualTargetVel{};
  idVec3 predictedTargetPos{};
  float linearSpeed{0.0F};
  int launchTime{0};
};

class idAIMemory;

struct idThreadHandle {
  int value{0};
};

struct idFiberHandle {
  int value{0};
};

struct idEventDef {
  int eventnum{0};
};

class idThread {
public:
  explicit idThread(int handleValue = 0);
  ~idThread();

  int GetHandle() const { return handle.value; }
  bool IsDying() const { return dying; }
  void SetDying(bool value) { dying = value; }
  void Execute() { ++executeCount; }
  int GetExecuteCount() const { return executeCount; }
  static idThread* Find(int handleValue);

private:
  idThreadHandle handle;
  bool dying{false};
  int executeCount{0};
};

struct idAIScriptState {
  idList<idThreadHandle, 5> threads;
  idList<idFiberHandle, 5> fibers;
  int waitThreadHandle{0};
  bool forcedCodeAction{false};
};

struct idAIAnimNodePlayback {
  int animationId{0};
  float timeScale{1.0F};
};

struct idCachedJoint {
  idVec3 translation{};
  idMat3 axis{};
  int frameNumber{0};
  int jointIndex{-1};
};

struct idMilliseconds {
  int value{0};
};

struct idParabolicParms {
  float speed{0.0F};
  float angle{0.0F};
};

struct idDeclProjectile {
  bool spawnable{true};
};

struct idAIPos {
  idVec3 pos{};
  bool valid{false};
};

struct idAmmoItem {
  idStr animationPrefix;
};

struct idDeclDamage {
  int index{0};
  float radius{0.0F};
};

struct idDeclProjectileImpactEffect {
  int index{0};
};

class idAI2;

class idWeapon : public idEntity {
public:
  enum triggerState_t { TRIGGERSTATE_RELEASED, TRIGGERSTATE_PULLED };
  enum fireState_t { FIRESTATE_IDLE, FIRESTATE_FIRING, FIRESTATE_RELOADING };
  enum weaponState_t { WEAPONSTATE_NORMAL, WEAPONSTATE_SECONDARY };

  const idDeclDamage* GetCurrentDamageDecl();
  idAmmoItem* GetLoadedAmmo(bool secondaryAmmo);
  void SetCurrentDamageDecl(const idDeclDamage* value) { currentDamageDecl = value; }
  void SetAnimationPrefix(const char* value) { animationPrefix = value; }
  const idStr& GetAnimationPrefix() const { return animationPrefix; }
  void SetLoadedAmmo(idAmmoItem* primary, idAmmoItem* secondary = nullptr) {
    ammoClip = primary;
    ammoClipSecondary = secondary;
  }
  void SetMuzzleTransform(const idVec3& position, const idMat3& axis) {
    muzzlePosition = position;
    muzzleAxis = axis;
    hasMuzzleTransform = true;
  }
  void SetLaserTransform(const idVec3& position, const idMat3& axis) {
    laserPosition = position;
    laserAxis = axis;
    hasLaserTransform = true;
  }
  bool QueryMuzzleTransform(idVec3* position, idMat3* axis) const;
  bool QueryLaserTransform(idVec3* position, idMat3* axis) const;
  bool UsesFireControl() const { return usesFireControl; }
  void SetUsesFireControl(bool value) { usesFireControl = value; }
  bool NeedsReload(bool secondary) const {
    return secondary ? secondaryNeedsReload : primaryNeedsReload;
  }
  void SetNeedsReload(bool primary, bool secondary = false) {
    primaryNeedsReload = primary;
    secondaryNeedsReload = secondary;
  }
  bool CanReload(idInventoryCollection* inventory,
                 const idDeclAmmo* ammo, bool force) const {
    (void)inventory;
    (void)force;
    return reloadAllowed && ammo != nullptr;
  }
  void SetReloadAllowed(bool value) { reloadAllowed = value; }
  void PullTrigger(idAI2* owner, bool secondary) {
    (void)owner;
    triggerState = TRIGGERSTATE_PULLED;
    weaponState = secondary ? WEAPONSTATE_SECONDARY : WEAPONSTATE_NORMAL;
    ++pullCount;
  }
  void ReleaseTrigger(idAI2* owner, bool secondary) {
    (void)owner;
    (void)secondary;
    triggerState = TRIGGERSTATE_RELEASED;
    ++releaseCount;
  }
  void BeginFiring(bool deferred) {
    (void)deferred;
    fireState = FIRESTATE_FIRING;
    ++burstCount;
    ++shotsFired;
  }
  void SetFireState(fireState_t value) { fireState = value; }
  void SetWeaponMode(bool secondary) {
    weaponState = secondary ? WEAPONSTATE_SECONDARY : WEAPONSTATE_NORMAL;
  }
  void StopAllFiringEffects() { firingEffectsStopped = true; }
  void SetEquipSlot(int value) { equipSlot = value; }
  int GetEquipSlot() const { return equipSlot; }
  void SetFireIntervals(int burstMinimum, int burstMaximum,
                        int repullMinimum, int repullMaximum,
                        int lastKnownMinimum, int lastKnownMaximum) {
    burstMin = burstMinimum;
    burstMax = burstMaximum;
    repullMin = repullMinimum;
    repullMax = repullMaximum;
    lastKnownMin = lastKnownMinimum;
    lastKnownMax = lastKnownMaximum;
  }
  void SetWanderingAim(float kickback, float wander, float maximum,
                       float convergence, int intervalMs) {
    aimKickback = std::max(0.0F, kickback);
    aimWanderScale = std::max(0.0F, wander);
    aimMaxDivergence = std::max(0.0F, maximum);
    aimConvergeRate = std::max(0.0F, convergence);
    aimDivergeInterval = std::max(0, intervalMs);
  }
  float GetAimKickback() const { return aimKickback; }
  float GetAimWanderScale() const { return aimWanderScale; }
  float GetAimMaxDivergence() const { return aimMaxDivergence; }
  float GetAimConvergeRate() const { return aimConvergeRate; }
  int GetAimDivergeInterval() const { return aimDivergeInterval; }
  void SetHolsterSlot(unsigned int value) { holsterSlot = value; }
  unsigned int GetHolsterSlot() const { return holsterSlot; }

  triggerState_t triggerState{TRIGGERSTATE_RELEASED};
  fireState_t fireState{FIRESTATE_IDLE};
  weaponState_t weaponState{WEAPONSTATE_NORMAL};
  int burstCount{0};
  int shotsFired{0};
  int pullCount{0};
  int releaseCount{0};
  int burstMin{100};
  int burstMax{100};
  int repullMin{100};
  int repullMax{100};
  int lastKnownMin{250};
  int lastKnownMax{250};
  bool firingEffectsStopped{false};

private:
  const idDeclDamage* currentDamageDecl{nullptr};
  idStr animationPrefix{"hands_"};
  idAmmoItem* ammoClip{nullptr};
  idAmmoItem* ammoClipSecondary{nullptr};
  idVec3 muzzlePosition{};
  idMat3 muzzleAxis{};
  idVec3 laserPosition{};
  idMat3 laserAxis{};
  bool hasMuzzleTransform{false};
  bool hasLaserTransform{false};
  bool usesFireControl{true};
  bool primaryNeedsReload{false};
  bool secondaryNeedsReload{false};
  bool reloadAllowed{true};
  int equipSlot{0};
  float aimKickback{0.02F};
  float aimWanderScale{0.005F};
  float aimMaxDivergence{0.08F};
  float aimConvergeRate{0.01F};
  int aimDivergeInterval{250};
  unsigned int holsterSlot{0};
};

enum class aiSubWeb_t : int {
  relaxed = 0,
  alert = 1,
  combat = 2,
  traversal = 3,
};

struct idAnimWebIndex {
  std::int16_t value{-1};
};

struct idAIAnimWebRuntimeState {
  idStr movementState{"walk"};
  idStr idleState{"idle_stand"};
  aiSubWeb_t subWeb{aiSubWeb_t::relaxed};
  idAnimWebIndex currentSubWeb{0};
  idAnimWebIndex currentState{0};
  int lastPathResult{0};
  std::array<idAIAnimNodePlayback, 8> nodeAnimations{};
  const class idDeclAnimWeb* declaration{nullptr};
  idAnimWebIndex currentNode{-1};
  std::vector<std::string> validScriptedPaths;
};

struct idDamageGroup {
  idStr groupName;
  float damageScale{1.0F};
  float maxHealth{0.0F};
  float health{0.0F};
  float HealthRatio() const;
};

struct idAIDebugState {
  idStr debugText;
  idStr damageText;
  int textLevel{0};
  int errorFlags{0};
};

struct idAIEventState {
  idStr lastEvent;
  int sequence{0};
  int alertCycle{0};
  int callback{0};
  int currentEventClass{0};
  int lastEventClass{0};
  idVec3 lastEventOrigin{};
  idVec3 bodyOrientation{1.0F, 0.0F, 0.0F};
  idEntity* lastEventOriginator{nullptr};
  idEntity* lastEventInstigator{nullptr};
  idEntity* path{nullptr};
  idEntity* overrideScenePoint{nullptr};
  idEntity* takedownRestrictionVolume{nullptr};
  idEntity* scenePointUserGroup{nullptr};
  idEntity* avoidEntity{nullptr};
  idEntity* actionNode{nullptr};
  const idDeclVoiceOver* playingVoiceOver{nullptr};
  float globalAggro{0.0F};
  float closeMeleeInterval{0.0F};
  float mediumMeleeInterval{0.0F};
  float longMeleeInterval{0.0F};
  float teslaCooldown{0.0F};
  float fieldOfView{90.0F};
  float empStunDuration{0.0F};
  float animWebScriptBlend{0.0F};
  int overloadEnableFlags{0};
  int overloadDisableFlags{0};
  bool teslaActive{false};
  bool laserSight{false};
  bool removeHealthWhenDamaged{true};
  bool tauntsEnabled{true};
  bool controlRelinquished{false};
  bool effectsStopped{false};
  bool lootBoxSpawned{false};
  bool bodyRemoved{false};
  idList<idVec3, 5> teslaBlindFirePositions;
};

struct idAIUtilityState {
  std::array<bool, 4> trajectoryHasResults{{false, false, false, false}};
  idAIProjectileInfo selectedProjectile;
  idAIProjectileInfo lastLaunch;
  std::array<idCachedJoint, 8> cachedJoints{};
  int cachedJointCount{0};
};

struct idAICoreState {
  idStr lastCall;
  int sequence{0};
  int modifiedSkill{0};
  std::uint64_t perceptionFlags{0};
  int aiEventResponseFlags{0};
  coverAction_t coverAction{COVERACTION_NONE};
  int coverActionTimestamp{0};
  bool sleeping{false};
  bool blinded{false};
  bool worldCollision{true};
  bool extendedClipModel{false};
  bool aiClip{true};
};

struct idAIRobotRuntimeState {
  bool usesRobot{false};
  bool isCharging{false};
  bool autoRecharge{false};
  bool useRechargeAnimation{false};
  bool usesTesla{false};
  bool usesLookScenePoints{true};
  bool cooldownEffectActive{false};
  bool chargingAnimationActive{false};
  bool powerOutTriggerValid{false};
  bool chargingDoneTriggerValid{false};
  int noFireTime{-1};
  int lastShootTime{-1};
  int chargeStartTime{-1};
  int chargeDoneTime{-1};
  int numberOfStages{1};
  int currentStage{0};
  int powerOutTriggerActivations{0};
  int chargingDoneTriggerActivations{0};
  int cooldownEffectStartTime{-1};
  int cooldownEffectStopTime{-1};
  float shootHealthDrain{0.0F};
  float stageHealth{0.0F};
  float currentStageHealth{0.0F};
  float chargeHealthFraction{0.0F};
  float bossHealth{0.0F};
};

class idProp_Moveable : public idEntity {
public:
  idRecoveredValue SetOwner(...);
};

enum scenePointType_t {
  SCENEPOINT_INVALID = 0,
  ROAM = 1,
  LOOK = 2,
  SEARCH = 3,
  COMBAT = 4,
  SPAWN = 5,
};

class idScenePoint : public idEntity {
public:
  void Disable(int currentTime, float disableSeconds);
  void ActivateScenePoint(idAI2* user, int time) {
    lastUser = user;
    lastActivationTime = time;
    ++activationCount;
  }
  void ReleaseScenePoint(idAI2* user, int time) {
    lastUser = user;
    lastReleaseTime = time;
    ++releaseCount;
  }
  void SetScenePointType(scenePointType_t value) { type = value; }
  scenePointType_t GetScenePointType() const { return type; }
  void SetScriptFlags(unsigned int value) { scriptFlags = value; }
  unsigned int GetScriptFlags() const { return scriptFlags; }
  void SetEnabled(bool value) { enabled = value; }
  bool IsEnabled() const { return enabled; }
  int GetActivationCount() const { return activationCount; }
  int GetReleaseCount() const { return releaseCount; }
  int GetRequestTime() const { return requestTime; }
  void SetCurrentUser(idAI2* value) { currentUser = value; }
  idAI2* GetCurrentUser() const { return currentUser; }

  scenePointType_t type{SCENEPOINT_INVALID};
  unsigned int scriptFlags{0};

private:
  idAI2* lastUser{nullptr};
  int lastActivationTime{-1};
  int lastReleaseTime{-1};
  int activationCount{0};
  int releaseCount{0};
  bool enabled{true};
  idAI2* currentUser{nullptr};
  int requestTime{0};
};

class idScenePoint_Combat : public idScenePoint {
public:
  bool IsTactical() const { return tactical; }
  void SetTactical(bool value) { tactical = value; }
  idProp_Moveable* GetDualPickup();
  bool HasDualPickup();
  void SetDualPickup(idProp_Moveable* value) { dualWieldPickup.Set(value); }
  bool HasEnterAnimation() const;
  void SetEnterAnimation(const char* value) { enterAnimation = value; }
  void SetExtraEnterAnimationCount(int value) {
    extraEnterAnimationCount = value;
  }
  idEntity* GetTurret() const;
  void SetTurret(idEntity* value) { turret = value; }
  void ClearEnterAnimations();
  int GetClearEnterAnimationsCount() const { return clearEnterAnimationsCount; }

private:
  bool tactical{false};
  idEntityPtr<idProp_Moveable> dualWieldPickup;
  idEntity* turret{nullptr};
  int clearEnterAnimationsCount{0};
  idStr enterAnimation;
  int extraEnterAnimationCount{0};
};

enum aiMoveReason_t { AIMOVEREASON_NONE, AIMOVEREASON_SCRIPT, AIMOVEREASON_COMBAT };
enum aiArrivalAction_t { AIARRIVAL_NONE, AIARRIVAL_STOP, AIARRIVAL_FACE };
enum posture_t { POSTURE_STAND = 1, POSTURE_CROUCH = 2 };

class idAIMoveParms {
public:
  idAIMoveParms();
  const idVec3& GetRelevantGoalOrigin(const idAI2* ai) const;
  idVec3 GetRelevantGoalDirection(const idAI2* ai) const;
  void Init(aiMoveReason_t reason, const idVec3& start,
            const idVec3& goal, const idEntity* goalEntity,
            const idQuat& orientation, const idScenePoint* goalScenePoint,
            float goalTolerance, float intermediateTolerance,
            aiArrivalAction_t arrivalAction, posture_t arrivalPosture,
            int flags, idAnimWebIndex sourceSubWeb,
            idAnimWebIndex sourceState, idAnimWebIndex destinationSubWeb,
            idAnimWebIndex destinationState);
  const idScenePoint* GetGoalScenePoint();
  void SetGoalScenePoint(idScenePoint* value) { goalScenePoint = value; }
  const idEntity* GetGoalEntity() const;
  bool MovingToEntity() const;
  aiMoveReason_t GetMoveReason() const { return moveReason; }
  float GetGoalTolerance() const { return goalTolerance; }
  int GetFlags() const { return flags; }

private:
  aiMoveReason_t moveReason{AIMOVEREASON_NONE};
  idVec3 startOrigin{};
  idVec3 goalOrigin{};
  const idEntity* goalEntity{nullptr};
  idQuat goalOrientation{};
  idScenePoint* goalScenePoint{nullptr};
  float goalTolerance{0.1F};
  float intermediateTolerance{0.1F};
  aiArrivalAction_t arrivalAction{AIARRIVAL_NONE};
  posture_t arrivalPosture{POSTURE_STAND};
  int flags{0};
  idAnimWebIndex sourceSubWeb{-1};
  idAnimWebIndex sourceState{-1};
  idAnimWebIndex destinationSubWeb{-1};
  idAnimWebIndex destinationState{-1};
};

struct idAIAnimEventState {
  std::string lastEvent;
  int sequence{0};
  int meleeTraceBounds{0};
  int health{100};
  float fade{1.0F};
  bool autoFocus{false};
  bool fakeGrenadeVisible{false};
  bool headTracking{true};
  bool eyeTracking{true};
  bool worldCollision{true};
  bool solidCollision{true};
  bool damageEnabled{true};
  bool extendedClipModel{false};
  bool ikEnabled{true};
  bool blinking{true};
  bool lidDeformation{true};
  bool staggeringPain{true};
  bool deathAnimations{true};
  bool canRagdoll{true};
  bool ragdollActive{false};
  bool meleeTraceActive{false};
  bool jetPackFxActive{false};
  bool weaponFxActive{false};
  bool autoLipsync{false};
  bool dead{false};
  bool allowLooting{false};
};

class idAI2 : public idActor {
public:
  using idActor::SetEquippedWeapon;

  struct idAIVolatile {
    struct idAIWorldStateInfo { ~idAIWorldStateInfo(); };
    struct idPlayerAIControl { ~idPlayerAIControl(); };
    struct idAIDebugging { ~idAIDebugging(); };
    struct idAIDeath;
    struct idAIAnimationInfo {
      idAIAnimationInfo();
      ~idAIAnimationInfo();
    };
    struct idAIScriptInfo {
      idAIScriptInfo();
      ~idAIScriptInfo();
    };
    struct idAIPhysics { ~idAIPhysics(); };
    struct idAIPain { ~idAIPain(); };
    struct idAIFSMInfo { ~idAIFSMInfo(); };
    struct idAIFocusInfo { idAIFocusInfo(); };

    idAIVolatile();
    ~idAIVolatile();

    struct idAIRenderParamBlend {
      int parameter{0};
      float currentValue{0.0F};
      float targetValue{0.0F};
      float duration{0.0F};

      bool operator==(const idAIRenderParamBlend& other) const {
        return parameter == other.parameter;
      }
    };
  };

  struct idAIConstant {
    idAIConstant();
    ~idAIConstant();
  };

  struct idAnimWebDefaults {
    idAnimWebDefaults();
    ~idAnimWebDefaults();

    const idStr& GetWebPath() const { return webPath; }
    const idList<idAtomicString, 5>& GetActiveLayers() const {
      return activeLayers;
    }
    bool IsEnabled() const { return enabled; }

  private:
    idStr webPath;
    idList<idAtomicString, 5> activeLayers;
    bool enabled{false};
  };

  struct idVisibilityObscurity {
    idRecoveredValue GetDestination(...);
    idRecoveredValue Update(...);
  };

  idAI2();
  ~idAI2() override;
  bool CanSendGrappleTest() const;
  void SetGrappleActivationCheckTimer(int time);
  bool HasAimFocus() const;
  void SetAimFocusActive(bool value) { aimFocusActive = value; }
  int GetGrappleActivationCheckTimer() const {
    return grappleActivationCheckTimer;
  }

  enum aiDebugLevel_t {
    AIDEBUGLEVEL_NONE = 0,
    AIDEBUGLEVEL_ERROR = 1,
    AIDEBUGLEVEL_WARNING = 2,
    AIDEBUGLEVEL_INFO = 3,
  };

  eventVoid* AnimEvent_StopVoiceOver(eventVoid* result);
  void Action_LeapAttack(eventVoid* result, const idVec3* leapTarget);
  idPlayer* GetPlayerTarget(aiPlayer_t player);
  eventVoid* Action_Trigger(eventVoid* result, idEntity* entity);
  eventVoid* Action_MoveToPoint(eventVoid* result, const idVec3* destination,
                                int stopUponArrival, float tolerance,
                                aiActionBool_t align,
                                const idVec3* alignDirection);
  eventVoid* Action_MoveToPointNoFail(
      eventVoid* result, const idVec3* destination, int stopUponArrival,
      float tolerance, aiActionBool_t align, const idVec3* alignDirection,
      const idDeclVoiceOver* voiceOver);
  eventVoid* Action_MoveToEntity(eventVoid* result, const idEntity* entity,
                                 int stopUponArrival, float reachedDistance,
                                 aiActionBool_t align,
                                 const idVec3* alignDirection);
  eventVoid* Action_MoveToEntityNoFail(
      eventVoid* result, const idEntity* entity, int stopUponArrival,
      float reachedDistance, aiActionBool_t align,
      const idVec3* alignDirection, const idDeclVoiceOver* voiceOver);
  eventVoid* Action_MoveToPathPoint(eventVoid* result,
                                    const idEntity* pathPoint);
  eventVoid* Action_MoveToPathPointNoFail(
      eventVoid* result, const idEntity* pathPoint,
      const idDeclVoiceOver* voiceOver);
  eventVoid* Action_MoveToCover(eventVoid* result, const idVec3* destination,
                                int coverAction, posture_t posture,
                                const idVec3* alignDirection);
  eventVoid* Action_UseZipline(eventVoid* result, const idEntity* entity);
  eventVoid* Action_SetMovePushStatus(eventVoid* result, int pushMoveStatus);
  eventVoid* Action_Idle(eventVoid* result, int time, int idleTurnType);
  eventVoid* Action_Pain(eventVoid* result, painType_t painType,
                         aiDirection_t direction);
  eventVoid* Action_StandToCrouch(eventVoid* result);
  eventVoid* Action_CrouchToStand(eventVoid* result);
  eventVoid* Action_PullTriggerRight(eventVoid* result);
  eventVoid* Action_ReleaseTriggerRight(eventVoid* result);
  eventVoid* Action_PullTriggerLeft(eventVoid* result);
  eventVoid* Action_ReleaseTriggerLeft(eventVoid* result);
  eventVoid* Action_WaitForAnim(eventVoid* result,
                                const idAnimWebPath* animWebNode);
  eventVoid* Action_WaitForAnimVia(eventVoid* result,
                                   const idAnimWebPath* destinationNode,
                                   const idAnimWebPath* viaNode);
  eventVoid* Action_LoopAnim(eventVoid* result,
                             const idAnimWebPath* aliasName,
                             int milliseconds);
  eventVoid* Action_LoopAnimExitAtEnd(eventVoid* result,
                                      const idAnimWebPath* animWebNode,
                                      int milliseconds, int framesFromEnd);
  eventVoid* Action_StartAnim(eventVoid* result,
                              const idAnimWebPath* animWebNode,
                              aiActionBool_t finishBlend,
                              aiActionBool_t wait);
  eventVoid* Action_WaitForTraversalAnim(eventVoid* result,
                                         const idAnimWebPath* animWebNode,
                                         aiActionBool_t runWhenDone);
  eventVoid* Action_ChangeAnimState(eventVoid* result, int animWeb,
                                    const idAnimWebPath* destinationNode,
                                    aiAnimWait_t wait);
  eventVoid* Action_ChangeAnimStateVia(eventVoid* result, int animWeb,
                                       const idAnimWebPath* destinationNode,
                                       const idAnimWebPath* viaNode,
                                       aiAnimWait_t wait);
  eventVoid* Action_ForceAnimState(eventVoid* result, int animWeb,
                                   const idAnimWebPath* destinationNode,
                                   int blendDurationMilliseconds,
                                   aiAnimWait_t wait);
  eventVoid* Action_Dodge(eventVoid* result, const idVec3* dodgeFrom);
  eventVoid* Action_Wait(eventVoid* result, int milliseconds);
  eventVoid* Action_Melee(eventVoid* result, int overrideAnimation);
  eventVoid* Action_TurnToPoint(eventVoid* result, const idVec3* point);
  eventVoid* Action_TurnToEntity(eventVoid* result, const idEntity* entity);
  eventVoid* Action_TurnToEntityWithOffset(eventVoid* result,
                                           const idEntity* entity,
                                           float yawOffset);
  eventVoid* Action_Dive(eventVoid* result, float diveYaw, float faceYaw);
  eventVoid* Action_WaitForEntity(eventVoid* result, const idEntity* entity,
                                  float range);
  eventVoid* Action_PlayVoiceOver(eventVoid* result,
                                  const idDeclVoiceOver* voiceOver,
                                  aiActionBool_t waitUntilFinished);
  eventVoid* Action_PlayInteractionVoiceOver(
      eventVoid* result, const idDeclVoiceOver* voiceOver,
      const idEntity* trackEntity, float abortDistance, bool abortScript);
  eventVoid* Action_PlayOverrideAnim(eventVoid* result, int overrideAnim,
                                     aiActionBool_t cycle);
  eventVoid* Action_PlayOverrideAnimInterrupt(eventVoid* result,
                                              int overrideAnim,
                                              aiActionBool_t cycle);
  eventVoid* Action_HolsterWeapon(eventVoid* result);
  eventVoid* Action_DrawWeapon(eventVoid* result);
  eventVoid* Action_PerformCoverAction(eventVoid* result, int coverAction,
                                       int duration);
  eventVoid* Action_WaitForPlayerInteraction(eventVoid* result);
  eventVoid* Action_WaitForPlayerInteractionDist(eventVoid* result,
                                                 float maxDistance);
  eventVoid* Action_WaitForAIVar(eventVoid* result, const char* name, int op,
                                 const char* value);
  eventVoid* Action_SetAlertCycle(eventVoid* result, int alertCycle);
  eventVoid* Action_SetSubWeb(eventVoid* result, int subWeb);
  eventVoid* Action_SetFocus(eventVoid* result, const idEntity* ent,
                             aiActionBool_t aiming,
                             aiActionBool_t keepFocusInView,
                             aiActionBool_t trackOnlyVisible,
                             aimPoint_t aimPoint);
  eventVoid* Action_ClearLookFocus(eventVoid* result);
  eventVoid* Action_ClearAimFocus(eventVoid* result);
  eventVoid* Action_SetAimPoint(eventVoid* result, int aimPoint);
  eventVoid* Action_SetFireMode(eventVoid* result, int fireMode);
  eventVoid* Action_SetScriptAbort(eventVoid* result,
                                   aiActionBool_t abortOnCombat,
                                   aiActionBool_t abortOnSearch);
  eventVoid* Action_SetPosture(eventVoid* result, int state);
  eventVoid* Action_SetWalkState(eventVoid* result, int state);
  eventVoid* Action_SetSitState(eventVoid* result, int state);
  eventVoid* Action_SetStandState(eventVoid* result, int state);
  eventVoid* Action_SetIdleState(eventVoid* result, int indexType);
  eventVoid* Action_IgnorePlayerApproach(eventVoid* result,
                                         aiActionBool_t permanent);
  eventVoid* Action_NoticePlayerApproach(eventVoid* result,
                                         aiActionBool_t permanent);
  eventVoid* Action_GiveItem(eventVoid* result, const idEntity* giveTo,
                             const idDeclInventory* itemDecl);
  eventVoid* Action_TakeItem(eventVoid* result, const idEntity* takeFrom,
                             const idDeclInventory* itemDecl);
  eventVoid* Action_SetPlayerEnemy(eventVoid* result, int player);
  eventVoid* Action_SetEnemy(eventVoid* result, const idEntity* entity);
  eventVoid* Action_SearchToTarget(eventVoid* result, const idEntity* target);
  eventVoid* Action_SetPlayerFocus(eventVoid* result, int player,
                                   aiActionBool_t aiming,
                                   aiActionBool_t keepFocusInView);
  eventVoid* Action_SetActionNodeGroup(eventVoid* result, const char* group);
  eventVoid* Action_EnableDamage(eventVoid* result, aiActionBool_t enabled,
                                 aiActionBool_t permanent);
  eventVoid* Action_EnableAutoFocus(eventVoid* result, aiActionBool_t enabled,
                                    aiActionBool_t permanent);
  eventVoid* Action_EnableBodyRotation(eventVoid* result,
                                       aiActionBool_t enabled);
  eventVoid* Action_EnableWalkIK(eventVoid* result, aiActionBool_t enabled);
  eventVoid* Action_EnableHeadTracking(eventVoid* result,
                                       aiActionBool_t enabled);
  eventVoid* Action_ForceAwarenessByDistance(eventVoid* result, float radius,
                                             bool playerFlag, bool aiFlag);
  eventVoid* Action_SetAIVar(eventVoid* result, const char* name, int type,
                             const char* value);
  eventVoid* Action_SetScriptFlag(eventVoid* result, int flag);
  eventVoid* Action_ClearScriptFlag(eventVoid* result, int flag);
  eventVoid* Action_SetPerceptionFlag(eventVoid* result, int flag);
  eventVoid* Action_ClearPerceptionFlag(eventVoid* result, int flag);
  eventVoid* Action_ClearWorldState(eventVoid* result);
  eventVoid* Action_ShowAttachment(eventVoid* result, const char* slotName);
  eventVoid* Action_HideAttachment(eventVoid* result, const char* slotName);
  eventVoid* Action_DropAttachment(eventVoid* result, const char* slotName);
  eventVoid* Action_SetMoveMode(eventVoid* result, int moveMode);
  eventVoid* Action_SetAccuracy(eventVoid* result, int accuracy);
  eventVoid* Action_ScriptedAnimWeb(eventVoid* result,
                                    const idEntity* scriptedAnimWeb,
                                    int loopInterval);
  eventVoid* Action_ReloadWeapon(eventVoid* result, unsigned int slot,
                                 const char* ammoDeclName);
  eventVoid* Action_ReloadWeaponTorso(eventVoid* result, unsigned int slot,
                                      const char* ammoDeclName);

  eventVoid* Event_EnableAutoFocus(eventVoid* result, const idAIAnimNodePlayback* anim);
  std::int64_t SetupMeleeTraceBounds(float radius);
  eventVoid* AnimEvent_TriggerGore(eventVoid* result, ...);
  eventVoid* AnimEvent_BlendRenderParameter(eventVoid* result, ...);
  eventVoid* AnimEvent_DropAttachedProjectiles(eventVoid* result, ...);
  eventVoid* AnimEvent_HideFakeGrenade(eventVoid* result, ...);
  eventVoid* AnimEvent_ShowFakeGrenade(eventVoid* result, ...);
  eventVoid* AnimEvent_DeathAnimImpulse(eventVoid* result, ...);
  eventVoid* AnimEvent_DeathTrace(eventVoid* result, ...);
  eventVoid* AnimEvent_DeathPhys(eventVoid* result, ...);
  eventVoid* AnimEvent_SmoothAnimSetDone(eventVoid* result, ...);
  eventVoid* AnimEvent_SmoothAnimDestinationLerpStop(eventVoid* result, ...);
  eventVoid* AnimEvent_UseAdditivePainAnims(eventVoid* result, ...);
  eventVoid* AnimEvent_AllowExecutions(eventVoid* result, ...);
  eventVoid* AnimEvent_AIReloadWeapon(eventVoid* result, ...);
  eventVoid* AnimEvent_AIFireWeapon(eventVoid* result, ...);
  eventVoid* AnimEvent_AISetFireMode(eventVoid* result, ...);
  eventVoid* AnimEvent_AIInhibitFire(eventVoid* result, ...);
  eventVoid* AnimEvent_Kill(eventVoid* result, ...);
  eventVoid* AnimEvent_EquipItem(eventVoid* result, ...);
  eventVoid* AnimEvent_HolsterItem(eventVoid* result, ...);
  eventVoid* AnimEvent_DropAttachment(eventVoid* result, ...);
  eventVoid* AnimEvent_MeleeCustom(eventVoid* result, ...);
  eventVoid* AnimEvent_MeleeCustomForceDirection(eventVoid* result, ...);
  eventVoid* AnimEvent_MeleePunch(eventVoid* result, ...);
  eventVoid* AnimEvent_MeleeKick(eventVoid* result, ...);
  eventVoid* AnimEvent_ThrowGrenade(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableHeadTracking(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableHeadTracking(eventVoid* result, ...);
  eventVoid* AnimEvent_SetLookFocusHeadTrackingGroup(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableEyeTracking(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableEyeTracking(eventVoid* result, ...);
  eventVoid* AnimEvent_OverrideAnimOrientationWithFocus(eventVoid* result, ...);
  eventVoid* AnimEvent_VoiceOver(eventVoid* result, ...);
  eventVoid* AnimEvent_VoiceOverPriority(eventVoid* result, ...);
  eventVoid* AnimEvent_StopAutoLipsync(eventVoid* result, ...);
  eventVoid* AnimEvent_StartAutoLipsync(eventVoid* result, ...);
  eventVoid* AnimEvent_SetMovePushStatus(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableWorldCollision(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableWorldCollision(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableSolidCollision(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableSolidCollision(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableDamage(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableDamage(eventVoid* result, ...);
  eventVoid* AnimEvent_ClearReferenceJointOverride(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableExtendedClipModel(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableExtendedClipModel(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableIK(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableIK(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableIKAnimDuration(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableLegIK(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableLegIK(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableTwoLegIK(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableTwoLegIK(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableFourLegIK(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableFourLegIK(eventVoid* result, ...);
  eventVoid* AnimEvent_EndMeleeTrace(eventVoid* result, ...);
  eventVoid* AnimEvent_StartJetPackFX(eventVoid* result, ...);
  eventVoid* AnimEvent_StopJetPackFX(eventVoid* result, ...);
  eventVoid* AnimEvent_DropJetPack(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableBlinking(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableBlinking(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableLidDeformation(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableLidDeformation(eventVoid* result, ...);
  eventVoid* AnimEvent_SetLookFocusOffset(eventVoid* result, ...);
  eventVoid* AnimEvent_SetAimFocusOffset(eventVoid* result, ...);
  eventVoid* AnimEvent_SetLookFocusRotationOffset(eventVoid* result, ...);
  eventVoid* AnimEvent_SetAimFocusRotationOffset(eventVoid* result, ...);
  eventVoid* AnimEvent_LockTracking(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableStaggeringPain(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableStaggeringPain(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableDeathAnimations(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableDeathAnimations(eventVoid* result, ...);
  eventVoid* AnimEvent_EnableCanRagdoll(eventVoid* result, ...);
  eventVoid* AnimEvent_DisableCanRagdoll(eventVoid* result, ...);
  eventVoid* AnimEvent_StartWeaponFX(eventVoid* result, ...);
  eventVoid* AnimEvent_StopWeaponFX(eventVoid* result, ...);
  eventVoid* AnimEvent_DeltaScaleToTarget(eventVoid* result, ...);
  eventVoid* AnimEvent_SetDeltaScale(eventVoid* result, ...);
  eventVoid* AnimEvent_SetScriptDeltaScale(eventVoid* result, ...);
  eventVoid* AnimEvent_PopOffArmor(eventVoid* result, ...);
  eventVoid* AnimEvent_SetDefaultMoveMode(eventVoid* result, ...);
  eventVoid* AnimEvent_SetFullDeltaGravityMovementMode(eventVoid* result, ...);
  eventVoid* AnimEvent_Death(eventVoid* result, ...);
  eventVoid* AnimEvent_ForceAllowLooting(eventVoid* result, ...);
  eventVoid* AnimEvent_BeginDrop(eventVoid* result, ...);
  eventVoid* AnimEvent_ParticleAtRottrack(eventVoid* result, ...);
  eventVoid* AnimEvent_PlayVoice(eventVoid* result, ...);
  eventVoid* AnimEvent_SetHealth(eventVoid* result, const idAIAnimNodePlayback* anim, int value);
  eventVoid* AnimEvent_AnimEndEnterAnimation(eventVoid* result, ...);
  eventVoid* AnimEvent_AnimReleaseWaitHandle(eventVoid* result, ...);
  eventVoid* AnimEvent_SetTakedownWindowState(eventVoid* result, ...);
  eventVoid* AnimEvent_SoundHurt(eventVoid* result, ...);
  eventVoid* AnimEvent_SoundDeath(eventVoid* result, ...);
  eventVoid* AnimEvent_CustomDeathSound(eventVoid* result, ...);
  eventVoid* AnimEvent_AnimationDone(eventVoid* result, ...);
  eventVoid* AnimEvent_SetFade(eventVoid* result, const idAIAnimNodePlayback* anim, float value);
  eventVoid* AnimEvent_StartRagdoll(eventVoid* result, ...);
  eventVoid* AnimEvent_KillSyncedTarget(eventVoid* result, ...);
  eventVoid* AnimEvent_LaunchItem(eventVoid* result, ...);
  eventVoid* AnimEvent_LaunchItemFromWeapon(eventVoid* result, ...);
  eventVoid* AnimEvent_SetReferenceJointOverride(eventVoid* result, ...);
  const idDeclDamage* GetMeleeTraceDamage(const idDeclDamage* damageDecl);
  const idDeclProjectileImpactEffect* GetMeleeTraceImpactEffects(bool useWeaponEffects);
  eventVoid* AnimEvent_StartMeleeTrace(eventVoid* result, ...);
  eventVoid* AnimEvent_StartMeleeTrace2(eventVoid* result, ...);
  eventVoid* AnimEvent_StartMeleeTraceTag(eventVoid* result, ...);
  eventVoid* AnimEvent_StartMeleeTraceTag2(eventVoid* result, ...);
  eventVoid* AnimEvent_RandomRunCycleIndex(eventVoid* result, ...);
  eventVoid* AnimEvent_RandomRunCycleNormal(eventVoid* result, ...);
  eventVoid* AnimEvent_StartDropToGround(eventVoid* result, ...);
  eventVoid* AnimEvent_DetonateFakeGrenade(eventVoid* result, ...);
  eventVoid* AnimEvent_MeleePistolWhip(eventVoid* result, ...);
  eventVoid* AnimEvent_TraversalAttackChoice(eventVoid* result, ...);
  eventVoid* AnimEvent_Explode(eventVoid* result, ...);
  eventVoid* AnimEvent_SetAimFocusToEnemy(eventVoid* result, ...);
  eventVoid* AnimEvent_SmoothAnimDestinationLerpStart(eventVoid* result, ...);
  eventVoid* AnimEvent_TraversalLerpStart(eventVoid* result, ...);
  eventVoid* AnimEvent_GiveItem(eventVoid* result, ...);
  eventVoid* AnimEvent_PushEntity(eventVoid* result, ...);

  void GetMovementStateName(idStr* output);
  void GetWeaponPrefix(const idWeapon* weapon, idStr* output);
  void GetWeaponPrefix(idStr* output);
  void GetSubWebPrefix(const idWeapon* weapon, int subWeb, idStr* output);
  void GetSubWebPrefix(aiSubWeb_t subWeb, idStr* output);
  void GetSubWebPrefix(idStr* output);
  void GetIdleStateName(idStr* output);
  void GetIdleTurnStateName(int turnDirection, float turnDeltaDot,
                            const char* baseTurnName, idStr* output);
  int ChangeAnimState(int web, int subWebIndex, int stateIndex,
                      int interruptPath, int interruptBlend);
  std::int64_t ChangeAnimState(int web, aiSubWeb_t subWeb, int stateIndex,
                               int interruptPath, int interruptBlend,
                               idAnimWebIndex* subWebIndex);
  std::int64_t ChangeAnimState(int web, aiSubWeb_t subWeb,
                               const char* stateName, int interruptPath,
                               int interruptBlend, idAnimWebIndex* subWebIndex,
                               idAnimWebIndex* stateIndex);
  void FindAnimWebAndStateIndices(int web, int subWeb, const char* stateName,
                                  bool changeWeapon, const idWeapon* newWeapon,
                                  idAnimWebIndex* subWebIndex,
                                  idAnimWebIndex* stateIndex);
  int ChangeAnimStateVia(int web, int subWebIndex, int stateIndex,
                         int viaSubWebIndex, int viaStateIndex,
                         int interruptPath, int interruptBlend);
  std::int64_t ChangeAnimStateVia(int web, aiSubWeb_t subWeb, int stateIndex,
                                  aiSubWeb_t viaSubWeb, int viaStateIndex,
                                  int interruptPath, int interruptBlend,
                                  idAnimWebIndex* subWebIndex,
                                  idAnimWebIndex* viaSubWebIndex);
  std::int64_t ChangeAnimStateVia(int web, aiSubWeb_t subWeb,
                                  const char* stateName, aiSubWeb_t viaSubWeb,
                                  const char* viaStateName, int interruptPath,
                                  int interruptBlend, idAnimWebIndex* subWebIndex,
                                  idAnimWebIndex* stateIndex,
                                  idAnimWebIndex* viaSubWebIndex,
                                  idAnimWebIndex* viaStateIndex);
  void GetSubWebName(const idStr* input, idStr* output);
  std::int64_t ForceAndChangeAnimWebState(const char* viaSubWebName,
                                          const char* viaStateName,
                                          const char* destinationSubWebName,
                                          const char* destinationStateName,
                                          const void* blendParameters);
  bool GetSubWebAndStateFromAnimWebPath(const char* path, idStr* subWebName,
                                        idStr* stateName);
  bool GetAnimWebNodeForOverride(int overrideType, idAnimWebIndex* subWebIndex,
                                 idAnimWebIndex* stateIndex);
  const idAIAnimNodePlayback* GetAnimForAnimWebNode(const idAnimWebIndex* subWebIndex,
                                         const idAnimWebIndex* stateIndex);
  float GetAnimTimeScaleForAnimWebNode(const idAnimWebIndex* subWebIndex,
                                       const idAnimWebIndex* stateIndex);
  const idAIAnimNodePlayback* GetAnimForAnimWebNodeIndex(
      const idAnimWebIndex* subWebIndex, const idAnimWebIndex* stateIndex,
      int index);
  void GetMovementSubWebAndStateIndices(idAnimWebIndex* subWebIndex,
                                         idAnimWebIndex* stateIndex);
  void GetCurrentSubWebAndStateIndices(idAnimWebIndex* subWebIndex,
                                        idAnimWebIndex* stateIndex);
  void SetDebugText(const char* text, aiDebugLevel_t debugLevel);
  void ClearErrorFlags(int flags);
  void ShowDebugDamageText(float baseDamage, float difficultyScale,
                           float bodyDamage, float limbDamage,
                           float armorDamage, float shieldDamage,
                           float armorDamageScale,
                           float armoredHealthDamageScale,
                           float bleedThroughDamage, float excessDamage,
                           float attitudeDamageScale,
                           const idDamageGroup* damageGroup,
                           const idVec3* impactPoint);
  static std::int64_t GetDebugLevel();
  static void SetDebugLevel(int value);
  eventVoid* Event_MoveToPathPoint(eventVoid* result, const idEntity* pathPoint);
  eventVoid* Event_SetAlertCycle(eventVoid* result, int alertCycle);
  eventInt* Event_SetCallback(eventInt* result);
  eventInt* Event_GetCurrentAIEventClass(eventInt* result);
  eventInt* Event_GetLastAIEventClass(eventInt* result);
  eventVector* Event_GetLastAIEventOrigin(eventVector* result);
  eventBool* Event_TeslaIsActive(eventBool* result);
  eventVoid* Event_UpdateGroupAmbush(eventVoid* result, ...);
  eventVoid* Event_SetGlobalAggro(eventVoid* result, float aggression);
  eventVoid* Event_KillAI(eventVoid* result);
  eventVoid* Event_AnimWeb_Wait(eventVoid* result, ...);
  eventVoid* Event_AnimWeb_BeginAnim(eventVoid* result, ...);
  eventVoid* Event_AnimWeb_ReloadRightFinished(eventVoid* result, ...);
  eventVoid* Event_AnimWeb_EnableFireFromCover(eventVoid* result, ...);
  eventVoid* Event_RelinquishControl(eventVoid* result);
  eventVoid* Event_TurnOffLaserSight(eventVoid* result);
  eventVoid* Event_TurnOnLaserSight(eventVoid* result);
  eventVoid* Event_Path(eventVoid* result, idEntity* newPath);
  eventVoid* Event_SetOverrideScenePoint(eventVoid* result, idEntity* scenePoint);
  eventVector* Event_GetBodyOrientation(eventVector* result);
  eventBool* Event_StartVoiceOver(eventBool* result,
                                  const idDeclVoiceOver* voiceOver, ...);
  eventBool* Event_IsVoiceOverPlaying(eventBool* result,
                                      const idDeclVoiceOver* voiceOver);
  eventBool* Event_IsPlayingVoiceOver(eventBool* result);
  eventDecl* Event_GetPlayingVoiceOver(eventDecl* result);
  eventVoid* Event_SetFocus(eventVoid* result, ...);
  eventVoid* Event_ClearLookFocus(eventVoid* result);
  eventVoid* Event_ClearAimFocus(eventVoid* result);
  eventVoid* Event_DisableAutoFocus(eventVoid* result, ...);
  eventVoid* Event_SetRemoveHealthWhenDamaged(eventVoid* result, bool value);
  eventVoid* Event_SetEventOverloadEnableFlag(eventVoid* result, int flags);
  eventVoid* Event_ClearEventOverloadEnableFlag(eventVoid* result, int flags);
  eventVoid* Event_SetEventOverloadDisableFlag(eventVoid* result, int flags);
  eventVoid* Event_ClearEventOverloadDisableFlag(eventVoid* result, int flags);
  eventVoid* Event_SetBehaviorDecl(eventVoid* result, ...);
  eventInt* Event_ShouldTurnTo(eventInt* result, ...);
  eventBool* Event_IsEntityVisible(eventBool* result, const idEntity* entity);
  eventVoid* Event_TriggerGore(eventVoid* result, ...);
  eventVoid* Event_SetCloseMeleeInterval(eventVoid* result, float interval);
  eventVoid* Event_SetMediumMeleeInterval(eventVoid* result, float interval);
  eventVoid* Event_SetLongMeleeInterval(eventVoid* result, float interval);
  eventVoid* Event_EnableTaunts(eventVoid* result, bool enable);
  eventVoid* Event_Push(eventVoid* result, ...);
  eventVoid* Event_StopAllEffects(eventVoid* result);
  eventVoid* Event_FakeSenseStimulus(eventVoid* result, ...);
  eventVoid* Event_SetTeslaCooldown(eventVoid* result, float duration);
  eventVoid* Event_SetFOV(eventVoid* result, float fov);
  eventVoid* Event_SetEMPStunDuration(eventVoid* result, float duration);
  void LocationCallingHelper(idEntity* entity);
  eventVoid* Event_SpawnLootBox(eventVoid* result);
  eventVoid* Event_RemoveBody(eventVoid* result);
  eventVoid* Event_DisintigrateHide(eventVoid* result);
  eventVoid* Event_SetAnimWebScriptBlend(eventVoid* result, float blendValue);
  eventVoid* Event_RemoveTeslaBlindFirePosition(eventVoid* result,
                                                const idVec3* position);
  eventVoid* Event_ClearTeslaBlindFirePositions(eventVoid* result);
  eventVoid* Event_AwarenessLost(eventVoid* result, ...);
  eventVoid* Event_ResetSearchPoints(eventVoid* result);
  void EnemySightedHelper(idEntity* entity, bool firstSight);
  eventVoid* Event_AwarenessRegained(eventVoid* result, ...);
  eventVoid* Event_FlyBack(eventVoid* result);
  eventEntity* Event_GetLastAIEventOriginator(eventEntity* result);
  eventEntity* Event_GetLastAIEventInstigator(eventEntity* result);
  eventVoid* Event_SetTakedownRestrictionVolume(eventVoid* result,
                                                idEntity* volume);
  eventVoid* Event_SetScenePointUserGroup(eventVoid* result,
                                          idEntity* userGroup);
  eventEntity* Event_GetAvoidEntity(eventEntity* result);
  eventVoid* Event_AwarenessGained(eventVoid* result, ...);
  eventVoid* Event_Surprised(eventVoid* result, ...);
  eventVoid* Event_AddAggression(eventVoid* result, ...);
  eventVoid* Event_AnimWeb_StartUseTurret(eventVoid* result, ...);
  eventVoid* Event_AnimWeb_BeginSyncedAnim(eventVoid* result, ...);
  eventVoid* Event_ActionNode(eventVoid* result, idEntity* actionNode);
  eventVoid* Event_Touch(eventVoid* result, ...);
  eventVoid* Event_AddTeslaBlindFirePosition(eventVoid* result,
                                             const idVec3* position);
  eventVoid* Event_BlendRenderParameter(eventVoid* result, int parameter,
                                        float amount, float blendTime);
  eventVoid* Event_DropAttachedProjectiles(eventVoid* result);
  void ExecuteThread(idThread* thread);
  bool AllThreadsAreFinished();
  bool IsExecutingScript();
  void RemoveThread(idThreadHandle threadHandle);
  idThreadHandle GetThread(int index);
  std::int64_t NumThreads();
  void ResetFSMWaitThreadIfPossible(idThread* thread);
  bool ForceFSMIntoExecutingCodeAction(const idEventDef* action);
  std::int64_t AddThread(idThreadHandle threadHandle);
  std::int64_t AddFiber(idFiberHandle fiberHandle);
  void FinishFiber(idFiberHandle fiberHandle);
  bool SelectThrowable(int selectorType, const idVec3* targetPosition,
                       int coverAction, idAIProjectileInfo* info);
  bool HasWaitingTrajectoryResults();
  void ClearTrajectoryResults();
  bool GetMuzzleTransform(idWeapon* weapon, idVec3* muzzlePosition,
                          idMat3* muzzleAxis);
  bool GetMuzzlePosition(idWeapon* weapon, idVec3* muzzlePosition);
  bool GetLaserSightTransform(idWeapon* weapon, idVec3* laserPosition,
                              idMat3* laserAxis);
  static void TargetMovingPoint(const idVec3* launchPoint, float launchSpeed,
                                const idVec3* targetPosition,
                                const idVec3* targetVelocity, float* lagTime,
                                float* animationDelay, idVec3* launchDirection,
                                idVec3* predictedTargetPosition);
  bool LaunchParabolicProjectile(const idDeclProjectile* projectileDecl,
                                 const idParabolicParms* trajectoryParameters,
                                 const idVec3* targetPosition,
                                 const idVec3* startPosition, int startTime);
  bool LeadTargetWithLinearProjectile(
      const idAIProjectileInfo* info, const idVec3* currentTargetPosition,
      const idVec3* currentTargetVelocity, const idVec3* projectileStart,
      idVec3* extrapolatedPosition, idVec3* directionToExtrapolatedPosition);
  bool LaunchLinearProjectile(const idDeclProjectile* projectileDecl,
                              float speed, const idVec3* startPosition,
                              const idVec3* targetPosition,
                              idVec3* launchDirection, int startTime);
  const idCachedJoint* GetModelSpaceJointTransformForAnimEvent(
      const idEventDef* eventDefinition, const idAIAnimNodePlayback* animation,
      int cachedIndex, idVec3* jointOrigin, idMat3* jointAxis,
      idMilliseconds* eventTime);
  const idCachedJoint* GetWorldSpaceJointTransformForAnimEvent(
      const idEventDef* eventDefinition, const idAIAnimNodePlayback* animation,
      int cachedIndex, idVec3* worldOrigin, idMat3* worldAxis,
      idMilliseconds* eventTime);
  bool SetupGrenadeThrow(idScenePoint_Combat* combatScenePoint,
                         const idAIPos* grenadePosition);

  bool ActionScriptFlagIsSet(int flags) const;
  idRecoveredValue ActivateExtendedClipModel(...);
  idRecoveredValue ActivateStealthKillTrigger(...);
  idRecoveredValue AimDebug(...);
  idRecoveredValue AlarmNotification(...);
  int AlertCycleToSubWeb(int alertCycle);
  idRecoveredValue AllocPresentable(...);
  bool AnimEventReceived(idAnimWebEvent::priority_t priority) const;
  idRecoveredValue ApplyDamage(...);
  idRecoveredValue ApplyDeathAnimationLogic(...);
  idRecoveredValue ApplyFakeDamageImpulse(...);
  void ApplyForce(int physicsId, int bodyId, const idVec3* point,
                  const idVec3* force);
  void ApplyImpulse(int physicsId, int bodyId, const idVec3* point,
                    const idVec3* impulse);
  idRecoveredValue ApplyWaterEffects(...);
  bool AwareOfEnemy() const;
  idRecoveredValue BeginSync(...);
  idRecoveredValue CalcMovementTurnRate(...);
  float CalcTurnRadius(float speed, float turnRate);
  idRecoveredValue CanBeTakenDown(...);
  bool CanFireRobot(int currentTime);
  bool CanReceiveAIEvents(int currentTime) const;
  bool CanTaunt() const;
  bool CanTeslaBlindFire(idAIEntityState* enemyState, int duration);
  idRecoveredValue CheckDeathCollision(...);
  idRecoveredValue CheckDelayedVoice(...);
  idRecoveredValue CheckForGore(...);
  idRecoveredValue CheckForGoreByName(...);
  idRecoveredValue CheckForOneShotKill(...);
  idRecoveredValue CheckForSuspectedToTrailUpdate(...);
  idRecoveredValue CheckPain(...);
  void ClearAIClip();
  void ClearAimFocus();
  idRecoveredValue ClearAllAnimWebEvents(...);
  idRecoveredValue ClearAnimWebEvent(...);
  idRecoveredValue ClearFakeEnemy(...);
  void ClearLookFocus();
  void ClearSyncedAnimData();
  void ClearWaitForOverrideAnim(idAnimWebEvent::priority_t priority);
  idRecoveredValue ClearWorldState(...);
  idRecoveredValue Damage(...);
  idRecoveredValue DamageFeedback(...);
  idRecoveredValue DamageGroupPopOffArmor(...);
  idRecoveredValue DamageRobot(...);
  idRecoveredValue DeactivateExtendedClipModel(...);
  bool DeathPending() const;
  idRecoveredValue Debug_Perception(...);
  idRecoveredValue Debug_ScenePoints(...);
  void DisableAIEventResponse(int flags);
  void DisableAnimatorsOnDeath();
  idRecoveredValue DisableInfluenceTrail(...);
  idRecoveredValue DoBlockedMeleeReaction(...);
  idRecoveredValue DoEmergencyCoverExit(...);
  idRecoveredValue DoMeleeDamage(...);
  idRecoveredValue DoMeleeDamage(idRecoveredOverloadTag<1>);
  idRecoveredValue DormantBegin(...);
  idRecoveredValue DormantEnd(...);
  idRecoveredValue DoSpawnSettings(...);
  idRecoveredValue Draw(...);
  idRecoveredValue Dropped(...);
  void EnableAIEventResponse(int flags);
  void EndActionScript();
  idRecoveredValue EndSync(...);
  idRecoveredValue EquipItem(...);
  idRecoveredValue Event_DetectPlayer(...);
  idRecoveredValue FindAutoFocus(...);
  idRecoveredValue FindAutoFocusEntity(...);
  idRecoveredValue FindClosestFocus(...);
  const idWeapon* FindWeaponToDraw() const;
  idRecoveredValue FinishFireWeapon(...);
  idRecoveredValue FirstThink(...);
  idRecoveredValue ForceStopOverrideAnim(...);
  idRecoveredValue GetAAS(...);
  idRecoveredValue GetAASPositionMgr(...);
  idLegsFSM* GetActionFSM();
  int GetActionStatus() const;
  idRecoveredValue GetAlertCycle(...);
  idRecoveredValue GetAlertCycleDuration(...);
  idRecoveredValue GetAlertCycleFSM(...);
  bool GetAnimationDelta(const idMD6Anim* animation, idVec3* outDelta) const;
  idRecoveredValue GetAnimationDelta(idRecoveredOverloadTag<1>);
  idRecoveredValue GetAnimationDelta(idRecoveredOverloadTag<2>);
  idRecoveredValue GetAnimationDeltaAndRot(...);
  idRecoveredValue GetAnimationDuration(...);
  idRecoveredValue GetAnimationPathReachability(...);
  idRecoveredValue GetAnimationPathReachability(idRecoveredOverloadTag<1>);
  idRecoveredValue GetAnimationRot(...);
  idRecoveredValue GetAnimationRot(idRecoveredOverloadTag<1>);
  idRecoveredValue GetAnimatorAnimWeb(...);
  idAIOrientation* GetBodyOrientation();
  idRecoveredValue GetBurstDuration(...);
  idRecoveredValue GetComponentTimeLine(...);
  idPlayer* GetControllingPlayer() const;
  int GetCoverActionDuration(coverAction_t action, int currentTime);
  idRecoveredValue GetCoverTransitionRadius(...);
  const idCombatStage* GetCurrentCombatStage() const;
  float GetDefaultSpeedByCurrentWalkState() const;
  idRecoveredValue GetDirectionTo(...);
  idRecoveredValue GetDirectionToForReactions(...);
  idRecoveredValue GetDynamicCoverMgr(...);
  int GetEffectiveAccuracy() const;
  bool GetEnableHeadTracking();
  idEncounterGroup* GetEncounterGroup() const;
  bool GetEnemyInsideGoNutsRange() const;
  bool GetEnemyInsideMaxFireRange(const idEntity* enemy) const;
  idRecoveredValue GetEnemyLookDot(...);
  idRecoveredValue GetEnemySenseState(...);
  unsigned int GetEventResponseFlags();
  idRecoveredValue GetEyeOffset(...);
  const idDeclFacialAnimationSet* GetFacialAnimationSet() const;
  idRecoveredValue GetFakeEnemy(...);
  idRecoveredValue GetFakeEnemyWeight(...);
  void GetFireAtLastKnownDuration(const idWeapon* weapon, int* minimum,
                                  int* maximum) const;
  idRecoveredValue GetHealthComponent(...);
  int GetIdleIndexForType(runIndexType_t type) const;
  idRecoveredValue GetInputSettings(...);
  idRecoveredValue GetJointIndexFromTrace(...);
  idRecoveredValue GetMeleeDirection(...);
  idRecoveredValue GetMeleeInterval(...);
  idRecoveredValue GetMeleeTypeForTarget(...);
  idRecoveredValue GetMissTimeAimPosition(...);
  void GetModelTransform(idVec3* modelOrigin, idMat3* modelAxis) const;
  int GetModifiedSkill();
  idAIMoveState* GetMoveInterface();
  idAIOrientation* GetMoveOrientation();
  idRecoveredValue GetOnlineUsableState(...);
  float GetPlayerViewDataFitness() const;
  idRecoveredValue GetRenderParam(...);
  idRecoveredValue GetRepullTriggerInterval(...);
  int GetRunCycleIndexForType(runIndexType_t type) const;
  void GetSoundTransform(idVec3* modelOrigin, idMat3* modelAxis) const override;
  float GetSpotGainedFireTime() const;
  float GetSpotLostFireTime() const;
  idRecoveredValue GetSpottedDuration(...);
  idRecoveredValue GetSuspectedEnemyPos_PushedIntoAAS(...);
  idRecoveredValue GetTakedownName(...);
  int GetTauntInterval();
  idRecoveredValue GetUsableState(...);
  idRecoveredValue GetViewTransform(...);
  idRecoveredValue GetVoiceController(...);
  walkState_t GetWalkState() const;
  idRecoveredValue GetWeaponSlot(...);
  const idBounds* GetWorldCollisionBounds();
  idRecoveredValue GiveAwarenessOfEntity(...);
  idRecoveredValue HandleAffirmativeResponse(...);
  idRecoveredValue HandleLoot(...);
  idRecoveredValue HandlePendingAIEvents(...);
  idRecoveredValue HasFocus(...);
  idRecoveredValue HasSyncedWalks(...);
  idRecoveredValue HideMeshNoAutoRestore(...);
  idRecoveredValue InhibitRunning(...);
  idRecoveredValue InitAAS(...);
  idRecoveredValue InitAnimation(...);
  idRecoveredValue InitEncounterGroup(...);
  void InitFireControl();
  idRecoveredValue InitFSMs(...);
  idRecoveredValue InitHeadTracking(...);
  idRecoveredValue InitMovement(...);
  idRecoveredValue InitPerception(...);
  idRecoveredValue InitPhysics(...);
  idRecoveredValue InitScriptObjects(...);
  idRecoveredValue InteractionCameraDeactivateCallback(...);
  idRecoveredValue InternalCallEvent(...);
  idRecoveredValue InternalRespondsTo(...);
  idRecoveredValue IsAASBlocked(...);
  bool IsBlindFiring() const;
  idRecoveredValue IsBlockedByFriendly(...);
  idRecoveredValue IsCrosshairSubdued(...);
  idRecoveredValue IsDead(...);
  idRecoveredValue IsDowned(...);
  idRecoveredValue IsDying(...);
  idRecoveredValue IsFacingMe(...);
  bool IsIncapacitated() const;
  bool IsInCover() const;
  bool IsInsideTrailUpdateRange(const idVec3* suspiciousPosition) const;
  bool IsModesAllowed(aiModeRestricFlag_t flags) const;
  bool IsModesRestricted(aiModeRestricFlag_t flags) const;
  bool IsMoveDone(std::int64_t checkAlignment = 0) const;
  idRecoveredValue IsOnElevator(...);
  idRecoveredValue IsOverrideAnimSupported(...);
  bool IsPlayerControlled() const;
  idRecoveredValue IsRecentlyGroupSpotted(...);
  bool IsRecentlyHeard(const idAIEntityState* entityState, float time) const;
  bool IsRecentlySpawned(float seconds) const;
  bool IsRecentlySpotted(const idAIEntityState* entityState, float time) const;
  idRecoveredValue IsRecentlySpotted(idRecoveredOverloadTag<1>);
  idRecoveredValue IsTargetInMeleeRange(...);
  bool IsTeslaDangerous(idVec3* position, float* radius) const;
  bool IsThrowingGrenade() const;
  bool IsWeaponEquipped() const;
  idRecoveredValue JobSync(...);
  void KillAI();
  idRecoveredValue KilledNotification(...);
  idRecoveredValue LeaveActionNode(...);
  idRecoveredValue LKWBlind(...);
  idRecoveredValue LookDebug(...);
  idRecoveredValue MeleeMaskForTarget(...);
  idRecoveredValue ModifyCrosshairInfo(...);
  bool OkToLKWBlind() const;
  void OnActionScriptFinished(int currentTime);
  idRecoveredValue OnActivate(...);
  idRecoveredValue OnAIEvent(...);
  idRecoveredValue OnAIEvent_Body(...);
  idRecoveredValue OnAIEvent_DeadBodyLand(...);
  idRecoveredValue OnAIEvent_DeploySound(...);
  idRecoveredValue OnAIEvent_DoorMotion(...);
  idRecoveredValue OnAIEvent_Explosion(...);
  idRecoveredValue OnAIEvent_Footstep(...);
  idRecoveredValue OnAIEvent_Grenade(...);
  idRecoveredValue OnAIEvent_Gunfire(...);
  idRecoveredValue OnAIEvent_Investigate(...);
  idRecoveredValue OnAIEvent_PlayerAim(...);
  idRecoveredValue OnAIEvent_PlayerInteraction(...);
  idRecoveredValue OnAIEvent_Projectile(...);
  idRecoveredValue OnAIEvent_ProjectileImpact(...);
  idRecoveredValue OnAIEvent_Vehicle(...);
  idRecoveredValue OnAIEvent_VehicleSound(...);
  idRecoveredValue OnAIEvent_Voice(...);
  idRecoveredValue OnHide(...);
  idRecoveredValue OnIgnoreDamage(...);
  idRecoveredValue OnShow(...);
  bool PerceptionFlagIsSet(int flags);
  idRecoveredValue PerformCrouchAction(...);
  idRecoveredValue PerformOverrideAnimAction(...);
  idRecoveredValue PerformStandUpAction(...);
  idRecoveredValue PlayAdditiveAlarmAnimation(...);
  idRecoveredValue PlayAdditiveFireAnim(...);
  idRecoveredValue PlayAdditivePain(...);
  idRecoveredValue PlayAdditivePainAnim(...);
  idRecoveredValue PlayAnimWebPath(...);
  idRecoveredValue PlayAnimWebPathPerfect(...);
  idRecoveredValue PlayDeathSound(...);
  idRecoveredValue PlayHurtSound(...);
  idRecoveredValue PushFromCollision(...);
  idRecoveredValue RelinquishControl(...);
  idRecoveredValue ReloadWeapon(...);
  idRecoveredValue Remove(...);
  bool RespondsToAIEvent(const idAIEvent* event) const;
  idRecoveredValue RobotBatteryExplodes(...);
  idRecoveredValue SaveFSM(...);
  idRecoveredValue SendDeathTrigger(...);
  idRecoveredValue SendGrappleActivationTest(...);
  idRecoveredValue SetAccuracy(...);
  idRecoveredValue SetActionNodeGroupName(...);
  idRecoveredValue SetActionScript(...);
  void SetActionScript(const idList<idScriptAction, 5>* actions,
                       idEntity* source, idEntity* activator);
  void SetActionScriptFlag(int flags, bool setFlag);
  void SetAIClip();
  void SetAimFocusAimPoint(aimPoint_t point);
  idRecoveredValue SetAimFocusEntity(...);
  idRecoveredValue SetAlertCycle(...);
  idRecoveredValue SetAnimWebEvent(...);
  bool SetBlinded(bool state = true, bool forceForever = false,
                  int blindedOverrideTimeMilliseconds = -1);
  void SetBodyOrientation(const idMat3& axis);
  void SetCoverAction(coverAction_t action, int currentTime);
  idRecoveredValue SetDelayedVoice(...);
  idRecoveredValue SetEMPStunned(...);
  void SetEnableAutoFocus(bool enable);
  void SetEnableHeadTracking(bool enable);
  idRecoveredValue SetFakeEnemy(...);
  void SetFOV(const float* fieldOfView);
  void SetIdealAimFocusPoint(const idVec3& point, int timeout,
                             aiFocus_t focusType);
  void SetIdealLookFocusPoint(const idVec3& point, int timeout,
                              aiFocus_t focusType);
  void SetLockHeadTracking(bool lock);
  idRecoveredValue SetLookFocusEntity(...);
  idRecoveredValue SetModel(...);
  idRecoveredValue SetMovePushStatus(...);
  void SetPerceptionFlags(int flags, bool setFlag);
  void SetRobotChargingDone();
  void SetRobotPowerOut();
  void SetSleeping(bool sleep = true);
  idRecoveredValue SetSolid(...);
  void SetSpeakingVO();
  idRecoveredValue SetSubWeb(...);
  idRecoveredValue SetTakeDownWindow(...);
  idRecoveredValue SetupFakePosition(...);
  idRecoveredValue SetupRobotDead(...);
  void SetWalkState(walkState_t state);
  void SetWorldCollision(bool collideWithWorld);
  idRecoveredValue ShouldDeadThink(...);
  idRecoveredValue ShouldEnterDormancy(...);
  idRecoveredValue ShouldHoldFire(...);
  bool ShouldLeaveDormancy() const override;
  idRecoveredValue ShouldReloadWeapon(...);
  idRecoveredValue ShouldScramble(...);
  idRecoveredValue ShowFlareEffect(...);
  idRecoveredValue Spawn(...);
  idRecoveredValue SpawnCorpse(...);
  idRecoveredValue SpawnFlareEffect(...);
  idRecoveredValue SpawnTeslaBomb(...);
  idRecoveredValue StartActionScript(...);
  void StartActionScript(int gameTime);
  idRecoveredValue StartBodyDeathFade(...);
  idRecoveredValue StartDeathSystem(...);
  idRecoveredValue StartDisintigrateDeathFade(...);
  idRecoveredValue StartRagdoll(...);
  idRecoveredValue StartRagdoll(idRecoveredOverloadTag<1>);
  idRecoveredValue StartReloading(...);
  idRecoveredValue StartStaggeringPain(...);
  idRecoveredValue StartWeaponFX(...);
  void StopFireRobot(int currentTime);
  idRecoveredValue StopRagdoll(...);
  idRecoveredValue StopTesla(...);
  idRecoveredValue StopWeaponFX(...);
  idRecoveredValue Takedown(...);
  idRecoveredValue TakedownWindoIsOpen(...);
  idRecoveredValue Teleport(...);
  idRecoveredValue TeleportPush(...);
  bool TeslaIsActive() const;
  idRecoveredValue TestFireWeapon(...);
  idRecoveredValue TestForBlockedTrace(...);
  idRecoveredValue Think(...);
  idRecoveredValue Think_AIControl(...);
  idRecoveredValue Think_Impl(...);
  idRecoveredValue Think_PlayerControl(...);
  idRecoveredValue ThinkRobot(...);
  void TurretFire();
  idRecoveredValue TurretShouldFire(...);
  idRecoveredValue UnequipItem(...);
  idRecoveredValue UpdateActions(...);
  idRecoveredValue UpdateAimFocus(...);
  idRecoveredValue UpdateAnimationControllers(...);
  idRecoveredValue UpdateAttachments(...);
  idRecoveredValue UpdateAvoidCharacter(...);
  idRecoveredValue UpdateBodyOrientationWhileMoving(...);
  idRecoveredValue UpdateDeathMovement(...);
  idRecoveredValue UpdateDeaths(...);
  idRecoveredValue UpdateDrop(...);
  idRecoveredValue UpdateEffects(...);
  idRecoveredValue UpdateEnemyInAAS(...);
  idRecoveredValue UpdateExtendedClipModel(...);
  idRecoveredValue UpdateFSMs(...);
  idRecoveredValue UpdateGoreLevel(...);
  idRecoveredValue UpdateGrappleActivation(...);
  idRecoveredValue UpdateGroupStatusConditions(...);
  idRecoveredValue UpdateLookFocus(...);
  idRecoveredValue UpdateMeleeCapabilities(...);
  idRecoveredValue UpdateMovement(...);
  idRecoveredValue UpdatePathToEnemy(...);
  idRecoveredValue UpdateRenderParamBlends(...);
  idRecoveredValue UpdateSpriteAnimator(...);
  idRecoveredValue UpdateTesla(...);
  idRecoveredValue UpdateTravelFlags(...);
  idRecoveredValue UpdateWetness(...);
  idRecoveredValue UpdateWorldState(...);
  idRecoveredValue Use(...);
  idRecoveredValue UsesSmoothAnimations(...);
  bool UsesTurrets() const;
  idRecoveredValue WaitForAnimToEnd(...);
  idRecoveredValue WaitForAnimToStart(...);
  idRecoveredValue WaitForAnimToStartVia(...);
  idRecoveredValue WaitForAnimToStartVia(idRecoveredOverloadTag<1>);
  idRecoveredValue WaitForOverrideAnim(...);
  idRecoveredValue WaitForTraversal(...);

  void SetVoiceOverPlaying(bool value) { voiceOverPlaying = value; }
  bool IsVoiceOverPlaying() const { return voiceOverPlaying; }
  idAIVoiceController& GetLocalVoiceController() {
    return *localVoiceController;
  }
  idFaceMgr& GetFaceManager() { return *faceManager; }
  void SetPresentable(idPresentable* value) { presentable = value; }
  idPresentable* GetPresentable() const { return presentable; }
  idAIScriptProxy& GetScriptProxy() { return scriptProxy; }
  const idAIScriptProxy& GetScriptProxy() const { return scriptProxy; }
  const idAIAnimEventState& GetAnimEventState() const { return animEventState; }
  void SetMeleeWeapon(idWeapon* value) { meleeWeapon = value; }
  void SetEquippedWeapon(idWeapon* value) {
    meleeWeapon = value;
    idActor::SetEquippedWeapon(0, value);
  }
  void SetMeleeImpactEffects(const idDeclProjectileImpactEffect* value) {
    meleeImpactEffects = value;
  }
  idAIAnimWebRuntimeState& GetAnimWebState() { return animWebState; }
  const idAIAnimWebRuntimeState& GetAnimWebState() const { return animWebState; }
  idAIDebugState& GetDebugState() { return debugState; }
  const idAIDebugState& GetDebugState() const { return debugState; }
  idAIEventState& GetAIEventState() { return aiEventState; }
  const idAIEventState& GetAIEventState() const { return aiEventState; }
  const idList<idAIVolatile::idAIRenderParamBlend, 5>&
  GetRenderParameterBlends() const {
    return renderParameterBlends;
  }
  idAIScriptState& GetScriptState() { return scriptState; }
  const idAIScriptState& GetScriptState() const { return scriptState; }
  idAIUtilityState& GetUtilityState() { return utilityState; }
  const idAIUtilityState& GetUtilityState() const { return utilityState; }
  const idAICoreState& GetCoreState() const { return coreState; }
  idAIRobotRuntimeState& GetRobotState() { return robotState; }
  const idAIRobotRuntimeState& GetRobotState() const { return robotState; }
  void SetTurretUsage(bool animationSupportsTurrets,
                      bool actionSettingsUseTurrets) {
    turretAnimationSupported = animationSupportsTurrets;
    turretActionSetting = actionSettingsUseTurrets;
  }
  void SetMovementRestrictions(aiModeRestricFlag_t flags,
                               bool stationaryModeValue) {
    movementModeRestrictionFlags = flags;
    stationaryMode = stationaryModeValue;
  }
  void SetWalkStateLimits(walkState_t minimum, walkState_t maximum) {
    minimumWalkState = minimum;
    maximumWalkState = maximum;
  }
  void SetTeslaTimes(int dischargeTime, int explodeTime) {
    teslaDischargeTime = dischargeTime;
    teslaExplodeTime = explodeTime;
  }
  void ConfigureTeslaBlindFire(float blindFireDuration, int timeStamp) {
    teslaBlindFireDuration = blindFireDuration;
    teslaBlindFireTimeStamp = timeStamp;
  }
  void SetTeslaDangerConfiguration(bool heavyGuardGun, float range,
                                   const idDeclDamage* damageDecl) {
    teslaHeavyGuardGun = heavyGuardGun;
    teslaRange = range;
    teslaDamageDecl = damageDecl;
  }
  void SetTeslaExplodePosition(const idVec3& position) {
    teslaExplodePosition = position;
  }
  void SetTeslaEvaluationTime(int time) { teslaEvaluationTime = time; }
  int GetAIClipMask() const { return aiClipMask; }
  idAIFireControl* GetFireControl() const { return fireControl.get(); }
  void SetAccuracyState(int groupCondition, int editableAccuracy,
                        int skillAccuracy) {
    groupStatusCondition = groupCondition;
    editableAimAccuracy = editableAccuracy;
    skillAimAccuracy = skillAccuracy;
  }
  int GetLastFireTime() const { return lastFireTime; }
  void SetAIEvaluationTime(int time) { aiEvaluationTime = time; }
  void SetArticulatedFigureState(bool loaded, bool active,
                                 bool clientAuthoritativeActive) {
    articulatedFigureLoaded = loaded;
    articulatedFigureActive = active;
    articulatedFigureClientActive = clientAuthoritativeActive;
  }
  void SetArticulatedFigureTransform(const idVec3& origin,
                                     const idMat3& axis) {
    articulatedFigureOrigin = origin;
    articulatedFigureAxis = axis;
  }
  void SetModelOffset(const idVec3& offset) { modelOffset = offset; }
  void ConfigureDroneSoundTransform(bool usesDroneValue,
                                    const idVec3& origin) {
    usesDrone = usesDroneValue;
    dronePresentOrigin = origin;
  }
  void SetDeathImpulseIndex(int value) { deathImpulseIndex = value; }
  bool IsWalkIKEnabled() const { return walkIKEnabled; }
  bool AreDeathAnimatorsEnabled() const { return deathAnimatorsEnabled; }
  void ConfigureBlinding(bool enabled, float duration, float reblindDelay) {
    usesBlindness = enabled;
    blindedDuration = duration;
    reblindCooldown = reblindDelay;
  }
  void SetBlindingTimeState(int currentTime, int stunnedEndTime,
                            bool painAnimationDoneValue,
                            float incomingDamageScaleValue = 1.0F) {
    aiEvaluationTime = currentTime;
    endStunnedTime = stunnedEndTime;
    painAnimationDone = painAnimationDoneValue;
    incomingDamageScale = incomingDamageScaleValue;
  }
  int GetEndBlindedTime() const { return endBlindedTime; }
  int GetEndReblindCooldown() const { return endReblindCooldown; }
  void ConfigureSleeping(bool enabled) { usesSleeping = enabled; }
  void SetVoiceProfile(idAIVoiceProfile* value) { voiceProfile = value; }
  idAIVoiceProfile* GetVoiceProfile() const { return voiceProfile; }
  void SetVoiceGroupController(idAIVoiceController* value) {
    voiceGroupController = value;
  }
  idAIVoiceController* GetVoiceGroupController() const {
    return voiceGroupController;
  }
  const idStr& GetScriptedActionName() const { return scriptedActionName; }
  idEntity* GetScriptedActionSource() const { return scriptedActionSource; }
  idEntity* GetScriptedActionActivator() const {
    return scriptedActionActivator;
  }
  int GetScriptedActionState() const { return scriptedActionState; }
  int GetScriptedActionStartTime() const { return scriptedActionStartTime; }
  int GetScriptedActionStartCount() const { return scriptedActionStartCount; }
  void SetFireControlWeapon(int slot, idWeapon* weapon) {
    if (slot >= 0 && slot < static_cast<int>(fireControlWeapons.size())) {
      fireControlWeapons[slot] = weapon;
      if (weapon != nullptr) weapon->SetEquipSlot(slot);
    }
  }
  idWeapon* GetFireControlWeapon(int slot) const {
    return slot >= 0 && slot < static_cast<int>(fireControlWeapons.size())
               ? fireControlWeapons[slot]
               : nullptr;
  }
  void SetFireTemporaryHold(bool value) { fireTemporaryHold = value; }
  bool HasFireTemporaryHold() const { return fireTemporaryHold; }
  void SetFireActionScriptRunning(bool value) {
    fireActionScriptRunning = value;
  }
  bool IsFireActionScriptRunning() const { return fireActionScriptRunning; }
  void SetFireShouldHold(bool value) { fireShouldHold = value; }
  bool ShouldHoldFireState() const { return fireShouldHold; }
  void SetFireMissTime(float value) { fireMissTime = value; }
  float GetFireMissTime() const { return fireMissTime; }
  void SetSecondaryFireRange(float value) { secondaryFireRange = value; }
  float GetSecondaryFireRange() const { return secondaryFireRange; }
  void SetCurrentInteraction(idEntity* value) { currentInteraction = value; }
  idEntity* GetCurrentInteraction() const { return currentInteraction; }
  void StartInteractionAction(const char* scriptName, idEntity* ownerEntity,
                              idEntity* actionActivator, int time) {
    interactionActionScript = scriptName;
    interactionOwner = ownerEntity;
    interactionActivator = actionActivator;
    interactionActionStartTime = time;
    ++interactionActionCount;
  }
  const idStr& GetInteractionActionScript() const {
    return interactionActionScript;
  }
  int GetInteractionActionCount() const { return interactionActionCount; }
  int GetInteractionActionStartTime() const {
    return interactionActionStartTime;
  }
  int GetDefaultMovementMode() const;
  void SetMoveFocusEntity(idEntity* value) { moveFocusEntity = value; }
  idEntity* GetMoveFocusEntity() const { return moveFocusEntity; }
  void SetExplicitMoveFocusPoint(const idVec3& value, bool enabled) {
    explicitMoveFocusPoint = value;
    hasExplicitMoveFocusPoint = enabled;
  }
  bool HasExplicitMoveFocusPoint() const { return hasExplicitMoveFocusPoint; }
  const idVec3& GetExplicitMoveFocusPoint() const {
    return explicitMoveFocusPoint;
  }
  float GetBaseHealth() const {
    return static_cast<float>(animEventState.health);
  }
  void SetBaseHealth(float value) {
    animEventState.health = static_cast<int>(value);
    animEventState.dead = animEventState.health <= 0;
  }
  bool IsDeadState() const { return animEventState.dead; }
  bool IsDyingState() const { return volumeDying; }
  void SetDyingState(bool value) { volumeDying = value; }
  void ApplyVolumeDamage(idEntity* damageInflictor, idEntity* damageAttacker,
                         const idDeclDamage* damageDeclaration) {
    lastVolumeInflictor = damageInflictor;
    lastVolumeAttacker = damageAttacker;
    lastVolumeDamageDecl = damageDeclaration;
    animEventState.health = 0;
    animEventState.dead = true;
  }
  idEntity* GetLastVolumeInflictor() const { return lastVolumeInflictor; }
  idEntity* GetLastVolumeAttacker() const { return lastVolumeAttacker; }
  const idDeclDamage* GetLastVolumeDamageDecl() const {
    return lastVolumeDamageDecl;
  }
  bool UseTauntChatterVO();
  void SetEncounterEnemy(idEntity* value) { encounterEnemy = value; }
  idEntity* GetEncounterEnemy() const { return encounterEnemy; }
  void SetEncounterAlertCycle(int value) { encounterAlertCycle = value; }
  int GetEncounterAlertCycle() const { return encounterAlertCycle; }
  void SetEncounterRole(encounterGroupRole_t value) { encounterRole = value; }
  encounterGroupRole_t GetEncounterRole() const { return encounterRole; }
  void SetEncounterActive(bool value) { encounterActive = value; }
  bool IsEncounterActive() const { return encounterActive; }
  void SetAIEventEncounterGroup(idEncounterGroup* value) {
    aiEventEncounterGroup = value;
  }
  const void* GetAIGroupIdentity() const override {
    return aiEventEncounterGroup;
  }
  void SetEncounterLastSeenTime(int value) { encounterLastSeenTime = value; }
  int GetEncounterLastSeenTime() const { return encounterLastSeenTime; }
  int GetTauntUseCount() const { return tauntUseCount; }
  void ConfigureTaunt(int dodgeFlags, float minimumInterval,
                      float maximumInterval, int nextTauntTime) {
    tauntDodgeFlags = dodgeFlags;
    tauntMinimumInterval = minimumInterval;
    tauntMaximumInterval = maximumInterval;
    nextAITauntTime = nextTauntTime;
  }
  static void SetSharedAIRandomSeed(unsigned int seed) {
    sharedAIRandomSeed = seed;
  }
  static unsigned int GetSharedAIRandomSeed() { return sharedAIRandomSeed; }
  aimPoint_t GetAimFocusAimPoint() const { return aimFocusAimPoint; }
  void ConfigureTrailRange(bool usesCloseValue, float closeMinimumRange) {
    usesCloseCover = usesCloseValue;
    closeMinRange = closeMinimumRange;
  }
  void SetFirstThinkTimestamp(int timestamp) {
    firstThinkTimestamp = timestamp;
  }
  void ConfigureFireRanges(float maximumFireRange, float goNutsRangeValue,
                           float currentEnemyRangeValue) {
    maxFireRange = maximumFireRange;
    goNutsRange = goNutsRangeValue;
    currentEnemyRange = currentEnemyRangeValue;
  }
  float GetFieldOfViewDot() const { return fieldOfViewDot; }
  bool IsAutoFocusEnabled() const { return autoFocusEnabled; }
  bool IsHeadTrackingLocked() const { return headTrackingLocked; }
  const idFocusInfo* GetFocusInfo(int index) const {
    return index >= 0 && index < static_cast<int>(focusInfos.size())
               ? focusInfos[static_cast<std::size_t>(index)].get()
               : nullptr;
  }
  idFocusInfo* GetFocusInfo(int index) {
    return index >= 0 && index < static_cast<int>(focusInfos.size())
               ? focusInfos[static_cast<std::size_t>(index)].get()
               : nullptr;
  }
  int GetLookFocusTimeout() const { return lookFocusTimeout; }
  void SetActionScriptRunning(bool running) { actionScriptRunning = running; }
  void ConfigureSpeakingVoice(bool mutant, bool mood, bool lipSync,
                              bool gesture, bool eyeTracking,
                              bool blinkingValue, bool lidDeformationValue) {
    mutantActorClass = mutant;
    speakingMoodCapability = mood;
    speakingLipSyncCapability = lipSync;
    speakingGestureCapability = gesture;
    speakingEyeTrackingCapability = eyeTracking;
    speakingBlinkingCapability = blinkingValue;
    speakingLidDeformationCapability = lidDeformationValue;
  }
  void ConfigureCurrentScenePoint(idScenePoint* value, int activationTime) {
    currentScenePoint = value;
    currentScenePointActivationTime = activationTime;
  }
  void SetDefaultPhysicsClipMask(int value) { defaultPhysicsClipMask = value; }
  void SetNextAIEventTime(int value) { nextAIEventTime = value; }
  void SetAnimWebEventPending(bool value) { animWebEventPending = value; }
  void ConfigureGrenadeThrowState(bool usesGrenadesValue,
                                  const void* itemDeclaration,
                                  int overrideAnimation) {
    usesGrenades = usesGrenadesValue;
    grenadeItemDeclaration = itemDeclaration;
    grenadeOverrideAnimation = overrideAnimation;
  }
  void ConfigurePlayerViewData(float range, int spottedTime, float dot) {
    playerViewRange = range;
    playerViewSpottedTime = spottedTime;
    playerViewDot = dot;
  }
  void ConfigureDefaultMovementSpeeds(float minimum, float maximum) {
    minimumMovementSpeed = minimum;
    maximumMovementSpeed = maximum;
  }
  void ConfigureFireSpotTimes(float lost, float gained) {
    spotLostFireTime = lost;
    spotGainedFireTime = gained;
  }
  void SetControllingPlayer(idPlayer* value) { controllingPlayer = value; }
  void SetCurrentEnemy(idEntity* value) { currentEnemy = value; }
  void ConfigureFacialAnimationSets(
      const idDeclFacialAnimationSet* current,
      const idDeclFacialAnimationSet* defaultSet) {
    currentFacialAnimationSet = current;
    defaultFacialAnimationSet = defaultSet;
  }
  void ConfigureSyncedAnimation(int overrideAnimation, int targetSpawnId) {
    syncedOverrideAnimation = overrideAnimation;
    syncedTargetSpawnId = targetSpawnId;
  }
  int GetSyncedOverrideAnimation() const { return syncedOverrideAnimation; }
  int GetSyncedTargetSpawnId() const { return syncedTargetSpawnId; }
  void SetActionScriptCount(int value) { actionScriptCount = value; }
  int GetActionScriptCount() const { return actionScriptCount; }
  bool IsActionScriptRunning() const { return actionScriptRunning; }
  bool IsPainAllowedAfterActionScript() const {
    return painAllowedAfterActionScript;
  }
  void SetBehaviorDeclaration(const idDeclAIBehavior* value) {
    behaviorDeclaration = value;
  }

private:
  eventVoid* RecordAnimEvent(const char* eventName, eventVoid* result);
  eventVoid* RecordGameEvent(const char* eventName, eventVoid* result);
  idRecoveredValue RecordCoreCall(const char* methodName);

  idAIScriptProxy scriptProxy;
  bool voiceOverPlaying{false};
  std::unique_ptr<idAIVoiceController> localVoiceController;
  std::unique_ptr<idFaceMgr> faceManager;
  std::unique_ptr<idAIOrientation> bodyOrientation;
  idPresentable* presentable{nullptr};
  idAIAnimEventState animEventState;
  idWeapon* meleeWeapon{nullptr};
  const idDeclProjectileImpactEffect* meleeImpactEffects{nullptr};
  idAIAnimWebRuntimeState animWebState;
  idAIDebugState debugState;
  idAIEventState aiEventState;
  idList<idAIVolatile::idAIRenderParamBlend, 5> renderParameterBlends;
  idAIScriptState scriptState;
  idAIUtilityState utilityState;
  idAICoreState coreState;
  idAIRobotRuntimeState robotState;
  bool volumeDying{false};
  idEntity* lastVolumeInflictor{nullptr};
  idEntity* lastVolumeAttacker{nullptr};
  const idDeclDamage* lastVolumeDamageDecl{nullptr};
  std::array<idWeapon*, 2> fireControlWeapons{{nullptr, nullptr}};
  bool fireTemporaryHold{false};
  bool fireActionScriptRunning{false};
  bool fireShouldHold{false};
  float fireMissTime{-1.0F};
  float secondaryFireRange{-1.0F};
  idEntity* currentInteraction{nullptr};
  idStr interactionActionScript;
  idEntity* interactionOwner{nullptr};
  idStr scriptedActionName;
  idEntity* scriptedActionSource{nullptr};
  idEntity* scriptedActionActivator{nullptr};
  int scriptedActionState{-1};
  int scriptedActionStartTime{-1};
  int scriptedActionStartCount{0};
  idAIVoiceProfile* voiceProfile{nullptr};
  idAIVoiceController* voiceGroupController{nullptr};
  idEntity* interactionActivator{nullptr};
  int interactionActionStartTime{-1};
  int interactionActionCount{0};
  idEntity* moveFocusEntity{nullptr};
  idVec3 explicitMoveFocusPoint{};
  bool hasExplicitMoveFocusPoint{false};
  idEntity* encounterEnemy{nullptr};
  int encounterAlertCycle{0};
  int encounterLastSeenTime{-1};
  encounterGroupRole_t encounterRole{ROLE_NONE};
  bool encounterActive{true};
  idEncounterGroup* aiEventEncounterGroup{nullptr};
  int tauntUseCount{0};
  int grappleActivationCheckTimer{-1};
  bool aimFocusActive{false};
  bool turretAnimationSupported{false};
  bool turretActionSetting{false};
  aiModeRestricFlag_t movementModeRestrictionFlags{0};
  bool stationaryMode{false};
  walkState_t minimumWalkState{WALKSTATE_DEFAULT};
  walkState_t maximumWalkState{WALKSTATE_RUNNING};
  int aiClipMask{0x400};
  int teslaDischargeTime{-1};
  int teslaExplodeTime{-1};
  float teslaBlindFireDuration{0.0F};
  int teslaBlindFireTimeStamp{0};
  bool teslaHeavyGuardGun{false};
  float teslaRange{0.0F};
  const idDeclDamage* teslaDamageDecl{nullptr};
  idVec3 teslaExplodePosition{};
  int teslaEvaluationTime{0};
  bool usesBlindness{true};
  float blindedDuration{-1.0F};
  float reblindCooldown{-1.0F};
  int endBlindedTime{0};
  int endReblindCooldown{-1};
  int endStunnedTime{0};
  int aiEvaluationTime{0};
  float incomingDamageScale{1.0F};
  bool painAnimationDone{true};
  bool usesSleeping{true};
  std::unique_ptr<idAIFireControl> fireControl;
  int groupStatusCondition{0};
  int editableAimAccuracy{0};
  int skillAimAccuracy{0};
  int lastFireTime{-1};
  bool articulatedFigureLoaded{false};
  bool articulatedFigureActive{false};
  bool articulatedFigureClientActive{false};
  idVec3 articulatedFigureOrigin{};
  idMat3 articulatedFigureAxis{};
  idVec3 modelOffset{};
  bool usesDrone{false};
  idVec3 dronePresentOrigin{};
  int deathImpulseIndex{-1};
  bool walkIKEnabled{true};
  bool deathAnimatorsEnabled{true};
  int tauntDodgeFlags{0};
  float tauntMinimumInterval{0.0F};
  float tauntMaximumInterval{0.0F};
  int nextAITauntTime{0};
  aimPoint_t aimFocusAimPoint{AIMPOINT_EYELEVEL};
  inline static unsigned int sharedAIRandomSeed{0};
  bool usesCloseCover{false};
  float closeMinRange{0.0F};
  int firstThinkTimestamp{0};
  float maxFireRange{0.0F};
  float goNutsRange{0.0F};
  float currentEnemyRange{0.0F};
  float fieldOfViewDot{0.0F};
  bool autoFocusEnabled{false};
  int lookFocusTimeout{-1};
  int actionScriptFlags{0};
  bool actionScriptRunning{false};
  bool mutantActorClass{false};
  bool speakingMoodCapability{false};
  bool speakingLipSyncCapability{false};
  bool speakingGestureCapability{false};
  bool speakingEyeTrackingCapability{false};
  bool speakingBlinkingCapability{false};
  bool speakingLidDeformationCapability{false};
  bool headTrackingLocked{false};
  std::array<std::unique_ptr<idFocusInfo>, 2> focusInfos;
  std::unique_ptr<idAIMoveState> moveState;
  idScenePoint* currentScenePoint{nullptr};
  int currentScenePointActivationTime{0};
  const idDeclAIBehavior* behaviorDeclaration{nullptr};
  int defaultPhysicsClipMask{0};
  int nextAIEventTime{0};
  bool animWebEventPending{false};
  std::unique_ptr<idLegsFSM> actionFSM;
  bool usesGrenades{false};
  const void* grenadeItemDeclaration{nullptr};
  int grenadeOverrideAnimation{0};
  float playerViewRange{0.0F};
  int playerViewSpottedTime{0};
  float playerViewDot{0.0F};
  float minimumMovementSpeed{0.0F};
  float maximumMovementSpeed{0.0F};
  float spotLostFireTime{0.0F};
  float spotGainedFireTime{0.0F};
  idPlayer* controllingPlayer{nullptr};
  idEntity* currentEnemy{nullptr};
  const idDeclFacialAnimationSet* currentFacialAnimationSet{nullptr};
  const idDeclFacialAnimationSet* defaultFacialAnimationSet{nullptr};
  int syncedOverrideAnimation{0};
  int syncedTargetSpawnId{0x1FFF};
  int actionScriptCount{0};
  bool painAllowedAfterActionScript{true};
};

extern idCVar ai_forceWalk;
extern idCVar ai_enableDormancy;
