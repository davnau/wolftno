#pragma once

#include "tungsten/game/ai/aitypes.h"
#include "tungsten/game/ai/aivisibility.h"
#include "tungsten/game/ai/corpsemanager.h"
#include "tungsten/game/animation/ponytail.h"
#include "tungsten/game/game_runtime.h"
#include "tungsten/game/declvoiceover.h"
#include "engine/cm/collisionmodel.h"
#include "engine/animation/md6.h"
#include "engine/decls/animwebs/declanimweb.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/containers/staticlist.h"
#include "shared/idlib/text/atomicstring.h"
#include "shared/idlib/text/str.h"

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

struct idScriptAction;
enum fxCondition_t : int;
class idAIVoiceController;
class idAI2;
class idAIOrientation;
class idFaction;
class idFaceMgr;
class idPresentable;
class idAnimWebPath;
class idAnimStack;
class idMD6Anim;
class idWeapon;
class idInventoryItem;
class idProxyMove;
class idAIFireControl;
class idAIMoveState;
class idFocusInfo;
class idAIEvent;
class idAIEventSphere;
class idAIEventProjectedSphere;
class idAIEvent_Vehicle;
class idAIEventSound;
class idAIEventVoice;
class idDeclAiEvent;
class idFiniteStateMachine;
class idFSMManager;
class idAIActionManager;
class idAIFSMCallback;
class idFile_String;
class idLegsFSM;
class idDeclFacialAnimationSet;
class idDeclDamage;
class idVolume_CustomDeath;
struct idAIVoiceProfile;
struct idDeclAIBehavior;
enum aiMelee_t : int;
enum invalidAliasHandle_t : int;
template <typename T, typename InvalidEnum, T InvalidValue>
struct idHandle;
using aiAnimationHandle_t =
    idHandle<unsigned short, invalidAliasHandle_t, 65535>;
class idInteractionCamera;
class idAIEntityState;
class aiSenseState_t;
class idAIWorldState;
class idTargetList;
class idTarget_FakeEnemy;
class idCheckSurroundingsState;
class idAICamera_Control;
class idFocusTrace;
class idAASPositionMgr;
class idDeclRenderParm;
class idLookupTable;
class idAIHealth;
class idAIDamageInfo;
class idAIDeathInfo;
struct idDamageImpulse;
class idComponentActionScript;
class idFormationCoverMgr;
class idGoreComponent;
class idSpawnFocusWithRequirement;
class idDeclEntityDef;
class idAnimator_AnimWeb;
class idAnimator_Drone;
class idFXManager;
class idTreeAnimator;
class idAnimator_Channel;
struct idSoundShader;
class idAnimatedEntity;
extern idCVar g_showTrajectories;
extern idCVar m_sensitivity;
extern idCVar in_invertLook;
extern idCVar joy_pitchSpeed;
extern idCVar joy_yawSpeed;
extern idCVar ai_debugAim;
extern idCVar ai_debugLook;
extern idCVar ai_debugMissTime;
extern idCVar ai_debugLevel;
extern idCVar ai_debugPain;
extern idCVar ai_allowStaggerPain;
extern idCVar ai_forceInjury;
extern idCVar ai_searchToStimDist;
extern idCVar ai_debugOrientation;
extern idCVar ai_debugPerception;
extern idCVar ai_debugDodge;
extern idCVar ai_showFOV;
extern idCVar ai_enableAutoFocus;
extern idCVar ai_useAimTracker;
extern idCVar ai_useLookTracker;
extern idCVar ai_debugHealth;
extern idCVar ai_debugArmorPopOff;
class idRagdollInfo {
public:
  int velocityTime{0};
  float slomoStart{0.0F};
  float slomoEnd{0.0F};
  float jointFrictionDent{0.0F};
  float jointFrictionStart{0.0F};
  float jointFrictionEnd{0.0F};
  float contactFrictionDent{0.0F};
  float contactFrictionStart{0.0F};
  float contactFrictionEnd{0.0F};
  float animationVelocityScale{0.0F};
  bool useOverrideVelocity{false};
  idVec3 overrideVelocity{};
};
static_assert(sizeof(idRagdollInfo) == 56, "idRagdollInfo ABI drift");

class idAnimator_AF {
public:
  enum testSolidResult_t {
    TESTSOLID_NOT_IN_SOLID = 0,
    TESTSOLID_IN_SOLID_NOCLIP = 1,
    TESTSOLID_IN_SOLID = 2,
    TESTSOLID_MAX = 3,
  };
  void ConfigureSyncBlockers(bool first, bool second) {
    syncBlockerA = first;
    syncBlockerB = second;
  }
  bool CanBeginSync() const { return !syncBlockerA && !syncBlockerB; }
  void BeginSync(int gameTime) {
    isSyncing = true;
    lastBeginSyncTime = gameTime;
    ++beginSyncCount;
  }
  void EndSync() {
    isSyncing = false;
    ++endSyncCount;
  }
  bool IsSyncing() const { return isSyncing; }
  void SetSyncing(bool value) { isSyncing = value; }
  int GetLastBeginSyncTime() const { return lastBeginSyncTime; }
  int GetBeginSyncCount() const { return beginSyncCount; }
  int GetEndSyncCount() const { return endSyncCount; }
  void SetLoadedForTest(bool value) { loaded = value; }
  bool IsLoaded() const { return loaded; }
  void SetActiveForTest(bool activeValue, bool clientActiveValue) {
    active = activeValue;
    clientAuthoritativeActive = clientActiveValue;
  }
  bool IsActive() const { return active; }
  bool IsClientAuthoritativeActive() const {
    return clientAuthoritativeActive;
  }
  void SetOriginForTest(const idVec3& value) { origin = value; }
  const idVec3& GetOrigin() const { return origin; }
  void SetGravity(const idVec3& value) {
    gravity = value;
    ++setGravityCount;
  }
  const idVec3& GetGravityForTest() const { return gravity; }
  int GetSetGravityCountForTest() const { return setGravityCount; }
  void UnattachBones(const idVec3& direction) {
    lastUnattachDirection = direction;
    ++unattachBonesCount;
  }
  int GetUnattachBonesCountForTest() const { return unattachBonesCount; }
  const idVec3& GetLastUnattachDirectionForTest() const {
    return lastUnattachDirection;
  }
  void StartFromSaved(const idList<AFBodyPart_t, 5>& bodies) {
    (void)bodies;
  }
  void PutToRest() {}

private:
  bool syncBlockerA{false};
  bool syncBlockerB{false};
  bool isSyncing{false};
  int lastBeginSyncTime{-1};
  int beginSyncCount{0};
  int endSyncCount{0};
  bool loaded{false};
  bool active{false};
  bool clientAuthoritativeActive{false};
  idVec3 origin{};
  idVec3 gravity{};
  idVec3 lastUnattachDirection{};
  int setGravityCount{0};
  int unattachBonesCount{0};
};
class idAttachmentCollection {
public:
  void HideAttachments() const {}
  void AddForGoreTest(class idAttachment* attachment);
  void ClearForGoreTest();
  int NumForAI() const;
  class idAttachment* ForIndexForAI(int index) const;
};

class idAttachment {
public:
  void ConfigureForGoreTest(idInventoryItem* itemValue,
                            short jointIndexValue, int handleValue,
                            bool recoveredWeaponFlagValue = true) {
    item = itemValue;
    jointIndex = jointIndexValue;
    handle = handleValue;
    recoveredWeaponFlag = recoveredWeaponFlagValue;
  }
  idInventoryItem* GetItemForAI() const { return item; }
  short GetJointIndexForAI() const { return jointIndex; }
  int GetHandleForAI() const { return handle; }
  void ClearRecoveredWeaponFlagForAI() {
    recoveredWeaponFlag = false;
  }
  bool GetRecoveredWeaponFlagForTest() const {
    return recoveredWeaponFlag;
  }

private:
  idInventoryItem* item{nullptr};
  short jointIndex{-1};
  int handle{-1};
  bool recoveredWeaponFlag{true};
};
struct idStrId {
  unsigned int id{0};
};
struct idJointIndex;
template <typename T>
class idCodeResource;
enum voiceMsg_t : int;
enum voiceController_t : int;
enum aiSense_t : int;

template <typename T>
class idEntityPtr {
public:
  idEntityPtr() = default;
  explicit idEntityPtr(T* value) : entity(value) {}

  void Set(T* value) { entity = value; }
  operator T*() const;
  T* GetEntity() const;
  T* operator->() const;
  T* PeekForTest() const { return entity; }

private:
  T* entity{nullptr};
};

class idDeclInventory {
public:
  const char* declTypeName{"inventoryItem"};
  bool resourceValid{true};
};

class idDisassembleLoot {};

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
enum aiMovePushStatus_t {
  AIMOVEPUSHSTATUS_PUSH_NONE = 0,
  AIMOVEPUSHSTATUS_PUSH_OTHER_NPCS = 1,
  AIMOVEPUSHSTATUS_PUSH_PLAYERS = 2,
  AIMOVEPUSHSTATUS_PUSH_PLAYERS_AND_NPCS = 3,
  AIMOVEPUSHSTATUS_MAX = 4,
};
enum posture_t {
  POSTURE_NONE = 0,
  POSTURE_STANDING = 1,
  POSTURE_STAND = POSTURE_STANDING,
  POSTURE_CROUCHED = 2,
  POSTURE_CROUCH = POSTURE_CROUCHED,
  POSTURE_SITTING = 3,
};
enum alertCycle_t {
  ALERTCYCLE_NONE = 0,
  ALERTCYCLE_RELAXED = 1,
  ALERTCYCLE_SEARCH = 2,
  ALERTCYCLE_COMBAT = 3,
  ALERTCYCLE_INCAPACITATED = 4,
  ALERTCYCLE_DYING = 5,
  ALERTCYCLE_DEAD = 6,
  ALERTCYCLE_RESERVED_7 = 7,
  ALERTCYCLE_RESERVED_8 = 8,
  ALERTCYCLE_RESERVED_9 = 9,
  ALERTCYCLE_RESERVED_10 = 10,
  ALERTCYCLE_MAX = 11,
};

struct idDeclAIBehaviorEvents {
  const idDeclAiEvent* memberKilledVOEventDecl{nullptr};
  const idDeclAiEvent* losingBattleVOEventDecl{nullptr};
  const idDeclAiEvent* heardSomethingVoiceEventDecl{nullptr};
};
enum actorClass_t {
  ACTORCLASS_PLAYER = 0,
  ACTORCLASS_HUMAN = 1,
  ACTORCLASS_MUTANT = 2,
  ACTORCLASS_ROBOT = 3,
  ACTORCLASS_MAX = 4,
};
enum aiDirection_t {
  AIDIR_FRONT = 0,
  AIDIR_FRONT_LEFT = 1,
  AIDIR_LEFT = 2,
  AIDIR_BEHIND_LEFT = 3,
  AIDIR_BEHIND = 4,
  AIDIR_BEHIND_RIGHT = 5,
  AIDIR_RIGHT = 6,
  AIDIR_FRONT_RIGHT = 7,
  AIDIR_MAX = 8,
};
enum leanState_t {
  LEANSTATE_NONE = 0,
  LEANSTATE_LEFT = 1,
  LEANSTATE_RIGHT = 2,
};
enum weaponReadyState_t {
  WEAPON_READY_NONE = 0,
  WEAPON_READY_LOWERED = 1,
  WEAPON_READY_RAISED = 2,
};
enum equipSlot_t : int { EQUIPSLOT_PRIMARY = 0, EQUIPSLOT_SECONDARY = 1 };
enum handHideReason_t : int {
  HAND_HIDE_GENERIC = 0,
  HAND_HIDE_VEHICLE = 1,
};

enum coverAction_t {
  COVERACTION_NONE = 0,
  COVERACTION_BLINDFIRE_OVER = 6,
  COVERACTION_BLINDFIRE_LEFT = 7,
  COVERACTION_BLINDFIRE_RIGHT = 8,
  COVERACTION_HIDE = 12,
  COVERACTION_FLINCH = 17,
};
enum coverOrientation_t {
  COVERORIENT_NONE = 0,
  COVERORIENT_BACKWARD = 3,
};
enum damageDirection_t : int {
  DAMAGEDIR_NONE = 0,
  DAMAGEDIR_FRONT = 1,
  DAMAGEDIR_BACK = 2,
  DAMAGEDIR_LEFT = 3,
  DAMAGEDIR_RIGHT = 4,
  DAMAGEDIR_FRONTLEFT = 5,
  DAMAGEDIR_FRONTRIGHT = 6,
  DAMAGEDIR_BACKLEFT = 7,
  DAMAGEDIR_BACKRIGHT = 8,
  DAMAGEDIR_BLINDED = 9,
  DAMAGEDIR_MAX = 10,
};
enum damageIntensity_t : int {
  DAMAGEINTENSITY_NONE = 0,
  DAMAGEINTENSITY_LIGHT = 1,
  DAMAGEINTENSITY_MEDIUM = 2,
  DAMAGEINTENSITY_HEAVY = 3,
  DAMAGEINTENSITY_MAX = 4,
};
enum painType_t : int {
  PAIN_NONE = 0,
  PAINTYPE_NONE = PAIN_NONE,
  PAIN_STAGGER = 1,
  PAIN_INJURED = 2,
  PAIN_DEATH = 3,
  PAIN_BLINDED = 4,
  PAIN_MAX = 5,
};
enum aiAnimWait_t { AIANIMWAIT_DONT_WAIT = 0, AIANIMWAIT_WAIT = 1 };
enum walkState_t : int {
  WALKSTATE_DEFAULT = 0,
  WALKSTATE_WALKING = 1,
  WALKSTATE_RUNNING = 2,
};
enum aiFoci_t { AIFOCI_AIM, AIFOCI_LOOK, AIFOCI_NONE };
enum aiFocus_t : int {
  AIFOCUS_NOTHING = 0,
  AIFOCUS_ENTITY = 1,
  AIFOCUS_CAMERA = 2,
  AIFOCUS_LAST_SEEN_POS = 3,
  AIFOCUS_POS_WHEN_LAST_SEEN = 4,
  AIFOCUS_LAST_SENSED_POS = 5,
  AIFOCUS_POS_WHEN_LAST_SENSED = 6,
  AIFOCUS_ENEMY_APPROACH_DIR = 7,
  AIFOCUS_PARTIAL_SENSE = 8,
  AIFOCUS_FRIENDLY = 9,
  AIFOCUS_FRIENDLY_FOCUS = 10,
  AIFOCUS_MOVE_DEST = 11,
  AIFOCUS_LOOK_AROUND = 12,
  AIFOCUS_SUPPRESSION_FIRE_POINT = 13,
  AIFOCUS_MISC = 14,
};
enum usableState_t : int {
  USABLE_NONE = 0,
  USABLE_DISABLED = 1,
  USABLE_DISABLED_NO_ICON = 2,
  USABLE_LOOT = 3,
  USABLE_AI_TALK = 5,
  USABLE_AI_TOWN = 6,
  USABLE_DISASSEMBLE = 13,
  USABLE_INFO = 15,
};
enum takedownMode_t {
  TAKEDOWNMODE_NONE = 0,
  TAKEDOWNMODE_STEALTH = 1,
  TAKEDOWNMODE_COMBAT = 2,
  TAKEDOWNMODE_STUNNED = 3,
  TAKEDOWNMODE_CUSTOM = 4,
  TAKEDOWNMODE_COUNTER = 5,
  TAKEDOWNMODE_GROUND = 6,
};
struct takedownTypes_t {
  idList<idStr, 5> stealthSWFWD;
  idList<idStr, 5> stealthSWREV;
  idList<idStr, 5> stealthDWFWD;
  idList<idStr, 5> stealthDWREV;
  idList<idStr, 5> counterSWFWD;
  idList<idStr, 5> counterSWREV;
  idList<idStr, 5> counterDWFWD;
  idList<idStr, 5> counterDWREV;
  idList<idStr, 5> stunnedSWFWD;
  idList<idStr, 5> stunnedSWREV;
  idList<idStr, 5> customGeneric;
  idList<idStr, 5> groundFront;
  idList<idStr, 5> groundRear;
  idList<idStr, 5> groundLeft;
  idList<idStr, 5> groundRight;
  idList<idStr, 5> AITakedowns;
};
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
enum aiAnimWeb_t { AIANIMWEB_BODY = 0, AIANIMWEB_HEAD = 1 };
enum animWebEvent_t {
  ANIMWEB_EVENT_DEST_CLAMPED = 0,
  ANIMWEB_EVENT_START_BLEND_IN = 1,
};
using aiModeRestricFlag_t = int;
enum aiRenderParam_t {
  AIRENDERPARAM_ROBOTGLOW = 0,
  AIRENDERPARAM_SELFDESTRUCTGLOW = 1,
  AIRENDERPARAM_INVALID = 2,
};
enum aiAccuracy_t {
  ACCURACY_FROMSKILL = 0,
  ACCURACY_1 = 1,
  ACCURACY_2 = 2,
  ACCURACY_3 = 3,
  ACCURACY_4 = 4,
  ACCURACY_5 = 5,
  ACCURACY_6 = 6,
};

bool OkToFocus(const idVec3* to, const idVec3* from,
               const idVec3* actualPosition);

struct idAIScriptEventArg {
  enum type_t { ENTITY, INTEGER, FLOAT, VECTOR, QUATERNION, STRING, DECL };

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
  static idAIScriptEventArg Quaternion(const idQuat& value) {
    idAIScriptEventArg arg;
    arg.type = QUATERNION;
    arg.quaternion = value;
    return arg;
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
  idQuat quaternion{};
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
  void BindOwner(idAI2* value);
  bool ForceFSMIntoExecutingScriptForTest() {
    return ForceFSMIntoExecutingScript();
  }
  void SetExecutingScriptStateForTest(bool executing);
  idAI2* GetOwnerForTest() const;
  idRecoveredValue InternalCallEvent(...);
  idRecoveredValue PauseCurrentFiberTillActionCompletes(...);
  idRecoveredValue PauseCurrentThreadTillActionCompletes(...);
  idRecoveredValue PauseFiberTillActionCompletes(...);
  idRecoveredValue PauseThreadTillActionCompletes(...);
  bool blocking{false};

private:
  bool ForceFSMIntoExecutingScript();
  invocation_t lastInvocation;
};

template <int Index>
struct idRecoveredOverloadTag {};

struct idFirstPersonEvent {
  int eventId{0};
};

class idActor : public idEntity {
public:
  virtual void JobSync() {}
  virtual actorClass_t GetActorClass() const { return ACTORCLASS_HUMAN; }
  idFaction* GetFaction() const override { return faction; }
  void SetFactionForTest(idFaction* value) { faction = value; }
  virtual const idVec3 GetEyeOffset() const { return eyeOffset; }
  virtual void GetEyePos(idVec3* eyePosition);
  void SetEyeOffsetForTest(const idVec3& value) { eyeOffset = value; }
  bool IsCrouching() const { return posture == POSTURE_CROUCHED; }
  bool IsSitting() const { return posture == POSTURE_SITTING; }
  eventVoid* AnimEvent_DropWeapons(eventVoid* result);
  bool WereWeaponsDropped() const { return weaponsDropped; }
  walkState_t GetWalkState() const;
  void SetWalkState(walkState_t state);
  idRecoveredValue SaveDamageGroup(...);
  void SetLastDamageInfo(const idEntity* inflictor,
                         const idDeclDamage* damageDefinition);
  const idEntity* GetLastDamageInflictorForTest() const {
    return lastDamageInflictor;
  }
  const idDeclDamage* GetLastDamageDefinitionForTest() const {
    return lastDamageDefinition;
  }
  const idAnimator_AF* GetAF() const;
  idAnimator_AF* GetAFMutable() { return &articulatedFigureProperties; }
  void GetLastEyePos(idVec3* eyePosition) const;
  void GetEyeAxis(idMat3* eyeAxis) const;
  void SetEyeAxis(const idMat3* eyeAxis);
  const idInventoryCollection* GetInventory() const;
  const idAttachmentCollection* GetAttachments() const;
  const idList<idEntityPtr<idEntity>, 5>* GetTriggerTouchList() const;
  idStrId GetDisplayName() const;
  int GetPosture() const;
  leanState_t GetLeanState() const;
  void SetLeanState(leanState_t leanState);
  weaponReadyState_t GetWeaponReadyState() const;
  void SetWeaponReadyState(weaponReadyState_t state);
  void StopRagdoll() {
    ragdollActive = false;
    ++stopRagdollCount;
  }
  void SetRagdollActive(bool value) { ragdollActive = value; }
  bool IsRagdollActive() const { return ragdollActive; }
  int GetStopRagdollCount() const { return stopRagdollCount; }
  virtual idAnimator_AF::testSolidResult_t StartRagdoll(
      const idRagdollInfo* info) {
    lastRagdollInfo = *info;
    ragdollActive = true;
    ++startRagdollCount;
    return startRagdollResult;
  }
  void SetStartRagdollResult(idAnimator_AF::testSolidResult_t value) {
    startRagdollResult = value;
  }
  int GetStartRagdollCount() const { return startRagdollCount; }
  const idRagdollInfo& GetLastRagdollInfo() const { return lastRagdollInfo; }
  void SetLootState(bool lootableValue, bool hasLootableItemsValue,
                    bool hasLootableProjectilesValue = false) {
    lootable = lootableValue;
    hasLootableItems = hasLootableItemsValue;
    hasLootableProjectiles = hasLootableProjectilesValue;
  }
  bool IsLootable() const { return lootable; }
  bool HasLootableItems() const { return hasLootableItems; }
  bool HasLootableProjectiles() const { return hasLootableProjectiles; }
  idDisassembleLoot* GetDisassembleLoot() { return disassembleLoot; }
  void SetDisassembleLootForTest(idDisassembleLoot* value) {
    disassembleLoot = value;
  }
  void SetLooted(bool value) { lootedByPlayer = value; }
  bool IsLootedForTest() const { return lootedByPlayer; }
  virtual void DropAttachment(int slot) {
    droppedAttachmentSlots.Append(slot);
  }
  virtual void RemoveAttachment(const void* attachmentDeclaration) {
    removedAttachmentDeclarations.Append(attachmentDeclaration);
  }
  int GetDroppedAttachmentCountForTest() const {
    return droppedAttachmentSlots.Num();
  }
  int GetDroppedAttachmentSlotForTest(int index) const {
    return droppedAttachmentSlots[index];
  }
  void ClearDroppedAttachmentsForTest() { droppedAttachmentSlots.Clear(); }
  int GetRemovedAttachmentCountForTest() const {
    return removedAttachmentDeclarations.Num();
  }
  const void* GetRemovedAttachmentForTest(int index) const {
    return removedAttachmentDeclarations[index];
  }
  bool EquipItem(idInventoryItem* item, equipSlot_t equipSlot) {
    lastEquipItem = item;
    lastEquipSlot = equipSlot;
    return actorEquipResult;
  }
  bool UnequipItem(idInventoryItem* item) {
    lastUnequipItem = item;
    return actorUnequipResult;
  }
  void ConfigureActorEquipResults(bool equipResult, bool unequipResult) {
    actorEquipResult = equipResult;
    actorUnequipResult = unequipResult;
  }
  idInventoryItem* GetLastActorEquipItem() const { return lastEquipItem; }
  idInventoryItem* GetLastActorUnequipItem() const { return lastUnequipItem; }
  equipSlot_t GetLastActorEquipSlot() const { return lastEquipSlot; }
  idInventoryItem* GetEquipped(unsigned int slot) const {
    return slot < equippedItems.size() ? equippedItems[slot] : nullptr;
  }
  void SetEquipped(unsigned int slot, idInventoryItem* item) {
    if (slot < equippedItems.size()) equippedItems[slot] = item;
  }
  void Teleport(const idVec3* origin, idAngles* angles) {
    lastTeleportOrigin = *origin;
    lastTeleportAngles = *angles;
    ++teleportCount;
  }
  void GetAASPosition(const idAAS2* aas) {
    lastAASPositionSource = aas;
    ++aasPositionUpdateCount;
  }
  const idVec3& GetLastTeleportOrigin() const { return lastTeleportOrigin; }
  const idAngles& GetLastTeleportAngles() const { return lastTeleportAngles; }
  int GetTeleportCount() const { return teleportCount; }
  const idAAS2* GetLastAASPositionSource() const {
    return lastAASPositionSource;
  }
  int GetAASPositionUpdateCount() const { return aasPositionUpdateCount; }
  idWeapon* GetEquippedWeapon(unsigned int slot) const {
    return slot < equippedWeapons.size() ? equippedWeapons[slot] : nullptr;
  }
  idWeapon* GetEquippedWeapon() const { return activeEquippedWeapon; }
  void SetActiveEquippedWeapon(idWeapon* weapon) {
    activeEquippedWeapon = weapon;
  }
  idTreeAnimator* GetEquippedWeaponAnimator() const {
    return equippedWeaponAnimator;
  }
  void SetEquippedWeaponAnimator(idTreeAnimator* animator) {
    equippedWeaponAnimator = animator;
  }
  void SetEquippedWeapon(unsigned int slot, idWeapon* weapon) {
    if (slot < equippedWeapons.size()) {
      equippedWeapons[slot] = weapon;
    }
  }
  idInventoryCollection* GetInventoryCollection() { return &inventory; }
  void SetLastEyePosition(const idVec3& value) { lastEyePosition = value; }
  void SetDisplayNameId(unsigned int value) { displayName.id = value; }
  void SetPosture(int value) { posture = value; }
  void AddTriggerTouch(idEntity* value) {
    touching.Append(idEntityPtr<idEntity>(value));
  }

protected:
  bool weaponsDropped{false};
  walkState_t walkState{WALKSTATE_DEFAULT};
  std::array<idWeapon*, 15> equippedWeapons{};
  std::array<idInventoryItem*, 15> equippedItems{};
  idWeapon* activeEquippedWeapon{nullptr};
  idTreeAnimator* equippedWeaponAnimator{nullptr};
  idInventoryCollection inventory;
  idAnimator_AF articulatedFigureProperties;
  idVec3 lastEyePosition{};
  idVec3 eyeOffset{0.0F, 0.0F, 64.0F};
  idMat3 eyeAxis{};
  idAttachmentCollection attachments;
  idList<idEntityPtr<idEntity>, 5> touching;
  idStrId displayName{};
  int posture{1};
  leanState_t leanState{LEANSTATE_NONE};
  weaponReadyState_t weaponReadyState{WEAPON_READY_NONE};
  bool ragdollActive{false};
  int stopRagdollCount{0};
  int startRagdollCount{0};
  idAnimator_AF::testSolidResult_t startRagdollResult{
      idAnimator_AF::TESTSOLID_NOT_IN_SOLID};
  idRagdollInfo lastRagdollInfo{};
  bool lootable{false};
  bool hasLootableItems{false};
  bool hasLootableProjectiles{false};
  idDisassembleLoot* disassembleLoot{nullptr};
  bool lootedByPlayer{false};
  idList<int, 5> droppedAttachmentSlots;
  idList<const void*, 5> removedAttachmentDeclarations;
  bool actorEquipResult{true};
  bool actorUnequipResult{true};
  idInventoryItem* lastEquipItem{nullptr};
  idInventoryItem* lastUnequipItem{nullptr};
  equipSlot_t lastEquipSlot{EQUIPSLOT_PRIMARY};
  idVec3 lastTeleportOrigin{};
  idAngles lastTeleportAngles{};
  int teleportCount{0};
  const idAAS2* lastAASPositionSource{nullptr};
  int aasPositionUpdateCount{0};
  idFaction* faction{nullptr};
  const idEntity* lastDamageInflictor{nullptr};
  const idDeclDamage* lastDamageDefinition{nullptr};
};

class idPlayer : public idActor {
public:
  actorClass_t GetActorClass() const override { return ACTORCLASS_PLAYER; }
  bool IsDying() const { return dyingForTest; }
  void SetDyingForTest(bool value) { dyingForTest = value; }
  idFirstPersonEvent* GetCurFirstPersonEvent();
  void SetCurFirstPersonEvent(idFirstPersonEvent* value) {
    currentFirstPersonEvent = value;
  }
  idRecoveredValue GetUCmdTracker1(...);
  bool GetIsSprinting() const;
  void SetSprintingForTest(bool value) { sprinting = value; }
  idRecoveredValue GetFocusTracker(...);
  idRecoveredValue IsZoomedIn(...);
  idProxyMove* GetProxyMove() const;
  void SetProxyMoveForTest(idEntity* value) { proxyMoveEntity = value; }
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
  void ConfigureTakedownFocusState(int focusState, bool inTakedown) {
    takedownFocusState = focusState;
    takedownInProgress = inTakedown;
  }
  int GetTakedownFocusState() const { return takedownFocusState; }
  bool PlayerBehavior_Takedown_IsInTakedown() const {
    return takedownInProgress;
  }
  void StartPerfectOrigin(const idQuat& rotation, const idVec3& origin,
                          int duration) {
    lastPerfectRotation = rotation;
    lastPerfectOrigin = origin;
    lastPerfectDuration = duration;
    ++startPerfectOriginCount;
  }
  void SetPendingHandsAction(int action, bool force) {
    pendingHandsAction = action;
    pendingHandsActionForce = force;
    ++setPendingHandsActionCount;
  }
  void PlayerBehavior_Takedown_StartTakedown(
      const char* name, const idVec3& destination, const idMat3& axis,
      bool dualWield) {
    takedownName = name != nullptr ? name : "";
    takedownDestination = destination;
    takedownAxis = axis;
    takedownDualWield = dualWield;
    takedownInProgress = true;
    ++startTakedownCount;
  }
  int GetStartPerfectOriginCountForTest() const {
    return startPerfectOriginCount;
  }
  const idQuat& GetLastPerfectRotationForTest() const {
    return lastPerfectRotation;
  }
  const idVec3& GetLastPerfectOriginForTest() const {
    return lastPerfectOrigin;
  }
  int GetLastPerfectDurationForTest() const { return lastPerfectDuration; }
  int GetSetPendingHandsActionCountForTest() const {
    return setPendingHandsActionCount;
  }
  int GetPendingHandsActionForTest() const { return pendingHandsAction; }
  bool GetPendingHandsActionForceForTest() const {
    return pendingHandsActionForce;
  }
  int GetStartTakedownCountForTest() const { return startTakedownCount; }
  const char* GetStartedTakedownNameForTest() const {
    return takedownName.c_str();
  }
  const idVec3& GetStartedTakedownDestinationForTest() const {
    return takedownDestination;
  }
  const idMat3& GetStartedTakedownAxisForTest() const {
    return takedownAxis;
  }
  bool GetStartedTakedownDualWieldForTest() const {
    return takedownDualWield;
  }
  float CalcFov(bool weaponFov) const {
    return weaponFov ? weaponFieldOfView : fieldOfView;
  }
  void SetFieldOfViewForTest(float normalFov, float weaponFov) {
    fieldOfView = normalFov;
    weaponFieldOfView = weaponFov;
  }
  idEntity* GetVehicle() const { return vehicle; }
  void SetVehicleForTest(idEntity* value) { vehicle = value; }
  idEntity* GetControl() const { return control; }
  void SetControlForTest(idEntity* value) { control = value; }
  bool CheckAdditionalGoreTypes(unsigned int damageTypes) {
    lastAdditionalGoreDamageTypes = damageTypes;
    ++additionalGoreQueryCount;
    return additionalGoreResult;
  }
  void SetAdditionalGoreResultForTest(bool value) {
    additionalGoreResult = value;
  }
  int GetAdditionalGoreQueryCountForTest() const {
    return additionalGoreQueryCount;
  }
  unsigned int GetLastAdditionalGoreDamageTypesForTest() const {
    return lastAdditionalGoreDamageTypes;
  }
  void SetCommanderHUDEnabledForTest(bool value) {
    commanderHUDEnabled = value;
  }
  bool IsCommanderHUDEnabled() const { return commanderHUDEnabled; }
  void Disassemble(idDisassembleLoot* loot, idEntity* entity) {
    lastDisassembleLoot = loot;
    lastDisassembledEntity = entity;
    ++disassembleCount;
  }
  void LootBoundItems(idEntity* entity) {
    lootedBoundEntities.Append(entity);
  }
  bool LootItems(idEntity* entity, const idDisassembleLoot* disassemble) {
    lastLootItemsEntity = entity;
    lastLootItemsDisassemble = disassemble;
    ++lootItemsCount;
    return lootItemsResult;
  }
  void SetLootItemsResultForTest(bool value) { lootItemsResult = value; }
  int GetDisassembleCountForTest() const { return disassembleCount; }
  idDisassembleLoot* GetLastDisassembleLootForTest() const {
    return lastDisassembleLoot;
  }
  idEntity* GetLastDisassembledEntityForTest() const {
    return lastDisassembledEntity;
  }
  int GetLootBoundItemsCountForTest() const {
    return lootedBoundEntities.Num();
  }
  idEntity* GetLootBoundItemsEntityForTest(int index) const {
    return lootedBoundEntities[index];
  }
  int GetLootItemsCountForTest() const { return lootItemsCount; }
  idEntity* GetLastLootItemsEntityForTest() const {
    return lastLootItemsEntity;
  }
  const idDisassembleLoot* GetLastLootItemsDisassembleForTest() const {
    return lastLootItemsDisassemble;
  }
  void SetTakedownIndicationTextForTest(unsigned int textId,
                                        const char* localizedText) {
    takedownIndicationTextId.id = textId;
    takedownIndicationLocalizedText =
        localizedText != nullptr ? localizedText : "";
  }
  idStrId GetTakedownIndicationTextId() const {
    ++playerTextsQueryCount;
    return takedownIndicationTextId;
  }
  const char* GetLocalizedPlayerText(idStrId textId) const {
    ++localizedPlayerTextQueryCount;
    return textId.id == takedownIndicationTextId.id
               ? takedownIndicationLocalizedText.c_str()
               : "";
  }
  int GetPlayerTextsQueryCountForTest() const {
    return playerTextsQueryCount;
  }
  int GetLocalizedPlayerTextQueryCountForTest() const {
    return localizedPlayerTextQueryCount;
  }
  void SetIsControllingAI(bool value) { isControllingAI = value; }
  bool IsControllingAIForTest() const { return isControllingAI; }
  void RemoveControl(idEntity* controlledEntity) {
    lastRemovedControl = controlledEntity;
    ++removeControlCount;
    if (control == controlledEntity) control = nullptr;
  }
  void AddControl(idEntity* controlledEntity) {
    lastAddedControl = controlledEntity;
    ++addControlCount;
    control = controlledEntity;
  }
  void SetViewAngles(const idAngles& value, bool force) {
    viewAngles = value;
    viewAnglesForced = force;
    ++setViewAnglesCount;
  }
  void SetCommanderHUDEnabled(bool value) { commanderHUDEnabled = value; }
  idEntity* SetupDeathCameraForAI(idEntity* controlledEntity,
                                  const idVec3& position,
                                  const idMat3& axis) {
    deathCameraTarget = controlledEntity;
    deathCameraPosition = position;
    deathCameraAxis = axis;
    ++deathCameraSetupCount;
    return deathCamera;
  }
  void HideHands(handHideReason_t reason) {
    handsHideMask |= 1U << static_cast<unsigned int>(reason);
    lastHandsHideReason = reason;
    ++handsHideCount;
  }
  void ShowHands(handHideReason_t reason) {
    handsHideMask &= ~(1U << static_cast<unsigned int>(reason));
    lastHandsShowReason = reason;
    ++handsShowCount;
  }
  void HideHandsForTest(handHideReason_t reason) {
    handsHideMask |= 1U << static_cast<unsigned int>(reason);
  }
  bool AreHandsHiddenForTest(handHideReason_t reason) const {
    return (handsHideMask &
            (1U << static_cast<unsigned int>(reason))) != 0;
  }
  int GetRemoveControlCountForTest() const { return removeControlCount; }
  idEntity* GetLastRemovedControlForTest() const {
    return lastRemovedControl;
  }
  int GetAddControlCountForTest() const { return addControlCount; }
  idEntity* GetLastAddedControlForTest() const { return lastAddedControl; }
  const idAngles& GetViewAnglesForTest() const { return viewAngles; }
  bool WereViewAnglesForcedForTest() const { return viewAnglesForced; }
  int GetSetViewAnglesCountForTest() const { return setViewAnglesCount; }
  idEntity* GetDeathCameraForTest() const { return deathCamera; }
  idEntity* GetDeathCameraTargetForTest() const { return deathCameraTarget; }
  const idVec3& GetDeathCameraPositionForTest() const {
    return deathCameraPosition;
  }
  const idMat3& GetDeathCameraAxisForTest() const { return deathCameraAxis; }
  int GetDeathCameraSetupCountForTest() const { return deathCameraSetupCount; }
  int GetHandsHideCountForTest() const { return handsHideCount; }
  int GetHandsShowCountForTest() const { return handsShowCount; }
  handHideReason_t GetLastHandsHideReasonForTest() const {
    return lastHandsHideReason;
  }
  handHideReason_t GetLastHandsShowReasonForTest() const {
    return lastHandsShowReason;
  }

private:
  idFirstPersonEvent* currentFirstPersonEvent{nullptr};
  bool noclip{false};
  idVec3 aiAimOrigin{};
  bool canGrapple{true};
  idVec3 aiAimDirection{1.0F, 0.0F, 0.0F};
  float aiAimDistance{4096.0F};
  float totalCurHealth{100.0F};
  bool spectating{false};
  bool sprinting{false};
  bool dyingForTest{false};
  int takedownFocusState{0};
  bool takedownInProgress{false};
  float fieldOfView{90.0F};
  float weaponFieldOfView{90.0F};
  idEntity* vehicle{nullptr};
  idEntity* control{nullptr};
  idEntity* proxyMoveEntity{nullptr};
  bool additionalGoreResult{false};
  int additionalGoreQueryCount{0};
  unsigned int lastAdditionalGoreDamageTypes{0};
  bool commanderHUDEnabled{false};
  idQuat lastPerfectRotation{};
  idVec3 lastPerfectOrigin{};
  int lastPerfectDuration{0};
  int startPerfectOriginCount{0};
  int pendingHandsAction{0};
  bool pendingHandsActionForce{false};
  int setPendingHandsActionCount{0};
  std::string takedownName;
  idVec3 takedownDestination{};
  idMat3 takedownAxis{};
  bool takedownDualWield{false};
  int startTakedownCount{0};
  idDisassembleLoot* lastDisassembleLoot{nullptr};
  idEntity* lastDisassembledEntity{nullptr};
  int disassembleCount{0};
  idList<idEntity*, 5> lootedBoundEntities;
  idEntity* lastLootItemsEntity{nullptr};
  const idDisassembleLoot* lastLootItemsDisassemble{nullptr};
  int lootItemsCount{0};
  bool lootItemsResult{false};
  bool isControllingAI{false};
  idEntity* lastRemovedControl{nullptr};
  int removeControlCount{0};
  idEntity* lastAddedControl{nullptr};
  int addControlCount{0};
  idAngles viewAngles{};
  bool viewAnglesForced{false};
  int setViewAnglesCount{0};
  idEntity ownedDeathCamera;
  idEntity* deathCamera{&ownedDeathCamera};
  idEntity* deathCameraTarget{nullptr};
  idVec3 deathCameraPosition{};
  idMat3 deathCameraAxis{};
  int deathCameraSetupCount{0};
  unsigned int handsHideMask{0};
  handHideReason_t lastHandsHideReason{HAND_HIDE_GENERIC};
  handHideReason_t lastHandsShowReason{HAND_HIDE_GENERIC};
  int handsHideCount{0};
  int handsShowCount{0};
  idStrId takedownIndicationTextId{};
  std::string takedownIndicationLocalizedText;
  mutable int playerTextsQueryCount{0};
  mutable int localizedPlayerTextQueryCount{0};
};

class idTreeAnimator {
public:
  struct tagData_t {
    idVec3 trans{};
    idQuat rot{};
    unsigned short parentJoint{0xFFFFU};
    unsigned short padding{0};
  };

  const char* GetModelDefName();
  void SetModelDefName(const char* value) { modelDefName = value; }
  void SetModelCapsForTest(unsigned int value) { modelCaps = value; }
  unsigned int GetModelCaps() const { return modelCaps; }
  void AddAliasForTest(const char* aliasName, unsigned short handle) {
    aliases.push_back(
        {aliasName != nullptr ? aliasName : "", handle});
  }
  unsigned short StrongLoadAliasForAI(const char* aliasName) const {
    if (aliasName == nullptr) return 0xFFFFU;
    for (const auto& alias : aliases) {
      if (alias.name == aliasName) return alias.handle;
    }
    return 0xFFFFU;
  }
  void AddEyeInfoForTest(const char* eyeInfoName, unsigned short handle) {
    eyeInfos.push_back(
        {eyeInfoName != nullptr ? eyeInfoName : "", handle});
  }
  unsigned short FindEyeInfoHandleForAI(const char* eyeInfoName) const {
    if (eyeInfoName == nullptr) return 0xFFFFU;
    for (const auto& eyeInfo : eyeInfos) {
      if (eyeInfo.name == eyeInfoName) return eyeInfo.handle;
    }
    return 0xFFFFU;
  }
  void AddTagForTest(const char* tagName, const idVec3& translation,
                     const idQuat& rotation, unsigned short parentJoint) {
    tags.push_back({"", tagName != nullptr ? tagName : "",
                    {translation, rotation, parentJoint, 0}});
  }
  void AddArmorPropTagForTest(const char* propName, const char* tagName,
                              const idVec3& translation,
                              const idQuat& rotation,
                              unsigned short parentJoint) {
    tags.push_back({propName != nullptr ? propName : "",
                    tagName != nullptr ? tagName : "",
                    {translation, rotation, parentJoint, 0}});
  }
  const tagData_t* FindTagForAI(const char* tagName) const {
    if (tagName == nullptr) return nullptr;
    for (const auto& tag : tags) {
      if (tag.name == tagName) return &tag.data;
    }
    return nullptr;
  }
  const tagData_t* FindArmorPropTagForAI(const char* propName,
                                         const char* tagName) const {
    if (tagName == nullptr) return nullptr;
    const std::string requestedProp =
        propName != nullptr ? propName : "";
    for (const auto& tag : tags) {
      if (tag.propName == requestedProp && tag.name == tagName) {
        return &tag.data;
      }
    }
    return FindTagForAI(tagName);
  }
  void ConfigureWorldSpaceTagTransformForTest(bool succeeds,
                                               const idVec3& origin,
                                               const idMat3& axis) {
    worldSpaceTagTransformSucceeds = succeeds;
    worldSpaceTagOrigin = origin;
    worldSpaceTagAxis = axis;
  }
  bool GetWorldSpaceTagTransformForAI(const tagData_t& tagData,
                                      idVec3& origin,
                                      idMat3& axis) const {
    lastWorldSpaceTagData = tagData;
    ++worldSpaceTagTransformCount;
    if (!worldSpaceTagTransformSucceeds) return false;
    origin = worldSpaceTagOrigin;
    axis = worldSpaceTagAxis;
    return true;
  }
  int GetWorldSpaceTagTransformCountForTest() const {
    return worldSpaceTagTransformCount;
  }
  const tagData_t& GetLastWorldSpaceTagDataForTest() const {
    return lastWorldSpaceTagData;
  }
  void ConfigureWorldSpaceJointForArmorTest(short jointIndex,
                                             bool succeeds,
                                             const idVec3& origin) {
    armorJointIndex = jointIndex;
    worldSpaceJointPositionSucceeds = succeeds;
    worldSpaceJointOrigin = origin;
  }
  short GetJointIndexForAI(unsigned short jointHandle) const {
    lastJointHandle = jointHandle;
    ++jointIndexQueryCount;
    return armorJointIndex >= 0 ? armorJointIndex
                               : static_cast<short>(jointHandle);
  }
  bool GetWorldSpaceJointPositionForAI(short jointIndex,
                                       idVec3& origin) const {
    lastJointIndex = jointIndex;
    ++worldSpaceJointPositionCount;
    if (!worldSpaceJointPositionSucceeds) return false;
    origin = worldSpaceJointOrigin;
    return true;
  }
  int GetJointIndexQueryCountForTest() const {
    return jointIndexQueryCount;
  }
  int GetWorldSpaceJointPositionCountForTest() const {
    return worldSpaceJointPositionCount;
  }
  void StrongLoadTwitchPainAliasesForAI() {
    ++twitchPainStrongLoadCount;
  }
  int GetTwitchPainStrongLoadCountForTest() const {
    return twitchPainStrongLoadCount;
  }
  void EnableWalkIKForAI() { walkIKEnabled = true; }
  bool IsWalkIKEnabledForTest() const { return walkIKEnabled; }
  void SetModelWalkIKDeclForTest(const void* value) {
    modelWalkIKDecl = value;
  }
  const void* GetModelWalkIKDeclForAI() const { return modelWalkIKDecl; }
  int GetMeshIndex(const char* meshName) const {
    if (meshName == nullptr) return -1;
    for (std::size_t index = 0; index < meshNames.size(); ++index) {
      if (meshNames[index] == meshName) return static_cast<int>(index);
    }
    return -1;
  }
  void AddMesh(const char* meshName) {
    meshNames.emplace_back(meshName != nullptr ? meshName : "");
    hiddenMeshes.push_back(false);
  }
  void HideMesh(int meshIndex) {
    if (meshIndex >= 0 &&
        meshIndex < static_cast<int>(hiddenMeshes.size())) {
      hiddenMeshes[static_cast<std::size_t>(meshIndex)] = true;
    }
  }
  void HideKitsExcept(int kitType, const char* name) {
    lastHiddenKitType = kitType;
    lastHiddenKitName = name != nullptr ? name : "";
    ++hideKitsExceptCount;
  }
  bool IsMeshHidden(int meshIndex) const {
    return meshIndex >= 0 &&
           meshIndex < static_cast<int>(hiddenMeshes.size()) &&
           hiddenMeshes[static_cast<std::size_t>(meshIndex)];
  }
  int GetHideKitsExceptCountForTest() const {
    return hideKitsExceptCount;
  }
  int GetLastHiddenKitTypeForTest() const { return lastHiddenKitType; }
  const char* GetLastHiddenKitNameForTest() const {
    return lastHiddenKitName.data;
  }
  void SetFrameFromAnim(const idMD6Anim* animation, int frame,
                        bool force, bool sync, void* callback) {
    (void)animation;
    (void)frame;
    (void)force;
    (void)sync;
    (void)callback;
  }
  void ConfigureRenderAxesForTest(const idMat3& globalAxisValue,
                                  const idMat3& deferredAxisValue,
                                  bool deferredAxisValidValue) {
    globalAxis = globalAxisValue;
    deferredAxis = deferredAxisValue;
    deferredAxisValid = deferredAxisValidValue;
  }
  const idMat3& GetRenderAxisForAI() const {
    return deferredAxisValid ? deferredAxis : globalAxis;
  }
  void ConfigureGoreSkeletonForTest(bool available);
  void AddGoreJointParentForTest(short child, short parent);
  void ClearGoreJointParentsForTest();
  bool HasGoreSkeletonForAI() const;
  bool IsGoreJointChildOfForAI(short child, short parent) const;
  int GetGoreJointChildQueryCountForTest() const;

private:
  struct namedHandle_t {
    std::string name;
    unsigned short handle;
  };
  struct namedTag_t {
    std::string propName;
    std::string name;
    tagData_t data;
  };

  idStr modelDefName;
  unsigned int modelCaps{0};
  std::vector<std::string> meshNames;
  std::vector<bool> hiddenMeshes;
  std::vector<namedHandle_t> aliases;
  std::vector<namedHandle_t> eyeInfos;
  std::vector<namedTag_t> tags;
  bool worldSpaceTagTransformSucceeds{false};
  idVec3 worldSpaceTagOrigin{};
  idMat3 worldSpaceTagAxis{};
  mutable tagData_t lastWorldSpaceTagData{};
  mutable int worldSpaceTagTransformCount{0};
  short armorJointIndex{-1};
  bool worldSpaceJointPositionSucceeds{false};
  idVec3 worldSpaceJointOrigin{};
  mutable unsigned short lastJointHandle{0xFFFFU};
  mutable short lastJointIndex{-1};
  mutable int jointIndexQueryCount{0};
  mutable int worldSpaceJointPositionCount{0};
  int twitchPainStrongLoadCount{0};
  bool walkIKEnabled{false};
  const void* modelWalkIKDecl{nullptr};
  idMat3 globalAxis{};
  idMat3 deferredAxis{};
  bool deferredAxisValid{false};
  int lastHiddenKitType{3};
  idStr lastHiddenKitName;
  int hideKitsExceptCount{0};
};

class idActionNode : public idEntity {
public:
  static idActionNode* CastTo(idEntity* entity) {
    return dynamic_cast<idActionNode*>(entity);
  }
  void LeaveActionNode(idAI2* ai) {
    lastLeavingAI = ai;
    ++leaveCount;
  }
  int GetLeaveCount() const { return leaveCount; }
  idAI2* GetLastLeavingAI() const { return lastLeavingAI; }

private:
  idAI2* lastLeavingAI{nullptr};
  int leaveCount{0};
};

struct idRelationship {
  enum attitude_t : int {
    ATTITUDE_UNKNOWN = -1,
    ATTITUDE_DESPISE = 0,
    ATTITUDE_HATE = 1,
    ATTITUDE_DISLIKE = 2,
    ATTITUDE_NEUTRAL = 3,
    ATTITUDE_LIKE = 4,
    ATTITUDE_LOVE = 5,
    ATTITUDE_IDOLIZE = 6,
    ATTITUDE_MAX = 7,
  };

  enum attitudeStage_t : int {
    ATTITUDESTAGE_OUTSIDER = 0,
    ATTITUDESTAGE_ACCEPTED = 1,
    ATTITUDESTAGE_HEROIC = 2,
    ATTITUDESTAGE_NUM = 3,
  };

  static attitudeStage_t AttitudeToAttitudeStage(attitude_t attitude) {
    if (attitude <= ATTITUDE_NEUTRAL) return ATTITUDESTAGE_OUTSIDER;
    return attitude > ATTITUDE_LIKE ? ATTITUDESTAGE_HEROIC
                                    : ATTITUDESTAGE_ACCEPTED;
  }
  static float FloatForAttitude(attitude_t attitude) {
    return static_cast<float>(attitude);
  }
};

class idFaction {
public:
  const char* GetName();
  void SetName(const char* value) { name = value; }
  idRelationship::attitude_t GetAttitudeTowards(
      const idEntity* otherEntity) const {
    return otherEntity == attitudeEntity ? attitude
                                         : idRelationship::ATTITUDE_NEUTRAL;
  }
  idRelationship::attitude_t GetAttitudeTowards(
      const idFaction* otherFaction) const {
    return otherFaction == attitudeFaction ? factionAttitude
                                           : idRelationship::ATTITUDE_NEUTRAL;
  }
  void SetAttitudeTowardsForTest(
      const idEntity* otherEntity, idRelationship::attitude_t value) {
    attitudeEntity = otherEntity;
    attitude = value;
  }
  void SetAttitudeTowardsForTest(
      const idFaction* otherFaction, idRelationship::attitude_t value) {
    attitudeFaction = otherFaction;
    factionAttitude = value;
  }
  void CullEntityRelationships() { ++cullEntityRelationshipsCount; }
  int GetCullEntityRelationshipsCountForTest() const {
    return cullEntityRelationshipsCount;
  }

private:
  idStr name;
  const idEntity* attitudeEntity{nullptr};
  idRelationship::attitude_t attitude{idRelationship::ATTITUDE_NEUTRAL};
  const idFaction* attitudeFaction{nullptr};
  idRelationship::attitude_t factionAttitude{
      idRelationship::ATTITUDE_NEUTRAL};
  int cullEntityRelationshipsCount{0};
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
  idTarget_FakeEnemy* GetFakeEnemyForTest() const { return fakeEnemy; }
  void SetFakeEnemyForTest(idTarget_FakeEnemy* value) { fakeEnemy = value; }

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
  combatStage_t stageType{COMBAT_STAGE_DEFAULT};
  idTarget_FakeEnemy* fakeEnemy{nullptr};
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
  void SetAnyMemberHighestAlertCycleForTest(int value) {
    anyMemberHighestAlertCycle = value;
  }
  int GetHighestAlertCycleQueryCountForTest() const {
    return highestAlertCycleQueryCount;
  }
  void ResetHighestAlertCycleQueryCountForTest() const {
    highestAlertCycleQueryCount = 0;
  }
  void SetGroupEnemyUpdateTimeStamp(const idEntity* enemy, int time,
                                    const idVec3* enemyPosition);
  void SetGroupEnemyUpdateTimeStamp(const idEntity* enemy, int time) {
    const idVec3 position =
        enemy != nullptr && enemy->GetPhysics() != nullptr
            ? enemy->GetPhysics()->GetOrigin()
            : idVec3{};
    SetGroupEnemyUpdateTimeStamp(enemy, time, &position);
  }
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
  void SetDeadMembersForTest(int value) { numDeadMembers = value; }
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
  int GetMemberKilledCallCountForTest() const { return memberKilledCallCount; }
  idAI2* GetLastKilledMemberForTest() const { return lastKilledMember; }
  const idEntity* GetLastKilledAttackerForTest() const {
    return lastKilledAttacker;
  }
  const idEntity* GetLastKilledInflictorForTest() const {
    return lastKilledInflictor;
  }
  int GetGroupEnemyUpdateTimeStampForTest() const {
    return groupEnemyUpdateTimeStamp;
  }
  const idEntity* GetGroupEnemyForTest() const { return groupEnemy; }

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
  mutable int highestAlertCycleQueryCount{0};
  int groupEnemyUpdateTimeStamp{-1};
  const idEntity* groupEnemy{nullptr};
  int curCombatStage{-1};
  int pendingCombatStage{-1};
  int pendingCombatStageTime{-1};
  int initialCombatStage{0};
  int recentlyKilledMembers{0};
  int stageChangeTime{0};
  int randomSeed{0x12345};
  bool anyMemberHasSeenDeadBody{false};
  bool shouldAmbush{false};
  int memberKilledCallCount{0};
  idAI2* lastKilledMember{nullptr};
  const idEntity* lastKilledAttacker{nullptr};
  const idEntity* lastKilledInflictor{nullptr};
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
  enum type_t { EVENT_NONE = 0, EVENT_FSMEVENT = 1 };
  int eventnum{0};
  type_t type{EVENT_NONE};
};

struct alignas(8) idEventArg {
  unsigned char type{0};
  unsigned char padding[7]{};
  union value_t {
    int i;
    float f;
    const char* s;
    const void* pointer;
    unsigned char bytes[16];
    value_t() : bytes{} {}
  } value;
};
static_assert(sizeof(idEventArg) == 24, "idEventArg ABI drift");

extern const idEventDef EV_DisintigrateHide;
extern const idEventDef EV_Hide;
extern const idEventDef EV_Show;
extern const idEventDef EV_Activate;
extern const idEventDef EV_DropAttachedProjectiles;
extern const idEventDef EV_Remove;
extern const idEventDef EV_RemoveBody;
extern const idEventDef EV_VaporizedAI;
extern const idEventDef EV_AnimWeb_Wait;
extern const idEventDef EV_AnimWeb_BeginSyncedAnim;
extern const idEventDef FSMEvent_Pain;
extern const idEventDef FSMEvent_Killed;
extern const idEventDef FSMEvent_Reaction;
extern const idEventDef ACTION_CrouchToStandEnter;
extern const idEventDef ACTION_StandToCrouchEnter;
extern const idEventDef ACTION_PlayOverrideAnimEnter;
extern idCVar ai_syncAF;

class idThread {
public:
  explicit idThread(int handleValue = 0);
  ~idThread();

  int GetHandle() const { return handle.value; }
  bool IsDying() const { return dying; }
  void SetDying(bool value) { dying = value; }
  void Execute() { ++executeCount; }
  int GetExecuteCount() const { return executeCount; }
  void ManualDelete() { manualDelete = true; }
  bool IsManualDeleteForTest() const { return manualDelete; }
  static idThread* Find(int handleValue);

private:
  idThreadHandle handle;
  bool dying{false};
  int executeCount{0};
  bool manualDelete{false};
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
  const idMD6Anim* animation{nullptr};
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

class idDeclDamage {
public:
  struct damageParms_t {
    damageIntensity_t intensity{DAMAGEINTENSITY_NONE};
    unsigned int damageTypes{0};
    unsigned int goreTypes{0};
    unsigned int unlockableGoreTypes{0};
  } damageParms;
  idDeclDamage() = default;
  explicit idDeclDamage(int indexValue) : index(indexValue) {}
  idAtomicString name;
  int index{0};
  float radius{0.0F};
};

struct idDeclProjectileImpactEffect {
  int index{0};
};

class idAI2;

class idInventoryItem : public idEntity {
public:
  void SetEquipSlotForGoreTest(int value);
  int GetEquipSlotForAI() const;
};

struct testFireWeaponParms_t {
  bool freeAmmo{false};
  std::uint8_t recoveredPadding0[3]{};
  float recoveredUnusedFloat0{0.99F};
  float recoveredUnusedFloat1{0.95F};
  float recoveredUnusedFloat2{0.8F};
  float alwaysAimingDistance{0.0F};
  bool useMuzzleDirForFireAxis{false};
  bool useWeaponSpread{false};
  bool ignoreAimFocus{false};
  std::uint8_t recoveredPadding1{};
};
static_assert(offsetof(testFireWeaponParms_t, alwaysAimingDistance) == 16,
              "test-fire weapon parms ABI drift");
static_assert(offsetof(testFireWeaponParms_t, useMuzzleDirForFireAxis) == 20,
              "test-fire weapon flags ABI drift");
static_assert(sizeof(testFireWeaponParms_t) == 24,
              "test-fire weapon parms size drift");

enum testForBlockedTraceResult_t : int {
  TEST_BLOCKED_TRACE_CLOSE = 0,
  TEST_BLOCKED_TRACE_DISTANT = 1,
  TEST_BLOCKED_TRACE_CLEAR = 2,
};

struct testForBlockedTraceParms_t {
  float recoveredUnusedFraction{0.5F};
  float maxBlockingDistance{0.0F};
};
static_assert(offsetof(testForBlockedTraceParms_t, maxBlockingDistance) == 4,
              "blocked-trace parms ABI drift");

struct multiHitTrace_t {
  int numHits{0};
  std::array<trace_t, 4> hits{};
};

class idTestFireResults {
public:
  idTestFireResults& operator=(const idTestFireResults&) = default;
  int numTraces{0};
  std::array<multiHitTrace_t, 8> multiTraces{};
  std::array<float, 8> traceLens{};
  bool useOverrideFxDir{false};

  // Client hit-result fields share this retail type in other game modules.
  idVec3 impact{};
  float damage{0.0F};
  int hitEntity{-1};
  bool hit{false};
};

class idFireParms {
public:
  idFireParms() { Clear(); }
  void Clear() {
    weapon = nullptr;
    start = {};
    muzzleOffset = {};
    fireAxis = {};
    fireAxis[0] = {1.0F, 0.0F, 0.0F};
    fireAxis[1] = {0.0F, 1.0F, 0.0F};
    fireAxis[2] = {0.0F, 0.0F, 1.0F};
    target = nullptr;
    freeAmmo = false;
    startTime = gameLocal != nullptr ? gameLocal->GetGameTimeMs() : 0;
  }

  idWeapon* weapon{nullptr};
  idVec3 start{};
  idVec3 muzzleOffset{};
  idMat3 fireAxis{};
  idPresentable* target{nullptr};
  bool freeAmmo{false};
  int startTime{0};
};

class idWeapon : public idInventoryItem {
public:
  enum triggerState_t { TRIGGERSTATE_RELEASED, TRIGGERSTATE_PULLED };
  enum fireState_t { FIRESTATE_IDLE, FIRESTATE_FIRING, FIRESTATE_RELOADING };
  enum weaponState_t { WEAPONSTATE_NORMAL, WEAPONSTATE_SECONDARY };

  static idWeapon* CastTo(idInventoryItem* item) {
    return dynamic_cast<idWeapon*>(item);
  }

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
  void SetMuzzleTransformFailureForTest(bool value) {
    forceMuzzleTransformFailure = value;
  }
  bool ForcesMuzzleTransformFailureForTest() const {
    return forceMuzzleTransformFailure;
  }
  void SetLaserTransform(const idVec3& position, const idMat3& axis) {
    laserPosition = position;
    laserAxis = axis;
    hasLaserTransform = true;
  }
  bool QueryMuzzleTransform(idVec3* position, idMat3* axis) const;
  bool QueryLaserTransform(idVec3* position, idMat3* axis) const;
  void GetMuzzleFlashWorldTransform(idTreeAnimator* animator, int flashIndex,
                                    idVec3* position, idMat3* axis) {
    lastMuzzleAnimator = animator;
    lastMuzzleFlashIndex = flashIndex;
    ++muzzleTransformCount;
    if (!QueryMuzzleTransform(position, axis)) {
      *position = {};
      *axis = {};
    }
  }
  void Reload(idInventoryCollection* inventory, idFXManager* effects,
              bool firstFlag, bool secondFlag, bool thirdFlag) {
    lastReloadInventory = inventory;
    lastReloadEffects = effects;
    lastReloadFlags = {{firstFlag, secondFlag, thirdFlag}};
    ++reloadCount;
  }
  int GetReloadCount() const { return reloadCount; }
  idInventoryCollection* GetLastReloadInventory() const {
    return lastReloadInventory;
  }
  idFXManager* GetLastReloadEffects() const { return lastReloadEffects; }
  const std::array<bool, 3>& GetLastReloadFlags() const {
    return lastReloadFlags;
  }
  int GetMuzzleTransformCount() const { return muzzleTransformCount; }
  idTreeAnimator* GetLastMuzzleAnimator() const { return lastMuzzleAnimator; }
  int GetLastMuzzleFlashIndex() const { return lastMuzzleFlashIndex; }
  bool UsesFireControl() const { return usesFireControl; }
  void SetUsesFireControl(bool value) { usesFireControl = value; }
  bool IsMelee() const { return melee; }
  void SetMeleeForTest(bool value) { melee = value; }
  bool ShouldAimAtHead() const { return shouldAimAtHead; }
  void SetShouldAimAtHeadForTest(bool value) { shouldAimAtHead = value; }
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
  float GetLoadedFraction(bool secondary) const {
    return secondary ? secondaryLoadedFraction : primaryLoadedFraction;
  }
  void SetLoadedFraction(float primary, float secondary = 1.0F) {
    primaryLoadedFraction = primary;
    secondaryLoadedFraction = secondary;
  }
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
  void StopAllFX(idFXManager* manager) {
    lastStopAllFXManager = manager;
    firingEffectsStopped = true;
    ++stopAllFXCount;
  }
  int GetReleaseCountForTest() const { return releaseCount; }
  int GetStopAllFXCountForTest() const { return stopAllFXCount; }
  idFXManager* GetLastStopAllFXManagerForTest() const {
    return lastStopAllFXManager;
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
  void AddSpreadToFireAxis(const idMat3& inputAxis, aiAccuracy_t accuracy,
                           float modifier, idMat3& outputAxis) const {
    ++spreadCallCount;
    lastSpreadInputAxis = inputAxis;
    lastSpreadAccuracy = accuracy;
    lastSpreadModifier = modifier;
    outputAxis = hasSpreadOutput ? spreadOutputAxis : inputAxis;
  }
  virtual void DeferredFire(idPresentable* ownerPresentable,
                            const idPresentable* sourcePresentable,
                            const idVec3& muzzleWorldPosition,
                            const idMat3& axis, idFireParms& fireParms,
                            bool secondary) {
    ++deferredFireCount;
    lastDeferredOwnerPresentable = ownerPresentable;
    lastDeferredSourcePresentable = sourcePresentable;
    lastDeferredMuzzlePosition = muzzleWorldPosition;
    lastDeferredFireAxis = axis;
    lastDeferredFireParms = &fireParms;
    lastDeferredSecondary = secondary;
    fireParms.start = muzzleWorldPosition;
    fireParms.fireAxis = axis;
  }
  void SetSpreadOutputForTest(const idMat3& value) {
    spreadOutputAxis = value;
    hasSpreadOutput = true;
  }
  void ClearSpreadOutputForTest() { hasSpreadOutput = false; }
  int GetSpreadCallCount() const { return spreadCallCount; }
  const idMat3& GetLastSpreadInputAxis() const { return lastSpreadInputAxis; }
  aiAccuracy_t GetLastSpreadAccuracy() const { return lastSpreadAccuracy; }
  float GetLastSpreadModifier() const { return lastSpreadModifier; }
  int GetDeferredFireCount() const { return deferredFireCount; }
  idPresentable* GetLastDeferredOwnerPresentable() const {
    return lastDeferredOwnerPresentable;
  }
  const idPresentable* GetLastDeferredSourcePresentable() const {
    return lastDeferredSourcePresentable;
  }
  const idVec3& GetLastDeferredMuzzlePosition() const {
    return lastDeferredMuzzlePosition;
  }
  const idMat3& GetLastDeferredFireAxis() const {
    return lastDeferredFireAxis;
  }
  idFireParms* GetLastDeferredFireParms() const {
    return lastDeferredFireParms;
  }
  bool GetLastDeferredSecondary() const { return lastDeferredSecondary; }

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
  int stopAllFXCount{0};
  idFXManager* lastStopAllFXManager{nullptr};

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
  bool forceMuzzleTransformFailure{false};
  bool hasLaserTransform{false};
  bool usesFireControl{true};
  bool melee{false};
  bool shouldAimAtHead{false};
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
  float primaryLoadedFraction{1.0F};
  float secondaryLoadedFraction{1.0F};
  idInventoryCollection* lastReloadInventory{nullptr};
  idFXManager* lastReloadEffects{nullptr};
  std::array<bool, 3> lastReloadFlags{{false, false, false}};
  int reloadCount{0};
  idTreeAnimator* lastMuzzleAnimator{nullptr};
  int lastMuzzleFlashIndex{-1};
  int muzzleTransformCount{0};
  bool hasSpreadOutput{false};
  idMat3 spreadOutputAxis{};
  mutable int spreadCallCount{0};
  mutable idMat3 lastSpreadInputAxis{};
  mutable aiAccuracy_t lastSpreadAccuracy{ACCURACY_FROMSKILL};
  mutable float lastSpreadModifier{0.0F};
  int deferredFireCount{0};
  idPresentable* lastDeferredOwnerPresentable{nullptr};
  const idPresentable* lastDeferredSourcePresentable{nullptr};
  idVec3 lastDeferredMuzzlePosition{};
  idMat3 lastDeferredFireAxis{};
  idFireParms* lastDeferredFireParms{nullptr};
  bool lastDeferredSecondary{false};
};

enum class aiSubWeb_t : int {
  relaxed = 0,
  combat = 1,
  hostile = 2,
  search = 3,
  pain = 4,
  painStagger = 5,
  painMoving = 6,
  painShock = 7,
  dying = 8,
  actionScript = 9,
  fullCover = 10,
  melee = 11,
  surprise = 12,
  dive = 13,
  dodge = 14,
  grenade = 15,
  throwAttack = 16,
  overrideAnimation = 17,
  turret = 18,
  remoteControl = 19,
  traversal = 20,
  zipline = 21,
  dropshipFall = 22,
  react = 23,
  max = 24,

  // Compatibility spelling used by the early manual validation harness.
  alert = search,
};

enum overrideAnim_t : int {
  ANIMOVERRIDE_NONE = 0,
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

struct idAIAnimatorInitRecord {
  idStr name;
  const void* declaration{nullptr};
  idTreeAnimator* treeAnimator{nullptr};
  int blendOp{0};
  int originBlend{0};
  int weightGroup{0};
  int filterGroup{0};
  int userGroup{0};
  float alpha{1.0F};
  unsigned int flags{0};
  bool initialized{false};
  bool proxyToBodyAnimWeb{false};
};

struct idAIAnimationInitRuntimeState {
  const void* twitchPainDecl{nullptr};
  const void* assignedTwitchPainDecl{nullptr};
  const void* rollBonesDecl{nullptr};
  const void* dragJointsDecl{nullptr};
  const void* freezeJointsDecl{nullptr};
  const void* walkIKDecl{nullptr};
  const void* walkIKModelDecl{nullptr};
  const void* walkIKClip{nullptr};
  const void* walkIKClipModel{nullptr};
  idStr bodySubWebName;
  idStr bodyStateName;
  std::vector<std::string> bodyActiveLayers;
  idAIAnimatorInitRecord bodyAnimator;
  idAIAnimatorInitRecord rollBonesAnimator;
  idAIAnimatorInitRecord faceAnimator;
  idAIAnimatorInitRecord dragJointsAnimator;
  idAIAnimatorInitRecord freezeJointsAnimator;
  idAIAnimatorInitRecord droneAnimator;
  idAIAnimatorInitRecord walkIKAnimator;
  idVec3 walkIKOrigin{};
  idMat3 walkIKAxis{};
  idVec3 walkIKUpNormal{0.0F, 0.0F, 1.0F};
  unsigned short eyeInfoHandle{0xFFFFU};
  int initTime{0};
  int ponytailCount{0};
  int initializedPonytailCount{0};
  std::vector<idEntity*> initializedPonytailOwners;
  int walkIKEntityNumber{-1};
  bool painInfoAssigned{false};
  bool twitchPainAliasesStrongLoaded{false};
  bool bodyInitialized{false};
  bool bodyEnabled{false};
  bool turretInitialized{false};
  bool faceManagerInitialized{false};
  bool walkIKInitialized{false};
};

class idDamageGroup {
public:
  struct goreLevelInfo_t {
    float healthRatio{0.0F};
    int goreLevel{0};
    bool requiresPain{false};
    bool enablePainAndDeath{true};
  };

  idStr groupName;
  float damageScale{1.0F};
  float maxHealth{0.0F};
  float health{0.0F};
  std::vector<goreLevelInfo_t> goreLevelInfo;
  float locationArmor{0.0F};
  idList<idStr, 5> hideMeshes;
  struct armorDestroyedMeshKit_t {
    int kitType{3};
    idStr name;
  } armorDestroyedMeshKit;
  idList<idStr, 5> enableHitGroups;
  idList<idStr, 5> disableHitGroups;
  const idDeclEntityDef* spawnEntity{nullptr};
  idStr tagName;
  const void* spawnParticle{nullptr};
  int popOffArmorFXCondition{0};
  const idSoundShader* spawnSound{nullptr};
  float HealthRatio() const;
};

class idProp_AIArmor : public idEntity {
public:
  void ConfigureForArmorPopOffTest(const char* propNameValue,
                                   const char* tagNameValue,
                                   bool inplaceModelValue,
                                   const idVec3& physicsOriginValue,
                                   const void* fxDeclarationValue) {
    propName = propNameValue != nullptr ? propNameValue : "prop";
    tagName = tagNameValue != nullptr ? tagNameValue : "";
    inplaceModel = inplaceModelValue;
    physicsOrigin = physicsOriginValue;
    fxDeclaration = fxDeclarationValue;
  }
  const char* GetPropNameForAI() const {
    return propName.IsEmpty() ? "prop" : propName.data;
  }
  const idStr& GetArmorTagNameForAI() const { return tagName; }
  bool IsInplaceModelForAI() const { return inplaceModel; }
  const idVec3& GetPhysicsOriginForAI() const { return physicsOrigin; }
  const void* GetFXDeclForAI() const { return fxDeclaration; }
  void SetModelTransformForAI(const idVec3& origin,
                              const idMat3& axis) {
    modelOrigin = origin;
    modelAxis = axis;
    ++setModelTransformCount;
  }
  void StartArmorFXForAI() { ++startFXCount; }
  const idVec3& GetModelOriginForTest() const { return modelOrigin; }
  const idMat3& GetModelAxisForTest() const { return modelAxis; }
  int GetSetModelTransformCountForTest() const {
    return setModelTransformCount;
  }
  int GetStartFXCountForTest() const { return startFXCount; }

private:
  idStr propName{"prop"};
  idStr tagName;
  bool inplaceModel{false};
  idVec3 physicsOrigin{};
  const void* fxDeclaration{nullptr};
  idVec3 modelOrigin{};
  idMat3 modelAxis{};
  int setModelTransformCount{0};
  int startFXCount{0};
};

struct idAIStaggerPainObservation {
  painType_t type{PAIN_NONE};
  short jointIndex{-1};
  const idEntity* attacker{nullptr};
  const idEntity* inflictor{nullptr};
  int painGroupHandle{0};
  damageDirection_t direction{DAMAGEDIR_NONE};
  damageIntensity_t intensity{DAMAGEINTENSITY_NONE};
  const idDeclDamage* damageDecl{nullptr};
  int setPainCount{0};
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

struct idTrajectory_Parabolic {
  void UpdateDeferredQueries(bool showTrajectories) {
    lastShowTrajectories = showTrajectories;
    ++deferredQueryUpdateCount;
  }
  int GetDeferredQueryUpdateCount() const {
    return deferredQueryUpdateCount;
  }
  bool GetLastShowTrajectories() const { return lastShowTrajectories; }

private:
  int deferredQueryUpdateCount{0};
  bool lastShowTrajectories{false};
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
  bool isBoss{true};
  bool bossRegistered{false};
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
  int immuneDoneTime{-1};
  int nextMTSPCheck{-1};
  int nextTurnfastTime{-1};
  int numberOfStages{1};
  int currentStage{0};
  int powerOutTriggerActivations{0};
  int chargingDoneTriggerActivations{0};
  int currentStageTriggerActivations{0};
  int cooldownEffectStartTime{-1};
  int cooldownEffectStopTime{-1};
  float shootHealthDrain{0.0F};
  float stageHealth{0.0F};
  float currentStageHealth{0.0F};
  float chargeHealthFraction{0.0F};
  float chargeTime{0.0F};
  float bossHealth{0.0F};
  idStr stageGroup{"battery"};
  const idSoundShader* turnfastSound{nullptr};
  idVec3 lastBodyDir{1.0F, 0.0F, 0.0F};
  int turnfastSoundPlayCount{0};
  idStr lastDebugText;
  idVec4 lastDebugColor{};
};

enum aiEffectUpdateType_t : int {
  AIEFFECTUPDATETYPE_NONE = 0,
  AIEFFECTUPDATETYPE_CHARGE = 1,
  AIEFFECTUPDATETYPE_HEALTH = 2,
};

class idProp_Loot : public idEntity {
public:
  void Activate(idEntity* activator, usableState_t usable) {
    lastActivator = activator;
    lastUsable = usable;
    ++activationCount;
  }
  int GetActivationCountForTest() const { return activationCount; }
  idEntity* GetLastActivatorForTest() const { return lastActivator; }
  usableState_t GetLastUsableForTest() const { return lastUsable; }

private:
  idEntity* lastActivator{nullptr};
  usableState_t lastUsable{USABLE_NONE};
  int activationCount{0};
};

class idProjectile : public idEntity {};

class idProjectile_Grenade : public idProjectile {
public:
  void SetDetonationTimeForTest(int value) { detonationTime = value; }
  int GetDetonationTime() const { return detonationTime; }

private:
  int detonationTime{0};
};

class idProp_Moveable : public idEntity {
public:
  void SetOwner(idEntity* owner);
  idEntity* GetOwnerForTest() const { return owner; }
  void RemoveFromWorld() { removedFromWorld = true; }
  bool WasRemovedFromWorldForTest() const { return removedFromWorld; }

private:
  idEntity* owner{nullptr};
  bool removedFromWorld{false};
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
  int GetLastReleaseTime() const { return lastReleaseTime; }
  int GetRequestTime() const { return requestTime; }
  void SetCurrentUser(idAI2* value) { currentUser = value; }
  idAI2* GetCurrentUser() const { return currentUser; }
  void SetTraversalFlagsForTest(unsigned int value) {
    traversalFlags = value;
  }
  unsigned int GetTraversalFlagsForTest() const { return traversalFlags; }

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
  unsigned int traversalFlags{0};
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
  void SetWorldStateTacticalFlagsForTest(unsigned int value) {
    worldStateTacticalFlags = value;
  }
  unsigned int GetWorldStateTacticalFlagsForTest() const {
    return worldStateTacticalFlags;
  }
  void SetPriorityFitnessForTest(float value) { priorityFitness = value; }
  float GetPriorityFitnessForTest() const { return priorityFitness; }

private:
  bool tactical{false};
  idEntityPtr<idProp_Moveable> dualWieldPickup;
  idEntity* turret{nullptr};
  int clearEnterAnimationsCount{0};
  idStr enterAnimation;
  int extraEnterAnimationCount{0};
  unsigned int worldStateTacticalFlags{0};
  float priorityFitness{0.0F};
};

enum aiMoveReason_t { AIMOVEREASON_NONE, AIMOVEREASON_SCRIPT, AIMOVEREASON_COMBAT };
enum aiArrivalAction_t { AIARRIVAL_NONE, AIARRIVAL_STOP, AIARRIVAL_FACE };

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

struct idAIDeathSystemObservation {
  int callCount{0};
  bool notCorpse{false};
  int timestamp{-1};
  int applyDeathAnimationCount{0};
  int fakeGrenadeRemovalCount{0};
  int stopTeslaCount{0};
  bool lastStopTeslaImmediate{false};
  int lastStopTeslaTime{-1};
  int meleeTraceEndCount{0};
  bool walkIKDeadKeepLastMods{false};
  int walkIKDisableAllCount{0};
  int walkIKEndCount{0};
  int walkIKEndTime{-1};
  int walkIKEndBlendTime{-1};
  bool walkIKEndReset{true};
  int checkpointKilled{-1};
  int setPainCount{0};
  painType_t painType{PAIN_NONE};
  short painJointIndex{-1};
  const idEntity* painAttacker{nullptr};
  const idEntity* painInflictor{nullptr};
  int painGroupHandle{0};
  damageDirection_t painDirection{DAMAGEDIR_NONE};
  damageIntensity_t painIntensity{DAMAGEINTENSITY_NONE};
  const idDeclDamage* painDamageDecl{nullptr};
  idEntity* focusOverrideEntity{nullptr};
  idEntity* deathSpawnedEntity{nullptr};
  idVec3 deathSpawnLinearVelocity{};
  idVec3 deathSpawnAngularVelocity{};
};

struct idAISpawnCorpseObservation {
  int updateVisualsCount{0};
  int walkIKDisableAllCount{0};
  bool walkIKDead{false};
  int becomeActiveCount{0};
  int lastBecomeActiveFlags{0};
  int startFromSavedCount{0};
  int savedBodyCount{0};
  int putAFToRestCount{0};
  int blendStackCount{0};
  const idAnimStack* blendStack{nullptr};
  int blendGameTime{0};
  int blendPreviousGameTime{0};
  int blendMillisecondsPerFrame{0};
  int blendGameTimePerSecond{0};
  int animationLookupCount{0};
  int firstAnimationLookupIndex{-1};
  int secondAnimationLookupIndex{-1};
  int movementModeSetCount{0};
  int requestedMovementMode{0};
  int moveStopCount{0};
  int requestedMoveStopStatus{0};
  int setFrameCount{0};
  const idMD6Anim* setFrameAnimation{nullptr};
  int setFrameIndex{0};
  bool setFrameForce{false};
  bool setFrameSync{false};
  int becomeInactiveCount{0};
  int lastBecomeInactiveFlags{0};
  int hideAttachmentsCount{0};
};

struct idAITeslaTraceState {
  bool valid{false};
  bool hasResult{false};
  float fraction{1.0F};
  idVec3 endPosition{};
  std::uint64_t queryIndex{0};
};

struct idAITeslaUpdateState {
  int resetTime{-1};
  bool useTeslaWhileBlinded{false};
  float minimumRange{0.0F};
  float explodeDelay{0.25F};
  const idDeclDamage* bombDamageDecl{nullptr};
  bool useBombGroundEffects{false};
  float inhibitFireTime{0.0F};
  float bombDangerZone{0.0F};
  const void* boltDecl{nullptr};
  float boltFlickerInterval{0.5F};
  const void* bombFXDecl{nullptr};
  bool bombFindPosition{false};
  int deferredEffect{0};
  idTreeAnimator::tagData_t teslaTag{};
  idTreeAnimator::tagData_t chargingBoltTag{};
  bool chargingBoltActive{false};
  int chargingBoltFlickerTime{0};
  float blindFireSpreadFactor{1.0F};
  idList<idVec3, 5> blindFireScriptPositions;
  idAITeslaTraceState groundTrace;
  idAITeslaTraceState bombTrace;
  idAITeslaTraceState blindFireTrace;
  idVec3 pendingBlindFireStart{};
  idVec3 pendingBlindFireEnd{};
};

struct idAITeslaUpdateObservation {
  int callCount{0};
  int stopTeslaCount{0};
  bool lastStopTeslaFire{false};
  int lastStopTeslaTime{-1};
  int spawnTeslaBombCount{0};
  int lastSpawnTeslaBombTime{-1};
  int radiusDamageCount{0};
  const idDeclDamage* lastRadiusDamageDecl{nullptr};
  idVec3 lastRadiusDamagePosition{};
  int deferredEffectStartCount{0};
  int bombEntityActivationCount{0};
  int clearDangerZoneCount{0};
  int groundQuerySubmitCount{0};
  idVec3 lastGroundQueryStart{};
  idVec3 lastGroundQueryEnd{};
  int chargingBoltStartCount{0};
  int chargingBoltUpdateNodesCount{0};
  int chargingBoltUpdateCount{0};
  int chargingBoltStopCount{0};
  int blindFireResetCount{0};
  int blindFireTraceResolveCount{0};
  int blindFireTraceSubmitCount{0};
  idVec3 lastBlindFireTraceStart{};
  idVec3 lastBlindFireTraceEnd{};
};

struct idAIArmorPopOffObservation {
  int callCount{0};
  int hideMeshAttemptCount{0};
  int unknownHideMeshCount{0};
  int hitGroupUpdateCount{0};
  int spawnRequestCount{0};
  idEntity* spawnedEntity{nullptr};
  bool tagFound{false};
  bool tagTransformResolved{false};
  int spawnedEntityTransformCount{0};
  idVec3 spawnedEntityOrigin{};
  idMat3 spawnedEntityAxis{};
  idVec3 ejectionDirection{};
  idVec3 linearVelocity{};
  idVec3 angularVelocity{};
  int armorModelTransformCount{0};
  int armorFXStartCount{0};
  int headEventCount{0};
  int particleSpawnCount{0};
  const void* lastParticleDeclaration{nullptr};
  int lastParticleTime{0};
  int lastParticleFrameMilliseconds{0};
  float lastParticleRandom{0.0F};
  idVec3 lastParticleOrigin{};
  idMat3 lastParticleAxis{};
  int popOffFXStartCount{0};
  int lastPopOffFXCondition{0};
  int soundStartCount{0};
  const idSoundShader* lastSound{nullptr};
  int lastSoundChannel{-1};
  unsigned int lastSoundFlags{0};
  unsigned int lastSoundPeerMask{0};
};

struct idAIUpdateDeathsPainObservation {
  int callCount{0};
  idEntity* inflictor{nullptr};
  idEntity* attacker{nullptr};
  const idDeclDamage* damageDecl{nullptr};
  float damage{0.0F};
  idVec3 impactPoint{};
  idVec3 impactDirection{};
  short jointIndex{-1};
  bool blind{false};
};

struct idAITakedownObservation {
  void Reset() {
    callCount = 0;
    activator = nullptr;
    requestedMode = TAKEDOWNMODE_NONE;
    resolvedMode = TAKEDOWNMODE_NONE;
    inputDualWield = false;
    reversedGrip = false;
    selectedDualWield = false;
    selectedName = false;
    takedownName.Clear();
    aiDestination = {};
    aiAxis = {};
    meleeTraceEndCount = 0;
    walkIKDisableAllCount = 0;
    walkIKEndCount = 0;
    walkIKEndTime = -1;
    walkIKEndBlendTime = -1;
    walkIKEndReset = true;
  }
  int callCount{0};
  idEntity* activator{nullptr};
  takedownMode_t requestedMode{TAKEDOWNMODE_NONE};
  takedownMode_t resolvedMode{TAKEDOWNMODE_NONE};
  bool inputDualWield{false};
  bool reversedGrip{false};
  bool selectedDualWield{false};
  bool selectedName{false};
  idStr takedownName;
  idVec3 aiDestination{};
  idMat3 aiAxis{};
  int meleeTraceEndCount{0};
  int walkIKDisableAllCount{0};
  int walkIKEndCount{0};
  int walkIKEndTime{-1};
  int walkIKEndBlendTime{-1};
  bool walkIKEndReset{true};
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
      struct idAnimWebAnimatorStorage {
        std::array<unsigned char, 0x478> bytes;
      };
      struct idTurretAnimatorStorage {
        std::array<unsigned char, 0x448> base;
        int lastBlendTriangle;
        std::array<unsigned short, 9> vertexAliasHandles;
        std::array<unsigned char, 2> tail;
      };
      template <std::size_t Size>
      struct idAnimatorStorage {
        std::array<unsigned char, Size> bytes;
      };
      struct tagData_t {
        idVec3 trans;
        idQuat rot;
        unsigned short parentJoint;
        unsigned short padding;
      };
      struct idSyncedAnimationInfo {
        int overrideAnim;
        int targetSpawnId;
      };

      idAIAnimationInfo();
      ~idAIAnimationInfo();

      int GetTurretLastBlendTriangleForTest() const {
        return turretAnimator.lastBlendTriangle;
      }
      unsigned short GetTurretAliasHandleForTest(int index) const {
        return turretAnimator.vertexAliasHandles[static_cast<std::size_t>(index)];
      }
      void SetTurretAliasHandleForInit(int index, unsigned short handle) {
        if (index >= 0 && index <
                              static_cast<int>(turretAnimator
                                                   .vertexAliasHandles.size())) {
          turretAnimator.vertexAliasHandles[static_cast<std::size_t>(index)] =
              handle;
        }
      }
      const tagData_t& GetViewTagDataForTest() const { return viewTagData; }
      int GetSyncedTargetSpawnIdForTest() const {
        return syncedAnim.targetSpawnId;
      }

      idAnimWebAnimatorStorage animWebAnimator;
      aiSubWeb_t subWeb;
      std::array<unsigned char, 4> subWebPadding;
      idTurretAnimatorStorage turretAnimator;
      idAnimatorStorage<0xB8> painAnimator;
      idAnimatorStorage<0xB8> shootAnimator;
      idAnimatorStorage<0x120> rollBonesAnimator;
      idAnimatorStorage<0xE0> faceAnimator;
      idAnimatorStorage<0x140> dragJointsAnimator;
      idAnimatorStorage<0xF0> freezeJointsAnimator;
      tagData_t viewTagData;
      aiMelee_t meleeFlags;
      aiMelee_t crouchMeleeFlags;
      idSyncedAnimationInfo syncedAnim;
      int ikEnableTime;
      std::array<unsigned char, 4> trailingPadding;
    };
    struct idAIScriptInfo {
      struct idScriptSpawnReference {
        int value;
      };
      idAIScriptInfo();
      ~idAIScriptInfo();

      int flags;
      idThread* thread;
      idStaticList<int, 4> threads;
      idStaticList<int, 4> fibers;
      const idAIEvent* currentAIEvent;
      int lastAIEventClass;
      idVec3 lastAIEventOrigin;
      idScriptSpawnReference lastAIEventOriginator;
      idScriptSpawnReference lastAIEventInstigator;
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

  struct idAnimWebDefaults {
    idAnimWebDefaults();
    ~idAnimWebDefaults();

    const idStr& GetWebPath() const { return webPath; }
    const idList<idAtomicString, 5>& GetActiveLayers() const {
      return activeLayers;
    }
    bool IsEnabled() const { return enabled; }
    void ConfigureForTest(const char* path, bool enabledValue,
                          std::initializer_list<const char*> layers) {
      webPath = path != nullptr ? path : "";
      activeLayers.Clear();
      for (const char* layer : layers) {
        activeLayers.Append(idAtomicString(layer != nullptr ? layer : ""));
      }
      enabled = enabledValue;
    }

  private:
    idStr webPath;
    idList<idAtomicString, 5> activeLayers;
    bool enabled{false};
  };

  struct idAIConstant {
    struct idContentsStorage {
      std::array<unsigned char, 3> bytes;
    };
    struct idAIPhysics {
      idVec3 playerCollisionSize;
      float heightCrouched;
      float aiPathfindRadius;
      idContentsStorage contents;
      idContentsStorage clipMask;
      idContentsStorage playerClipMask;
      idContentsStorage extendedClipMask;
    };
    struct idRangeStorage {
      float minimum;
      float maximum;
    };
    struct idEntityInfluenceTrailStorage {
      idVec3 offset;
      float innerRadius;
      float outerRadius;
      float angle;
      float trailDist;
      float trailFadeInTime;
      float trailFadeOutTime;
    };
    struct idAIMovement {
      idStr aasName;
      idRangeStorage moveSpeed;
      idRangeStorage bodyTurnRate;
      idRangeStorage moveTurnRate;
      float relaxedTurnRateScale;
      float accelRate;
      float decelRate;
      float alignmentTolerance;
      int defaultMovementMode;
      idEntityInfluenceTrailStorage influence;
      bool useMoveFSM;
      bool allowStrafing;
      bool allowHostileStance;
      bool requireWeaponForSearchCombat : 1;
      int modeRestricFlags;
    };
    struct idAIAnimation {
      const idDeclAnimWeb* animWebs[1];
      idAnimWebDefaults animWebDefaults[1];
      idStr eyeInfoName;
      idAtomicString viewTagName;
      bool useMoods : 1;
      bool useLipsync : 1;
      bool useGestures : 1;
      bool useEyeTracking : 1;
      bool useFaceSetupEyeTracking : 1;
      bool useBlinking : 1;
      bool useLidDeformation : 1;
      bool useFaceManager : 1;
      bool useFacialAnims : 1;
      bool useCombatWebOnly : 1;
      bool useRelaxedWebOnly : 1;
      bool useHandsRelaxed : 1;
      bool canUseTurret : 1;
      bool useAimOffsetAnims : 1;
      float focusTrackingYawOnlyOverride;
      bool useMoodsEnabled : 1;
      bool useLipsyncEnabled : 1;
      bool useGesturesEnabled : 1;
      bool useFacialAnimsEnabled : 1;
      bool useFaceSetupEyeTrackingEnabled : 1;
    };
    struct idAIOnlineScoring {
      bool heavy;
    };
    struct idAIStats {
      int killStat;
    };

    idAIConstant();
    ~idAIConstant();

    idAIPhysics physics;
    idAIMovement movement;
    idAIAnimation animation;
    idAIOnlineScoring onlineScoring;
    idAIStats stats;
  };

  struct idVisibilityObscurity {
    enum obscurity_t : int {
      OBSCURITY_UNKNOWN = 0,
      OBSCURITY_PATH_BLOCKED = 1,
      OBSCURITY_PATH_OPEN_WITH_OBSCURITY = 2,
      OBSCURITY_PATH_OPEN_WITH_COVER = 3,
      OBSCURITY_PATH_OPEN_AND_ENEMY_VISIBLE = 4,
      OBSCURITY_NUM = 5,
    };

    enum obscurityTest_t : int {
      OBSCURITYTEST_MUZZLE_POS = 0,
      OBSCURITYTEST_HALF_HEIGHT_FAR_LEFT = 1,
      OBSCURITYTEST_HALF_HEIGHT_FAR_RIGHT = 2,
      OBSCURITYTEST_HALF_HEIGHT_NEAR_LEFT = 3,
      OBSCURITYTEST_HALF_HEIGHT_NEAR_RIGHT = 4,
      OBSCURITYTEST_FULL_HEIGHT_FORWARD = 5,
      OBSCURITYTEST_FULL_HEIGHT_BACK = 6,
      OBSCURITYTEST_NUM = 7,
    };

    enum obscurityQuery_t : int {
      OBSCURITYQUERY_CROUCH_COVER = 0,
      OBSCURITYQUERY_DODGE_LEFT = 1,
      OBSCURITYQUERY_DIVE_LEFT = 2,
      OBSCURITYQUERY_DODGE_RIGHT = 3,
      OBSCURITYQUERY_DIVE_RIGHT = 4,
      OBSCURITYQUERY_DIVE_FWD = 5,
      OBSCURITYQUERY_DIVE_BACK = 6,
      OBSCURITYQUERY_MAX = 7,
    };

    idVec3 GetDestination(const idAI2* ai,
                          obscurityTest_t obscurityTest) const;
    void Update(idAI2* ai);

    obscurity_t GetObscurityForTest(obscurityTest_t test) const {
      return visibilityObscurity[static_cast<std::size_t>(test)];
    }
    const std::array<idClipQuery, OBSCURITYQUERY_MAX>&
    GetClipQueriesForTest() const {
      return clipQuery;
    }
    int GetLastSubmitFrameForTest() const { return lastSubmitFrame; }
    void SetLastSubmitFrameForTest(int value) { lastSubmitFrame = value; }
    bool HasAnyCoverForTest() const { return anyCover; }
    bool HasAnyClearLanesForTest() const { return anyClearLanes; }
    bool IsValidForTest() const { return valid; }

    std::array<idClipQuery, OBSCURITYQUERY_MAX> clipQuery{};
    std::array<obscurity_t, OBSCURITYTEST_NUM> visibilityObscurity{};
    int lastSubmitFrame{-1};
    bool anyCover{false};
    bool anyClearLanes{false};
    bool valid{false};
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
  void GetSubWebPrefix(aiSubWeb_t subWeb, idStr* output) const;
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
  bool GetSubWebAndStateFromAnimWebPath(const char* path, idStr& subWebName,
                                        idStr& stateName) const;
  bool GetAnimWebNodeForOverride(int overrideType, idAnimWebIndex* subWebIndex,
                                 idAnimWebIndex* stateIndex);
  const idAIAnimNodePlayback* GetAnimForAnimWebNode(const idAnimWebIndex* subWebIndex,
                                         const idAnimWebIndex* stateIndex);
  const idMD6Anim* GetAnimForAnimWebNode(
      const idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t>& subWebIndex,
      const idIndex<short, idDeclAnimWeb::invalidStateIndex_t>& stateIndex)
      const;
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
  bool GetMuzzleTransform(idWeapon* weapon, idVec3& muzzlePosition,
                          idMat3& muzzleAxis) const;
  bool GetMuzzlePosition(idWeapon* weapon, idVec3& muzzlePosition) const;
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
  void ActivateStealthKillTrigger();
  void AimDebug(const char* text, int level) const;
  void AlarmNotification();
  int AlertCycleToSubWeb(int alertCycle);
  idRecoveredValue AllocPresentable(...);
  bool AnimEventReceived(idAnimWebEvent::priority_t priority) const;
  idRecoveredValue ApplyDamage(...);
  idRecoveredValue ApplyDeathAnimationLogic(...);
  void ApplyFakeDamageImpulse(const idVec3* impulse);
  void ApplyForce(int physicsId, int bodyId, const idVec3* point,
                  const idVec3* force);
  void ApplyImpulse(int physicsId, int bodyId, const idVec3* point,
                    const idVec3* impulse);
  idRecoveredValue ApplyWaterEffects(...);
  bool AwareOfEnemy() const;
  void BeginSync();
  float CalcMovementTurnRate(const idVec3* idealDirection,
                             const idVec3* currentDirection);
  float CalcTurnRadius(float speed, float turnRate);
  idRecoveredValue CanBeTakenDown(...);
  bool CanFireRobot(int currentTime);
  bool CanReceiveAIEvents(int currentTime) const;
  bool CanTaunt() const;
  bool CanTeslaBlindFire(idAIEntityState* enemyState, int duration);
  idRecoveredValue CheckDeathCollision(...);
  bool CheckDelayedVoice(int currentTime);
  bool CheckForGore();
  bool CheckForGoreByName(const idAtomicString* goreName);
  bool CheckForOneShotKill(idEntity* attacker);
  bool CheckForSuspectedToTrailUpdate(int currentTime, bool force);
  idRecoveredValue CheckPain(...);
  void ClearAIClip();
  void ClearAimFocus();
  void ClearAllAnimWebEvents(aiAnimWeb_t web,
                             idAnimWebEvent::priority_t priority);
  void ClearAnimWebEvent(aiAnimWeb_t web,
                         idAnimWebEvent::priority_t priority,
                         animWebEvent_t eventType);
  void ClearFakeEnemy();
  void ClearLookFocus();
  void ClearSyncedAnimData();
  void ClearWaitForOverrideAnim(idAnimWebEvent::priority_t priority);
  void ClearWorldState();
  idRecoveredValue Damage(...);
  idRecoveredValue DamageFeedback(...);
  void DamageGroupPopOffArmor(idDamageGroup* damageGroup,
                              idVec3 impactPoint, idVec3 impactDirection,
                              float damage, bool& outArmorPoppedOff);
  float DamageRobot(idEntity* attacker, float damageDone,
                    idDamageGroup* damageGroup);
  void DeactivateExtendedClipModel();
  bool DeathPending() const;
  void Debug_Perception();
  idRecoveredValue Debug_ScenePoints(...);
  void DisableAIEventResponse(int flags);
  void DisableAnimatorsOnDeath();
  void DisableInfluenceTrail();
  void DoBlockedMeleeReaction();
  bool DoEmergencyCoverExit();
  idRecoveredValue DoMeleeDamage(...);
  idRecoveredValue DoMeleeDamage(idRecoveredOverloadTag<1>);
  idRecoveredValue DormantBegin(...);
  idRecoveredValue DormantEnd(...);
  void DoSpawnSettings();
  bool Draw(idPlayer* player);
  void Dropped(idEntity* user, const idDeclInventory* inventoryDeclaration);
  void EnableAIEventResponse(int flags);
  void EndActionScript();
  void EndSync();
  bool EquipItem(idInventoryItem* item, equipSlot_t equipSlot);
  eventVoid* Event_DetectPlayer(eventVoid* result);
  void FindAutoFocus();
  float FindAutoFocusEntity(idEntity** bestEntity);
  const idWeapon* FindWeaponToDraw() const;
  idRecoveredValue FinishFireWeapon(...);
  idRecoveredValue FirstThink(...);
  void ForceStopOverrideAnim(idAnimWebEvent::priority_t priority,
                             int blendDurationMs);
  const idAAS2* GetAAS() const;
  idAASPositionMgr* GetAASPositionMgr();
  idLegsFSM* GetActionFSM();
  int GetActionStatus() const;
  alertCycle_t GetAlertCycle() const;
  int GetAlertCycleDuration() const;
  idFiniteStateMachine* GetAlertCycleFSM();
  bool GetAnimationDelta(const idMD6Anim* animation, idVec3& outDelta) const;
  bool GetAnimationDelta(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t> subWeb,
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t> state,
      idVec3& outDelta, bool verifyAWPath) const;
  bool GetAnimationDelta(const idAnimWebPath& animPath, idVec3& outDelta,
                         bool verifyAWPath) const;
  bool GetAnimationDeltaAndRot(const idAnimWebPath& animPath,
                               idVec3& outDelta, idAngles& outRotation,
                               bool verifyAWPath) const;
  idRecoveredValue GetAnimationDuration(...);
  bool GetAnimationPathReachability(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t> subWeb,
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t> state) const;
  bool GetAnimationPathReachability(const idAnimWebPath& animPath) const;
  bool GetAnimationRot(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t> subWeb,
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t> state,
      idAngles& outRotation, bool verifyAWPath) const;
  bool GetAnimationRot(const idAnimWebPath& animPath,
                       idAngles& outRotation, bool verifyAWPath) const;
  idAIVolatile::idAIAnimationInfo* GetAnimatorAnimWeb();
  idAIOrientation* GetBodyOrientation();
  void GetBurstDuration(const idWeapon* weapon, int* minimum,
                        int* maximum) const;
  idComponentActionScript* GetComponentTimeLine();
  idPlayer* GetControllingPlayer() const;
  int GetCoverActionDuration(coverAction_t action, int currentTime);
  idRecoveredValue GetCoverTransitionRadius(...);
  const idCombatStage* GetCurrentCombatStage() const;
  float GetDefaultSpeedByCurrentWalkState() const;
  aiDirection_t GetDirectionTo(const idVec3* position,
                               bool fourDirectionLimit);
  aiDirection_t GetDirectionToForReactions(const idVec3* position);
  idFormationCoverMgr* GetDynamicCoverMgr();
  int GetEffectiveAccuracy() const;
  bool GetEnableHeadTracking();
  idEncounterGroup* GetEncounterGroup() const;
  bool GetEnemyInsideGoNutsRange() const;
  bool GetEnemyInsideMaxFireRange(const idEntity* enemy) const;
  float GetEnemyLookDot(const idEntity* enemy) const;
  const aiSenseState_t* GetEnemySenseState(aiSense_t senseType) const;
  unsigned int GetEventResponseFlags();
  const idVec3 GetEyeOffset() const override;
  const idDeclFacialAnimationSet* GetFacialAnimationSet() const;
  const idEntity* GetFakeEnemy() const;
  float GetFakeEnemyWeight();
  void GetFireAtLastKnownDuration(const idWeapon* weapon, int& minimum,
                                  int& maximum);
  idAIHealth* GetHealthComponent();
  int GetIdleIndexForType(runIndexType_t type) const;
  inputSettings_t* GetInputSettings(inputSettings_t* result,
                                    idPlayer* player);
  idJointIndex GetJointIndexFromTrace(const trace_t* trace) const;
  idRecoveredValue GetMeleeDirection(...);
  int GetMeleeInterval(aiMelee_t meleeType);
  idRecoveredValue GetMeleeTypeForTarget(...);
  idVec3 GetMissTimeAimPosition(const idEntity* target, idVec3 fireOrigin,
                                const idMat3& fireAxis,
                                bool trackAlongGround,
                                float missTimeFractionLeft) const;
  void GetModelTransform(idVec3* modelOrigin, idMat3* modelAxis) const;
  int GetModifiedSkill();
  idAIMoveState* GetMoveInterface();
  idAIOrientation* GetMoveOrientation();
  int GetOnlineUsableState();
  float GetPlayerViewDataFitness() const;
  idCodeResource<idDeclRenderParm>* GetRenderParam(aiRenderParam_t parameter);
  void GetRepullTriggerInterval(const idWeapon* weapon, int* minimum,
                                int* maximum) const;
  int GetRunCycleIndexForType(runIndexType_t type) const;
  void GetSoundTransform(idVec3* modelOrigin, idMat3* modelAxis) const override;
  float GetSpotGainedFireTime() const;
  float GetSpotLostFireTime() const;
  int GetSpottedDuration(const idEntity* enemy, int currentTime) const;
  bool GetSuspectedEnemyPos_PushedIntoAAS(idVec3* aiPosition);
  bool GetTakedownName(const idList<idStr, 5>* takedownList,
                       idStr* takedownName);
  int GetTauntInterval();
  int GetUsableState(const idEntity* activator, const idFocusTrace* focusTrace);
  idRecoveredValue GetViewTransform(...);
  idAIVoiceController* GetVoiceController(voiceController_t controller);
  walkState_t GetWalkState() const;
  int GetWeaponSlot() const;
  const idBounds* GetWorldCollisionBounds();
  void GiveAwarenessOfEntity(const idEntity* entity,
                             alertCycle_t alertCycle);
  idRecoveredValue HandleAffirmativeResponse(...);
  void HandleLoot(bool fullLoot, idEntity* item, idEntity* activator);
  idRecoveredValue HandlePendingAIEvents(...);
  bool HasFocus();
  bool HasSyncedWalks() const;
  bool HideMeshNoAutoRestore(const char* meshName);
  void InhibitRunning(bool inhibit);
  idRecoveredValue InitAAS(...);
  void InitEncounterGroup();
  void InitFireControl();
  void InitFSMs();
  idRecoveredValue InitHeadTracking(...);
  void InitMovement();
  void InitPerception(alertCycle_t cycle);
  idRecoveredValue InitPhysics(...);
  void InitScriptObjects();
  void InteractionCameraDeactivateCallback(idPlayer* player);
  idEventArg* InternalCallEvent(idEventArg* result,
                                const idEventDef* event,
                                idEventArg* arguments);
  bool InternalRespondsTo(const idEventDef* event);
  idRecoveredValue IsAASBlocked(...);
  bool IsBlindFiring() const;
  bool IsBlockedByFriendly(const idEntity* enemy);
  bool IsCrosshairSubdued(const idEntity* activator,
                          const idFocusTrace* focusTrace,
                          usableState_t usable) const;
  bool IsDead() const;
  bool IsDowned(float delay) const;
  bool IsDying() const;
  bool IsFacingMe(const idEntity* entity, float dotThreshold);
  bool IsIncapacitated() const;
  bool IsInCover() const;
  bool IsInsideTrailUpdateRange(const idVec3* suspiciousPosition) const;
  bool IsModesAllowed(aiModeRestricFlag_t flags) const;
  bool IsModesRestricted(aiModeRestricFlag_t flags) const;
  bool IsMoveDone(std::int64_t checkAlignment = 0) const;
  idRecoveredValue IsOnElevator(...);
  bool IsOverrideAnimSupported(overrideAnim_t anim) const;
  bool IsPlayerControlled() const;
  bool IsRecentlyGroupSpotted(const idEntity* enemy, float time) const;
  bool IsRecentlyHeard(const idAIEntityState* entityState, float time) const;
  bool IsRecentlySpawned(float seconds) const;
  bool IsRecentlySpotted(const idAIEntityState* entityState, float time) const;
  bool IsRecentlySpotted(const idEntity* enemy, float time) const;
  bool IsTargetInMeleeRange(const idVec3* targetPosition,
                            aiMelee_t meleeType);
  bool IsTeslaDangerous(idVec3* position, float* radius) const;
  bool IsThrowingGrenade() const;
  bool IsWeaponEquipped() const;
  void JobSync() override;
  void KillAI();
  void KilledNotification(idEntity* victim, idEntity* inflictor,
                          const idDeclDamage* damageDecl,
                          float damage = 0.0F,
                          bool killedByDualWield = false,
                          bool coverKill = false) override;
  void LeaveActionNode(bool force);
  bool LKWBlind();
  void LookDebug(const char* text, int level, int duration) const;
  unsigned int MeleeMaskForTarget(const idEntity* target);
  bool ModifyCrosshairInfo(const idEntity* activator,
                           const idFocusTrace* focusTrace,
                           usableState_t usable,
                           idCrosshairInfo* info) override;
  bool OkToLKWBlind() const;
  void OnActionScriptFinished(int currentTime);
  void OnActivate(idEntity* activator) override;
  idRecoveredValue OnAIEvent(...);
  void OnAIEvent_Body(idAIEventSphere* event);
  idRecoveredValue OnAIEvent_Investigate(...);
  idRecoveredValue OnAIEvent_PlayerAim(...);
  void OnAIEvent_PlayerInteraction(idAIEventSphere* event);
  idRecoveredValue OnAIEvent_Projectile(...);
  idRecoveredValue OnAIEvent_VehicleSound(...);
  idRecoveredValue OnHide(...);
  idRecoveredValue OnIgnoreDamage(...);
  idRecoveredValue OnShow(...);
  bool PerceptionFlagIsSet(int flags);
  void PerformCrouchAction();
  void PerformOverrideAnimAction(int overrideAnim, bool holdFireUntilDone);
  void PerformStandUpAction();
  void PlayAdditiveAlarmAnimation();
  void PlayAdditiveFireAnim();
  idRecoveredValue PlayAdditivePain(...);
  bool PlayAdditivePainAnim(const aiAnimationHandle_t* painAlias);
  idRecoveredValue PlayAnimWebPath(...);
  idRecoveredValue PlayAnimWebPathPerfect(...);
  void PlayDeathSound();
  void PlayHurtSound();
  idRecoveredValue PushFromCollision(...);
  bool RelinquishControl();
  void ReloadWeapon(idWeapon* weapon);
  idRecoveredValue Remove(...);
  bool RespondsToAIEvent(const idAIEvent* event) const;
  bool RobotBatteryExplodes(idEntity* attacker);
  void SaveFSM(idFile_String* file, const char* fsmName,
               const char* indent);
  void SendDeathTrigger();
  void SendGrappleActivationTest(idEntity* target);
  void SetAccuracy(aiAccuracy_t accuracy);
  void SetActionNodeGroupName(const char* groupName);
  void SetActionScript(const idList<idScriptAction, 5>* actions,
                       idEntity* source, idEntity* activator);
  void SetActionScriptFlag(int flags, bool setFlag);
  void SetAIClip();
  void SetAimFocusAimPoint(aimPoint_t point);
  void SetAimFocusEntity(const idEntity* entity, bool keepFocusInView,
                         aimPoint_t aimPoint, int timeout);
  void SetAlertCycle(alertCycle_t alertCycle);
  void SetAnimWebEvent(aiAnimWeb_t web,
                       idAnimWebEvent::priority_t priority,
                       idAnimWebIndex subWebIndex,
                       idAnimWebIndex stateIndex,
                       animWebEvent_t eventType,
                       const idEventDef* eventDefinition);
  bool SetBlinded(bool state = true, bool forceForever = false,
                  int blindedOverrideTimeMilliseconds = -1);
  void SetBodyOrientation(const idMat3& axis);
  void SetCoverAction(coverAction_t action, int currentTime);
  void SetDelayedVoice(voiceMsg_t message, float delay);
  idRecoveredValue SetEMPStunned(...);
  void SetEnableAutoFocus(bool enable);
  void SetEnableHeadTracking(bool enable);
  void SetFakeEnemy(idEntity* entity);
  void SetFOV(const float* fieldOfView);
  void SetIdealAimFocusPoint(const idVec3& point, int timeout,
                             aiFocus_t focusType);
  void SetIdealLookFocusPoint(const idVec3& point, int timeout,
                              aiFocus_t focusType);
  void SetLockHeadTracking(bool lock);
  void SetLookFocusEntity(const idEntity* entity, bool keepFocusInView,
                          aimPoint_t aimPoint, int timeout);
  idRecoveredValue SetModel(...);
  void SetMovePushStatus(aiMovePushStatus_t newStatus,
                         std::int64_t frameCount);
  void SetPerceptionFlags(int flags, bool setFlag);
  void SetRobotChargingDone();
  void SetRobotPowerOut();
  void SetSleeping(bool sleep = true);
  void SetSolid(bool isSolid);
  void SetSpeakingVO();
  bool SetSubWeb(aiSubWeb_t subWeb);
  void SetTakeDownWindow(bool open);
  void SetupFakePosition(const idEntity* instigator,
                         const idVec3& fakePosition, bool detected);
  void SetWalkState(walkState_t state);
  void SetWorldCollision(bool collideWithWorld);
  bool ShouldDeadThink() const;
  bool ShouldEnterDormancy() const override;
  aiHoldFireReason_t ShouldHoldFire(const idEntity* enemy);
  bool ShouldLeaveDormancy() const override;
  bool ShouldReloadWeapon(idWeapon* weapon, bool secondary);
  bool ShouldScramble();
  idRecoveredValue ShowFlareEffect(...);
  idRecoveredValue Spawn(...);
  void SpawnCorpse(const idCorpseManager::corpseSave_t* corpse);
  idRecoveredValue SpawnFlareEffect(...);
  idRecoveredValue SpawnTeslaBomb(...);
  void StartActionScript(int gameTime);
  void StartBodyDeathFade();
  void StartDisintigrateDeathFade();
  idAnimator_AF::testSolidResult_t StartRagdoll(
      const idRagdollInfo* info, const char* ragdollReason);
  idAnimator_AF::testSolidResult_t StartRagdoll(
      const idRagdollInfo* info) override;
  idRecoveredValue StartReloading(...);
protected:
  void StartDeathSystem(bool notCorpse);
  bool StartStaggeringPain(const idAIDamageInfo& info);
  void UpdateAvoidCharacter();
  void UpdateWorldState();
public:
  int StartWeaponFX(fxCondition_t condition);
  void StopFireRobot(int currentTime);
  void StopRagdoll();
  idRecoveredValue StopTesla(...);
  void StopWeaponFX(fxCondition_t condition);
  void Takedown(idEntity* activator, bool dualWield, bool reversedGrip,
                takedownMode_t takeDownMode);
  bool TakedownWindoIsOpen(idEntity* activator);
  void Teleport(const idVec3* origin, idAngles* angles);
  idRecoveredValue TeleportPush(...);
  bool TeslaIsActive() const;
  bool TestFireWeapon(idWeapon* weapon, idFireParms& fireParms,
                      idTestFireResults& testFireResults,
                      const testFireWeaponParms_t& testParms);
  testForBlockedTraceResult_t TestForBlockedTrace(
      const idFireParms& fireParms,
      const idTestFireResults& testFireResults,
      const testForBlockedTraceParms_t& testParms) const;
  idRecoveredValue Think(...);
  idRecoveredValue Think_AIControl(...);
  idRecoveredValue Think_Impl(...);
  idRecoveredValue Think_PlayerControl(...);
  void ThinkRobot(int currentTime);
  void TurretFire();
  bool TurretShouldFire(idWeapon* weapon);
  bool UnequipItem(idInventoryItem* item);
  void UpdateActions(int curTime);
  bool UpdateAnimationControllers();
  idRecoveredValue UpdateAttachments(...);
  void UpdateBodyOrientationWhileMoving();
  void UpdateDeathMovement();
  static void UpdateDeaths();
  idRecoveredValue UpdateDrop(...);
  idRecoveredValue UpdateEnemyInAAS(...);
  idRecoveredValue UpdateExtendedClipModel(...);
  idRecoveredValue UpdateFSMs(...);
  idRecoveredValue UpdateGoreLevel(...);
  idRecoveredValue UpdateGrappleActivation(...);
  void UpdateGroupStatusConditions();
  idRecoveredValue UpdateLookFocus(...);
  void UpdateMeleeCapabilities();
  void UpdateMovement();
  idRecoveredValue UpdatePathToEnemy(...);
  idRecoveredValue UpdateRenderParamBlends(...);
  idRecoveredValue UpdateSpriteAnimator(...);
  void UpdateTesla();
  idRecoveredValue UpdateWetness(...);
  bool Use(idEntity* activator, usableState_t usable);
  bool UsesSmoothAnimations() const;
  bool UsesTurrets() const;
  bool WaitForAnimToEnd(idAnimWebEvent::priority_t priority,
                        const char* subWebName, const char* stateName,
                        animWebEvent_t eventType);
  bool WaitForAnimToStart(aiAnimWeb_t lookupWeb,
                          idAnimWebEvent::priority_t priority,
                          const char* subWebName, const char* stateName,
                          animWebEvent_t eventType);
  bool WaitForAnimToStartVia(aiAnimWeb_t web,
                             idAnimWebEvent::priority_t priority,
                             idAnimWebIndex destinationSubWeb,
                             idAnimWebIndex destinationState,
                             idAnimWebIndex viaSubWeb,
                             idAnimWebIndex viaState,
                             animWebEvent_t eventType);
  bool WaitForAnimToStartVia(aiAnimWeb_t web,
                             idAnimWebEvent::priority_t priority,
                             const char* destinationSubWebName,
                             const char* destinationStateName,
                             const char* viaSubWebName,
                             const char* viaStateName);
  bool WaitForOverrideAnim(idAnimWebEvent::priority_t priority,
                           overrideAnim_t overrideAnimation,
                           animWebEvent_t eventType, bool force,
                           bool randomizeAnimation,
                           const idEntity* syncedTarget);
  idRecoveredValue WaitForTraversal(...);

  void StartDeathSystemForTest(bool notCorpse) {
    StartDeathSystem(notCorpse);
  }
  void ConfigureDeathInfoForTest(const idEntity* attacker,
                                 const idEntity* inflictor,
                                 const idDamageImpulse& impulse,
                                 int startDeathTime,
                                 unsigned int deathFlags);
  idAIDeathInfo* GetDeathInfoForTest() const { return deathInfo.get(); }
  void ConfigureDeathSystemForTest(bool goredFullBodyValue,
                                   bool useDeadPhysicsValue,
                                   bool usesTeslaValue,
                                   bool volatileGrenadeFlagValue,
                                   bool firstAlarmValue,
                                   bool commanderValue,
                                   int removeTimeValue) {
    goredFullBody = goredFullBodyValue;
    deathUseDeadPhysics = useDeadPhysicsValue;
    deathUsesTesla = usesTeslaValue;
    deathVolatileGrenadeFlag = volatileGrenadeFlagValue;
    firstAlarm = firstAlarmValue;
    isCommander = commanderValue;
    deathRemoveTime = removeTimeValue;
  }
  void ConfigureDeathTriggersForTest(idEntity* fullBodyTrigger,
                                     idEntity* dyingTrigger,
                                     idEntity* teslaFXEntity) {
    deathTrigger = fullBodyTrigger;
    triggerOnDying = dyingTrigger;
    teslaBombFXEntity = teslaFXEntity;
  }
  void ConfigureDeathSpawnDefinitionsForTest(
      const idDeclEntityDef* focusDefinition,
      std::initializer_list<const char*> focusRequirements,
      const idDeclEntityDef* spawnedDeathDefinition);
  void SetDeathBehaviorBodyEventForTest(const void* declaration) {
    deathBehaviorBodyEventDecl = declaration;
  }
  void SetEnemyKilledVOEventForTest(const void* declaration) {
    enemyKilledVOEventDecl = declaration;
  }
  unsigned int GetKilledNotificationFlagsForTest() const {
    return killedNotificationFlags;
  }
  void SetKilledNotificationFlagsForTest(unsigned int value) {
    killedNotificationFlags = static_cast<std::uint8_t>(value);
  }
  void SetDeathPainGroupHandleForTest(int value) {
    deathPainGroupHandle = value;
  }
  void SetDeathModelAxisForTest(const idMat3& value) {
    deathModelAxis = value;
  }
  void SetWalkIKActiveForDeathTest(bool value) { walkIKActive = value; }
  void SetDroneUseForDeathTest(bool value) { usesDrone = value; }
  const idAIDeathSystemObservation& GetDeathSystemObservationForTest() const {
    return deathSystemObservation;
  }
  void ResetDeathSystemObservationForTest() {
    deathSystemObservation = {};
  }
  idEntity* GetFocusOverrideEntityForTest() const {
    return focusOverrideEntity;
  }
  bool IsFlareFadingOutForDeathTest() const { return flareFadeOutEffect; }
  int GetCheckpointKilledForTest() const { return checkpointKilled; }
  bool GetWalkIKDeadKeepLastModsForTest() const {
    return walkIKDeadKeepLastMods;
  }
  void TakedownForTest(idEntity* activator, bool dualWield,
                       bool reversedGrip, takedownMode_t takeDownMode) {
    Takedown(activator, dualWield, reversedGrip, takeDownMode);
  }
  takedownTypes_t& GetTakedownTypesForTest() { return takedownTypes; }
  const idAITakedownObservation& GetTakedownObservationForTest() const {
    return takedownObservation;
  }
  void ResetTakedownObservationForTest() { takedownObservation.Reset(); }
  bool GetCutNextMeleeDamageForTest() const { return cutNextMeleeDamage; }
  void SetMeleeTraceActiveForTakedownTest(bool value) {
    animEventState.meleeTraceActive = value;
  }
  void SetDroppedItemsForUseTest(idEntity* first, idEntity* second) {
    droppedItem1 = first;
    droppedItem2 = second;
  }
  void SetAttachedLootForCrosshairTest(idEntity* value) {
    crosshairAttachedLoot = value;
  }
  void ConfigureUseHealthForTest(float current, float maximum);
  idAnimator_AF* GetUseAFForTest() { return GetAFMutable(); }
  void UpdateAvoidCharacterForTest() { UpdateAvoidCharacter(); }
  void SetCanBeBlockedByFriendsForTest(bool value) {
    canBeBlockedByFriends = value;
  }
  void SetFriendlyBlockerBindAnimationForTest(bool value) {
    hasFriendlyBlockerBindAnimation = value;
  }
  void SetCurrentEnemySuspectedPositionForTest(const idVec3& value,
                                               bool valid);
  idAnimator_Drone* GetHoldFireDroneAnimatorForTest() const {
    return robotDeathDroneAnimator.get();
  }

  void SetVoiceOverPlaying(bool value) { voiceOverPlaying = value; }
  bool IsVoiceOverPlaying() const { return voiceOverPlaying; }
  idAIVoiceController& GetLocalVoiceController() {
    return *localVoiceController;
  }
  idAICamera_Control& GetPlayerControlCamera() {
    return *playerControlCamera;
  }
  idFaceMgr& GetFaceManager() { return *faceManager; }
  void SetPresentable(idPresentable* value) { presentable = value; }
  idPresentable* GetPresentable() const { return presentable; }
  void ConfigureSpawnSettingsForTest(bool detectCombat,
                                     bool detectSearch,
                                     const idSoundShader* ambientSound,
                                     const char* entranceAnimationPath,
                                     bool playerLeadsSquad) {
    spawnDetectPlayerCombat = detectCombat;
    spawnDetectPlayerSearch = detectSearch;
    spawnAmbientSound = ambientSound;
    spawnEntranceAnimationPath =
        entranceAnimationPath != nullptr ? entranceAnimationPath : "";
    spawnPlayerIsSquadLeader = playerLeadsSquad;
  }
  void ClearSpawnMissingArmorForTest() { spawnMissingArmor.Clear(); }
  void AddSpawnMissingArmorForTest(const char* damageGroupName) {
    spawnMissingArmor.Append(
        idStr(damageGroupName != nullptr ? damageGroupName : ""));
  }
  idDamageGroup& AddDamageGroupForTest(const char* damageGroupName,
                                       float armor) {
    idDamageGroup group;
    group.groupName = damageGroupName != nullptr ? damageGroupName : "";
    group.locationArmor = armor;
    staggerDamageGroups.push_back(group);
    return staggerDamageGroups.back();
  }
  void SetSpawnAnimationStackForTest(const idAnimStack* value) {
    spawnAnimationStack = value;
  }
  void SetSpawnPlayerGenerationForTest(int value) {
    spawnPlayerGeneration = value;
  }
  int GetSpawnAwarenessCycleForTest() const {
    return spawnAwarenessCycle;
  }
  int GetSquadLeaderSpawnIdForTest() const {
    return squadLeaderSpawnId;
  }
  int GetSpawnAmbientSoundStartCountForTest() const {
    return spawnAmbientSoundStartCount;
  }
  int GetSpawnAmbientSoundChannelForTest() const {
    return spawnAmbientSoundChannel;
  }
  const idSoundShader* GetLastSpawnAmbientSoundForTest() const {
    return lastSpawnAmbientSound;
  }
  unsigned int GetSpawnAmbientSoundFlagsForTest() const {
    return spawnAmbientSoundFlags;
  }
  unsigned int GetSpawnAmbientSoundPeerMaskForTest() const {
    return spawnAmbientSoundPeerMask;
  }
  void SetAnimWebNodeAnimationIndexForTest(int index,
                                           const idMD6Anim* animation) {
    if (index < 0 ||
        index >= static_cast<int>(animWebState.nodeAnimations.size())) {
      return;
    }
    animWebState.nodeAnimations[static_cast<std::size_t>(index)].animation =
        animation;
  }
  const idAISpawnCorpseObservation& GetSpawnCorpseObservationForTest() const {
    return spawnCorpseObservation;
  }
  idAIScriptProxy& GetScriptProxy() { return scriptProxy; }
  const idAIScriptProxy& GetScriptProxy() const { return scriptProxy; }
  const idAIAnimEventState& GetAnimEventState() const { return animEventState; }
  void SetMeleeWeapon(idWeapon* value) { meleeWeapon = value; }
  void SetEquippedWeapon(idWeapon* value) {
    meleeWeapon = value;
    idActor::SetEquippedWeapon(0, value);
    SetActiveEquippedWeapon(value);
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
  void SetHasScriptObjectForTest(bool value) { hasScriptObject = value; }
  idThread* GetPrimaryScriptThreadForTest() const {
    return primaryScriptThread;
  }
  idAIUtilityState& GetUtilityState() { return utilityState; }
  const idAIUtilityState& GetUtilityState() const { return utilityState; }
  int GetAnimatedEntityJobSyncCount() const {
    return animatedEntityJobSyncCount;
  }
  const idTrajectory_Parabolic& GetTrajectoryForTest(int index) const {
    return trajectories[static_cast<std::size_t>(index)];
  }
  idFSMManager* GetFSMManagerForTest() const { return fsmManager.get(); }
  idAIActionManager* GetActionManagerForTest() const {
    return actionManager.get();
  }
  idAIFSMCallback* GetFSMCallbackForTest() const {
    return fsmCallback.get();
  }
  idFaceMgr* GetFaceManagerForTest() const { return faceManager.get(); }
  void SetGreetingsEnabledForTest(bool value) { greetingsEnabled = value; }
  const std::string& GetLastLookDebugTextForTest() const {
    return lastLookDebugText;
  }
  int GetLastLookDebugLevelForTest() const { return lastLookDebugLevel; }
  int GetLastLookDebugDurationForTest() const {
    return lastLookDebugDuration;
  }
  const idVec3& GetLastLookDebugPositionForTest() const {
    return lastLookDebugPosition;
  }
  int GetLookDebugCountForTest() const { return lookDebugCount; }
  void ClearLookDebugForTest() const {
    lastLookDebugText.clear();
    lastLookDebugLevel = 0;
    lastLookDebugDuration = 0;
    lastLookDebugPosition = {};
    lookDebugCount = 0;
  }
  int GetLastSignalForTest() const { return lastSignal; }
  int GetSignalCountForTest() const { return signalCount; }
  const idAICoreState& GetCoreState() const { return coreState; }
  idAIRobotRuntimeState& GetRobotState() { return robotState; }
  const idAIRobotRuntimeState& GetRobotState() const { return robotState; }
  void SetTurretUsage(bool animationSupportsTurrets,
                      bool actionSettingsUseTurrets) {
    turretAnimationSupported = animationSupportsTurrets;
    turretActionSetting = actionSettingsUseTurrets;
    if (aiConstants != nullptr) {
      aiConstants->animation.canUseTurret = animationSupportsTurrets;
    }
  }
  void ConfigureAnimationWebForTest(
      const idDeclAnimWeb* declaration, const char* defaultPath,
      bool enabled, std::initializer_list<const char*> activeLayers) {
    aiConstants->animation.animWebs[0] = declaration;
    aiConstants->animation.animWebDefaults[0].ConfigureForTest(
        defaultPath, enabled, activeLayers);
  }
  void ConfigureAnimationNamesForTest(const char* eyeInfoName,
                                      const char* viewTagName) {
    aiConstants->animation.eyeInfoName =
        eyeInfoName != nullptr ? eyeInfoName : "";
    aiConstants->animation.viewTagName.Set(
        viewTagName != nullptr ? viewTagName : "");
  }
  void ConfigureAnimationCapabilitiesForTest(
      bool moods, bool lipsync, bool gestures, bool eyeTracking,
      bool faceSetupEyeTracking, bool blinking, bool lidDeformation,
      bool faceManagerEnabled, bool facialAnimations,
      bool turretAnimations) {
    auto& animation = aiConstants->animation;
    animation.useMoods = moods;
    animation.useLipsync = lipsync;
    animation.useGestures = gestures;
    animation.useEyeTracking = eyeTracking;
    animation.useFaceSetupEyeTracking = faceSetupEyeTracking;
    animation.useBlinking = blinking;
    animation.useLidDeformation = lidDeformation;
    animation.useFaceManager = faceManagerEnabled;
    animation.useFacialAnims = facialAnimations;
    animation.canUseTurret = turretAnimations;
    turretAnimationSupported = turretAnimations;
  }
  void ConfigureAnimationOptionalChannelsForTest(
      const void* twitchPain, const void* rollBones, const void* dragJoints,
      const void* freezeJoints, const void* walkIKDeclaration,
      int ponytails, bool drone) {
    auto& state = *animationInitState;
    state.twitchPainDecl = twitchPain;
    state.rollBonesDecl = rollBones;
    state.dragJointsDecl = dragJoints;
    state.freezeJointsDecl = freezeJoints;
    state.walkIKDecl = walkIKDeclaration;
    state.ponytailCount = std::max(0, ponytails);
    usesDrone = drone;
  }
  void ConfigureWalkIKBindingsForTest(const void* clip,
                                      const void* clipModel) {
    animationInitState->walkIKClip = clip;
    animationInitState->walkIKClipModel = clipModel;
  }
  bool InitAnimationForTest() { return InitAnimation(); }
  const idAIAnimationInitRuntimeState& GetAnimationInitStateForTest() const {
    return *animationInitState;
  }
  idAIAnimationInitRuntimeState& GetAnimationInitStateForTest() {
    return *animationInitState;
  }
  const idAIConstant& GetAIConstantsForTest() const { return *aiConstants; }
  const idAIVolatile::idAIAnimationInfo& GetAnimationInfoForTest() const {
    return *animationInfo;
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
  const idVec3& GetTeslaExplodePositionForTest() const {
    return teslaExplodePosition;
  }
  void SetTeslaEvaluationTime(int time) { teslaEvaluationTime = time; }
  idAITeslaUpdateState& GetTeslaUpdateStateForTest() {
    return teslaUpdateState;
  }
  const idAITeslaUpdateState& GetTeslaUpdateStateForTest() const {
    return teslaUpdateState;
  }
  const idAITeslaUpdateObservation& GetTeslaUpdateObservationForTest() const {
    return teslaUpdateObservation;
  }
  int GetTeslaBlindFireTimeStampForTest() const {
    return teslaBlindFireTimeStamp;
  }
  void SetArmorPopOffHeadEventForTest(const void* declaration) {
    armorPopOffHeadEventDecl = declaration;
  }
  const idAIArmorPopOffObservation&
  GetArmorPopOffObservationForTest() const {
    return armorPopOffObservation;
  }
  const idList<idEntityPtr<idEntity>, 5>&
  GetSpawnedDetachablesForTest() const {
    return spawnedDetachables;
  }
  const idList<idStr, 5>& GetEnabledArmorHitGroupsForTest() const {
    return enabledArmorHitGroups;
  }
  const idList<idStr, 5>& GetDisabledArmorHitGroupsForTest() const {
    return disabledArmorHitGroups;
  }
  static void SetActiveAIForUpdateDeathsTest(
      std::initializer_list<idAI2*> activeAI);
  void SetUpdateDeathsDeactivatedForTest(bool value) {
    updateDeathsDeactivated = value;
  }
  void SetUpdateDeathsAdditivePainEnabledForTest(bool value) {
    updateDeathsAdditivePainEnabled = value;
  }
  void SetHealthDeadForUpdateDeathsTest(bool value);
  const idAIUpdateDeathsPainObservation&
  GetUpdateDeathsPainObservationForTest() const {
    return updateDeathsPainObservation;
  }
  int GetGoreDeathsUpdateCountForTest() const;
  int GetAIClipMask() const { return aiClipMask; }
  idAIFireControl* GetFireControl() const { return fireControl.get(); }
  void ConfigureTestFireAimForTest(float compensationTime,
                                   bool predictMovement,
                                   float normalAimDot,
                                   float upsideDownAimDot,
                                   float gangstaPoseAimDot,
                                   bool trackMissAlongGround) {
    focusCompensationTime = compensationTime;
    predictTargetMovement = predictMovement;
    testFireAimDot = normalAimDot;
    testFireAimDotUpsideDown = upsideDownAimDot;
    testFireGangstaAimDot = gangstaPoseAimDot;
    missTrackAlongGround = trackMissAlongGround;
  }
  void SetTestFireMuzzleContentsForTest(std::uint32_t value) {
    testFireMuzzleContents = value;
  }
  void SetTestFireLinearVelocityForTest(const idVec3& value) {
    testFireLinearVelocity = value;
  }
  idVec3 GetLinearVelocity() const override { return testFireLinearVelocity; }
  void SetAimDebugEyeOffsetForTest(const idVec3& value) {
    aimDebugEyeOffset = value;
  }
  void ClearAimDebugForTest() const {
    lastAimDebugText.clear();
    lastAimDebugLevel = 0;
    lastAimDebugPosition = {};
    aimDebugCount = 0;
  }
  const std::string& GetLastAimDebugTextForTest() const {
    return lastAimDebugText;
  }
  int GetLastAimDebugLevelForTest() const { return lastAimDebugLevel; }
  const idVec3& GetLastAimDebugPositionForTest() const {
    return lastAimDebugPosition;
  }
  int GetAimDebugCountForTest() const { return aimDebugCount; }
  void SetCurrentAnimWebStateNameForTest(const char* value) {
    animWebState.movementState = value != nullptr ? value : "";
  }
  void ClearMissTimeDebugForTest() const {
    missTimeDebugPointCount = 0;
    missTimeDebugLineCount = 0;
    lastMissTimeDebugPoint = {};
  }
  int GetMissTimeDebugPointCountForTest() const {
    return missTimeDebugPointCount;
  }
  int GetMissTimeDebugLineCountForTest() const {
    return missTimeDebugLineCount;
  }
  const idVec3& GetLastMissTimeDebugPointForTest() const {
    return lastMissTimeDebugPoint;
  }
  void SetAccuracyState(int groupCondition, int editableAccuracy,
                        int skillAccuracy) {
    groupStatusCondition = groupCondition;
    editableAimAccuracy = editableAccuracy;
    skillAimAccuracy = skillAccuracy;
  }
  void ConfigureGroupStatusInputs(bool coverValid,
                                  const idVec3& coverOrigin,
                                  int firedAtTime) {
    groupCurrentCoverValid = coverValid;
    groupCurrentCoverOrigin = coverOrigin;
    groupLastFiredAtTime = firedAtTime;
  }
  int GetGroupStatusConditionForTest() const {
    return groupStatusCondition;
  }
  void ConfigureTraversalClassesForTest(bool classA, bool classB,
                                        bool classC, bool classD,
                                        bool classE) {
    useTraversalClassA = classA;
    useTraversalClassB = classB;
    useTraversalClassC = classC;
    useTraversalClassD = classD;
    useTraversalClassE = classE;
  }
  void UpdateTravelFlagsForTest() { UpdateTravelFlags(); }
  std::uint32_t GetTravelFlagsForTest() const { return aasTravelFlags; }
  void SetTravelFlagsForTest(std::uint32_t value) { aasTravelFlags = value; }
  const std::string& GetLastTravelDebugTextForTest() const {
    return lastTravelDebugText;
  }
  const idVec3& GetLastTravelDebugPositionForTest() const {
    return lastTravelDebugPosition;
  }
  int GetTravelDebugCountForTest() const { return travelDebugCount; }
  void ConfigureDeadEffectForTest(bool active, int duration,
                                  bool suppressBodyRemoval) {
    deadEffectActive = active;
    deadEffectDuration = duration;
    noBodyRemoval = suppressBodyRemoval;
  }
  void ConfigureFlareEffectForTest(
      int handle, aiEffectUpdateType_t updateType, bool killed,
      bool fadeOut, float fadeTimerValue, float fadeScale,
      const idLookupTable* table = nullptr) {
    flareHandle = handle;
    flareUpdateType = updateType;
    flareHasBeenKilled = killed;
    flareFadeOutEffect = fadeOut;
    flareFadeTimer = fadeTimerValue;
    flareFadeOutScale = fadeScale;
    flareFadeTable = table;
  }
  void ConfigureEffectsTagForTest(idTreeAnimator* animator,
                                  const idTreeAnimator::tagData_t& tag) {
    effectsTreeAnimator = animator;
    flareTagData = tag;
  }
  void ConfigureEffectsHealthForTest(float currentHealth,
                                     float maximumHealth);
  void SetEffectsRobotGlowForTest(float value);
  void ConfigureEffectsFadeForTest(int timestamp, int duration,
                                   int deathStartTimeValue) {
    fadeOutTimestamp = timestamp;
    fadeOutDuration = duration;
    deathStartTime = deathStartTimeValue;
  }
  void UpdateEffectsForTest() { UpdateEffects(); }
  bool IsDeadEffectActiveForTest() const { return deadEffectActive; }
  int GetDeadEffectDurationForTest() const { return deadEffectDuration; }
  bool IsFlareKilledForTest() const { return flareHasBeenKilled; }
  bool IsFlareFadingForTest() const { return flareFadeOutEffect; }
  float GetFlareFadeTimerForTest() const { return flareFadeTimer; }
  float GetEffectsRobotGlowForTest() const { return effectsRobotGlow; }
  const idVec4& GetEffectsFlareColorForTest() const {
    return effectsFlareColor;
  }
  idFXManager* GetEffectsFXManagerForTest() const {
    return effectsFXManager.get();
  }
  idPresentable* GetEffectsPresentableForTest() const {
    return presentable;
  }
  int GetEffectsUpdateFXCountForTest() const {
    return effectsUpdateFXCount;
  }
  void ConfigureBodyOrientationMovementForTest(
      float minimumSpeed, float maximumSpeed, float minimumTurnRate,
      float maximumTurnRate, float relaxedScale, bool strafingAllowed) {
    aiConstants->movement.moveSpeed = {minimumSpeed, maximumSpeed};
    aiConstants->movement.bodyTurnRate = {minimumTurnRate,
                                          maximumTurnRate};
    aiConstants->movement.relaxedTurnRateScale = relaxedScale;
    aiConstants->movement.allowStrafing = strafingAllowed;
  }
  void ConfigureMovementTurnRateForTest(float minimumSpeed,
                                        float maximumSpeed,
                                        float minimumTurnRate,
                                        float maximumTurnRate,
                                        float relaxedScale) {
    aiConstants->movement.moveSpeed = {minimumSpeed, maximumSpeed};
    aiConstants->movement.moveTurnRate = {minimumTurnRate,
                                          maximumTurnRate};
    aiConstants->movement.relaxedTurnRateScale = relaxedScale;
  }
  void ConfigureBodyRotationForTest(bool enabled, bool suppressed) {
    bodyRotationEnabled = enabled;
    suppressBodyRotation = suppressed;
  }
  void SetRunFacingForwardForTest(bool value) {
    movementBehaviorRunsFacingForward = value;
  }
  int GetLastFireTime() const { return lastFireTime; }
  void SetAIEvaluationTime(int time) { aiEvaluationTime = time; }
  void SetArticulatedFigureState(bool loaded, bool active,
                                 bool clientAuthoritativeActive) {
    articulatedFigureLoaded = loaded;
    articulatedFigureActive = active;
    articulatedFigureClientActive = clientAuthoritativeActive;
  }
  void SetArticulatedFigureAtRestForTest(bool value) {
    articulatedFigureAtRest = value;
  }
  void ConfigureDormancyGatesForTest(bool immediatePhysicsGate,
                                     bool activeAnimWebTransition) {
    immediatePhysicsDormancyGate = immediatePhysicsGate;
    animWebTransitionActive = activeAnimWebTransition;
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
  int GetLKWBlindEffectCount() const { return lkwBlindEffectCount; }
  void ConfigureLKWBlindState(int reblindCooldownEnd, int blindedEnd,
                              int stunnedEnd) {
    endReblindCooldown = reblindCooldownEnd;
    endBlindedTime = blindedEnd;
    endStunnedTime = stunnedEnd;
  }
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
  void SetEncounterGroupNameForTest(const char* value) {
    encounterGroupName = value != nullptr ? value : "";
  }
  void SetEncounterGroupHandleForTest(std::int16_t value) {
    encounterGroupHandle = value;
  }
  std::int16_t GetEncounterGroupHandleForTest() const {
    return encounterGroupHandle;
  }
  void ConfigureBehaviorEventDeclarationsForTest(
      const idDeclAiEvent* memberKilled,
      const idDeclAiEvent* losingBattle) {
    behaviorEventDeclarations.memberKilledVOEventDecl = memberKilled;
    behaviorEventDeclarations.losingBattleVOEventDecl = losingBattle;
    behaviorEventDeclarationsConfigured = true;
  }
  void InvokeOnAIEventVoiceForTest(const idAIEventVoice* event) {
    OnAIEvent_Voice(event);
  }
  void ConfigureFootstepPerceptionForTest(bool deaf, float hearingStimulus,
                                          float combatTimeout) {
    perceptionDeaf = deaf;
    perceptionHearingStimulus = hearingStimulus;
    perceptionCombatTimeoutInterval = combatTimeout;
  }
  void InvokeOnAIEventFootstepForTest(const idAIEventSound* event) {
    OnAIEvent_Footstep(event);
  }
  void ConfigureGunfirePerceptionForTest(bool shootingSenseCheat,
                                         float firedAtCosine,
                                         bool drone) {
    shootingSenseUpdateCheat = shootingSenseCheat;
    firedAtCosAngleOff = firedAtCosine;
    usesDrone = drone;
  }
  void ConfigureGunfireAlarmForTest(idEntity* alarmTrigger,
                                    bool commander, bool alarmWasFirst,
                                    std::uint32_t pending,
                                    std::uint32_t forbidden) {
    onAlarmTrigger = alarmTrigger;
    isCommander = commander;
    firstAlarm = alarmWasFirst;
    pendingReactions = pending;
    forbiddenReactions = forbidden;
  }
  void InvokeOnAIEventGunfireForTest(const idAIEventSound* event) {
    OnAIEvent_Gunfire(event);
  }
  void ConfigureExplosionBehaviorEventDeclarationForTest(
      const idDeclAiEvent* heardSomething) {
    behaviorEventDeclarations.heardSomethingVoiceEventDecl = heardSomething;
    behaviorEventDeclarationsConfigured = true;
  }
  void InvokeOnAIEventExplosionForTest(const idAIEventSound* event) {
    OnAIEvent_Explosion(event);
  }
  void ConfigureProjectileImpactBehaviorEventDeclarationForTest(
      const idDeclAiEvent* heardSomething) {
    behaviorEventDeclarations.heardSomethingVoiceEventDecl = heardSomething;
    behaviorEventDeclarationsConfigured = true;
  }
  void InvokeOnAIEventProjectileImpactForTest(
      const idAIEventSound* event) {
    OnAIEvent_ProjectileImpact(event);
  }
  void ConfigureDeadBodyLandBehaviorEventDeclarationForTest(
      const idDeclAiEvent* heardSomething) {
    behaviorEventDeclarations.heardSomethingVoiceEventDecl = heardSomething;
    behaviorEventDeclarationsConfigured = true;
  }
  void InvokeOnAIEventDeadBodyLandForTest(const idAIEventSound* event) {
    OnAIEvent_DeadBodyLand(event);
  }
  void InvokeOnAIEventDoorMotionForTest(const idAIEventSound* event) {
    OnAIEvent_DoorMotion(event);
  }
  void ConfigureDeploySoundBehaviorEventDeclarationForTest(
      const idDeclAiEvent* heardSomething) {
    behaviorEventDeclarations.heardSomethingVoiceEventDecl = heardSomething;
    behaviorEventDeclarationsConfigured = true;
  }
  void InvokeOnAIEventDeploySoundForTest(const idAIEventSound* event) {
    OnAIEvent_DeploySound(event);
  }
  void ConfigureGrenadeDetectionForTest(bool detectInAir) {
    detectGrenadesInAir = detectInAir;
  }
  void InvokeOnAIEventGrenadeForTest(
      const idAIEventProjectedSphere* event) {
    OnAIEvent_Grenade(event);
  }
  void ConfigureVehiclePerceptionForTest(bool ignoreFriendly) {
    ignoreFriendlyVehicles = ignoreFriendly;
  }
  void InvokeOnAIEventVehicleForTest(const idAIEvent_Vehicle* event) {
    OnAIEvent_Vehicle(event);
  }
  std::uint32_t GetPendingReactionsForTest() const {
    return pendingReactions;
  }
  std::uint32_t GetForbiddenReactionsForTest() const {
    return forbiddenReactions;
  }
  idAIWorldState& GetWorldState() { return *worldState; }
  const idAIWorldState& GetWorldState() const { return *worldState; }
  idCheckSurroundingsState& GetCheckSurroundingsStateForTest() {
    return *checkSurroundingsState;
  }
  idAIMemory& GetMemory() { return *memory; }
  const idAIMemory& GetMemory() const { return *memory; }
  void InvokeUpdateWorldStateForTest() { UpdateWorldState(); }
  void ConfigureWorldStateSleepingForTest(bool usesSleep, bool sleeping,
                                          float radius) {
    usesSleeping = usesSleep;
    coreState.sleeping = sleeping;
    sleepRadius = radius;
  }
  void ConfigureWorldStatePerceptionForTest(bool perceptionDuringScript,
                                            std::uint64_t flags) {
    perceptionWhenScripted = perceptionDuringScript;
    coreState.perceptionFlags = flags;
  }
  void ConfigureWorldStateTrackingForTest(const trackingParms_t& look,
                                          const trackingParms_t& aim) {
    worldLookTrackingParms = look;
    worldAimTrackingParms = aim;
  }
  void ConfigureWorldStateLookScenePointsForTest(bool usesLook,
                                                 float combatDelaySeconds,
                                                 float idleTalkPeriodValue) {
    worldUsesLookScenePoints = usesLook;
    combatTargetDelayLook = combatDelaySeconds;
    idleTalkPeriod = idleTalkPeriodValue;
  }
  void ConfigureWorldStateTargetKindsForTest(bool kampHund, bool robot,
                                             bool drone) {
    isKampHund = kampHund;
    robotState.usesRobot = robot;
    usesDrone = drone;
  }
  void ConfigureWorldStateTriggersForTest(idEntity* alarm,
                                          idEntity* detected,
                                          idEntity* spotted,
                                          bool onlyPlayerSpots) {
    onAlarmTrigger = alarm;
    onDetectEnemyTrigger = detected;
    onSpotEnemyTrigger = spotted;
    onlyPlayerTriggersOnSpot = onlyPlayerSpots;
  }
  void SetWorldStateReactionTimeForTest(int value) {
    enemyReactionTime = value;
  }
  int GetWorldStateAimDivergeCountForTest() const {
    return worldStateAimDivergeCount;
  }
  int GetWorldStateAffirmativeResponseCountForTest() const {
    return worldStateAffirmativeResponseCount;
  }
  int GetWorldStateLastAffirmativeVoiceForTest() const {
    return worldStateLastAffirmativeVoice;
  }
  void SetAAS(const idAAS2* value) { aas = value; }
  const void* GetAIGroupIdentity() const override {
    return aiEventEncounterGroup;
  }
  void SetEncounterLastSeenTime(int value) { encounterLastSeenTime = value; }
  int GetEncounterLastSeenTime() const { return encounterLastSeenTime; }
  int GetTauntUseCount() const { return tauntUseCount; }
  int GetDelayedVoiceType() const { return delayedVoiceType; }
  int GetDelayedVoiceTime() const { return delayedVoiceTime; }
  void ConfigureBehaviorHeaderState(int weaponSlotValue,
                                    bool hasSyncedWalksValue) {
    behaviorWeaponSlot = weaponSlotValue;
    behaviorHasSyncedWalks = hasSyncedWalksValue;
  }
  void ConfigureBodyDeathFade(float seconds, int timestamp = -1) {
    bodyFadeTime = seconds;
    fadeOutTimestamp = timestamp;
  }
  int GetFadeOutDuration() const { return fadeOutDuration; }
  int GetFadeOutTimestamp() const { return fadeOutTimestamp; }
  int GetStackedTakedownWindows() const { return stackedTakedownWindows; }
  int GetTakedownWindowClosedTime() const { return takedownWindowClosed; }
  void ConfigureTakedownWindowCheck(bool restrictToBehind,
                                    int windowClosedTime) {
    takedownRestrictedToBehind = restrictToBehind;
    takedownWindowClosed = windowClosedTime;
  }
  int GetCounterBlockedUntil() const { return counterBlockedUntil; }
  bool WasStunnedByPlayerBlock() const { return stunnedByPlayerBlock; }
  void SetStunnedByPlayerBlock(bool value) {
    stunnedByPlayerBlock = value;
  }
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
  void ConfigurePerception(float idleRadius, float searchRadius,
                           float actorRadius, float fieldOfView,
                           float targetingRange) {
    actorIdlePerceptionRadius = idleRadius;
    actorSearchPerceptionRadius = searchRadius;
    actorPerceptionRadius = actorRadius;
    configuredFieldOfView = fieldOfView;
    maxFireRange = targetingRange;
  }
  float GetMaximumFireRangeForTest() const { return maxFireRange; }
  float GetFieldOfViewDot() const { return fieldOfViewDot; }
  bool IsAutoFocusEnabled() const { return autoFocusEnabled; }
  bool IsHeadTrackingLocked() const { return headTrackingLocked; }
  const idFocusInfo* GetFocusInfo(int index) const {
    return index >= 0 && index < static_cast<int>(focusInfos.size())
               ? focusInfos[static_cast<std::size_t>(index)].get()
               : nullptr;
  }
  idFocusInfo* GetFocusInfoForAnimationTest(int index) {
    return index >= 0 && index < static_cast<int>(focusInfos.size())
               ? focusInfos[static_cast<std::size_t>(index)].get()
               : nullptr;
  }
  void ConfigureAnimationControllersForTest(
      bool usesDroneValue, bool headTrackingEnabled,
      bool faceManagerEnabled, bool corpseAnimationBlocked,
      int ikEnableTimeValue) {
    usesDrone = usesDroneValue;
    animEventState.headTracking = headTrackingEnabled;
    aiConstants->animation.useFaceManager = faceManagerEnabled;
    corpseAnimationControllerBlocked = corpseAnimationBlocked;
    animationInfo->ikEnableTime = ikEnableTimeValue;
  }
  const idMat3& GetAnimationModelAxisForTest() const {
    return animationModelAxis;
  }
  idAnimator_Drone* GetAnimationControllerDroneForTest() const {
    return animationControllerDroneAnimator.get();
  }
  idTreeAnimator* GetAnimationControllerTreeForTest() const {
    return effectsTreeAnimator;
  }
  int GetAnimationControllerIKEnableTimeForTest() const {
    return animationInfo->ikEnableTime;
  }
  idFocusInfo* GetFocusInfo(int index) {
    return index >= 0 && index < static_cast<int>(focusInfos.size())
               ? focusInfos[static_cast<std::size_t>(index)].get()
               : nullptr;
  }
  void UpdateAimFocusForTest() { UpdateAimFocus(); }
  void SetupRobotDeadForTest() { SetupRobotDead(); }
  bool FindClosestFocusForTest() { return FindClosestFocus(); }
  void ConfigureAutoFocusForTest(int entityMode, int surroundingsMode,
                                 int minimumTime, int maximumTime) {
    autoFocusEntityMode = entityMode;
    autoFocusSurroundingsMode = surroundingsMode;
    autoFocusMinimumTime = minimumTime;
    autoFocusMaximumTime = maximumTime;
  }
  int GetLookFocusTimeout() const { return lookFocusTimeout; }
  void SetActionScriptRunning(bool running);
  int GetActionScriptRunningQueryCountForTest() const;
  void ResetActionScriptRunningQueryCountForTest();
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
    if (aiConstants != nullptr) {
      aiConstants->animation.useMoods = mood;
      aiConstants->animation.useLipsync = lipSync;
      aiConstants->animation.useGestures = gesture;
      aiConstants->animation.useEyeTracking = eyeTracking;
      aiConstants->animation.useBlinking = blinkingValue;
      aiConstants->animation.useLidDeformation = lidDeformationValue;
    }
  }
  void ConfigureCurrentScenePoint(idScenePoint* value, int activationTime) {
    currentScenePoint = value;
    currentScenePointActivationTime = activationTime;
  }
  void SetDefaultPhysicsClipMask(int value) { defaultPhysicsClipMask = value; }
  void SetDefaultPhysicsContentsForTest(std::uint32_t value) {
    defaultPhysicsContents = value;
  }
  bool IsSolidForTest() const { return solidState; }
  aiMovePushStatus_t GetMovePushStatusForTest() const {
    return movePushStatus;
  }
  int GetMovePushEndTimeForTest() const { return movePushEndTime; }
  int GetDefaultPhysicsClipMaskForTest() const {
    return defaultPhysicsClipMask;
  }
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
  void ConfigurePlayerControlForTest(idPlayer* player,
                                     int possessionEvent,
                                     const idAngles& viewAngles) {
    controllingPlayer = player;
    playerControlPossessionEvent = possessionEvent;
    playerControlViewAngles = viewAngles;
  }
  int GetPlayerControlPossessionEventForTest() const {
    return playerControlPossessionEvent;
  }
  void SetCurrentEnemy(idEntity* value) { currentEnemy = value; }
  void SetSafeReloadFractionForTest(float value) {
    safeReloadFraction = value;
  }
  idEntity* GetCurrentEnemy() const { return currentEnemy; }
  const idDeclInventory* GetInventoryDeclarationForTest() const {
    return inventoryDecl;
  }
  void SetDeathTriggerForTest(idEntity* trigger) { deathTrigger = trigger; }
  idEntity* GetDeathTriggerForTest() const { return deathTrigger; }
  void SetCurrentActionNodeForTest(idEntity* actionNode) {
    currentActionNode = actionNode;
  }
  idEntity* GetCurrentActionNodeForTest() const { return currentActionNode; }
  const char* GetActionNodeGroupNameForTest() const {
    return actionNodeGroupName.data;
  }
  void SetStealthKillTriggerForTest(idEntity* trigger) {
    stealthKillTrigger = trigger;
  }
  idEntity* GetStealthKillTriggerForTest() const {
    return stealthKillTrigger;
  }
  void ConfigureOneShotKillForTest(int state, bool ignoreDamage,
                                   float nonPlayerChance) {
    dropState = state;
    ignoreDamageWhileDropping = ignoreDamage;
    nonPlayerOneShotChance = nonPlayerChance;
  }
  void ConfigureRagdollForTest(bool enabled, bool suppressed,
                               const idVec3& pastVelocity) {
    ragdollEnabled = enabled;
    ragdollSuppressed = suppressed;
    dronePastVelocity = pastVelocity;
  }
  const char* GetRagdollReasonForTest() const { return reasonForRagdoll.data; }
  idEntity* GetFakeEnemyEntity() const { return fakeEnemy; }
  void ConfigureAlertCycleState(int cycle, int timestamp) {
    aiEventState.alertCycle = cycle;
    alertCycleTimestamp = timestamp;
  }
  void ConfigureAlertCycleDetailsForTest(int currentCycle,
                                         int previousCycle,
                                         int timestamp,
                                         int highestCycle,
                                         int surprisedTime,
                                         bool forceCombat) {
    aiEventState.alertCycle = currentCycle;
    previousAlertCycle = previousCycle;
    alertCycleTimestamp = timestamp;
    highestAlertCycle = highestCycle;
    lastSurprisedTime = surprisedTime;
    alwaysInCombat = forceCombat;
  }
  int GetPreviousAlertCycleForTest() const { return previousAlertCycle; }
  int GetHighestAlertCycleForTest() const { return highestAlertCycle; }
  int GetLastSurprisedTimeForTest() const { return lastSurprisedTime; }
  aiSubWeb_t GetCurrentSubWebForTest() const { return animWebState.subWeb; }
  aiModeRestricFlag_t GetMovementRestrictionFlagsForTest() const {
    return movementModeRestrictionFlags;
  }
  void ConfigureSmoothAnimations(bool relaxed, bool search, bool combat) {
    usesSmoothAnimRelaxed = relaxed;
    usesSmoothAnimSearch = search;
    usesSmoothAnimCombat = combat;
  }
  void ConfigureForcedSubWebs(bool relaxed, bool combat) {
    forceRelaxedSubWeb = relaxed;
    forceCombatSubWeb = combat;
  }
  void ConfigureMovementInitialization(int defaultMode, float maxSpeed,
                                       float acceleration,
                                       float deceleration) {
    defaultMovementMode = defaultMode;
    initialMaxMoveSpeed = maxSpeed;
    initialAccelRate = acceleration;
    initialDecelRate = deceleration;
  }
  void ConfigureMeleeAnimationFlags(unsigned int standingFlags,
                                    unsigned int crouchingFlags) {
    meleeAnimationFlags = standingFlags;
    crouchMeleeAnimationFlags = crouchingFlags;
  }
  void ConfigureFireTiming(bool useFireTiming, float minimumBurstDuration,
                           float maximumBurstDuration,
                           float minimumBurstPause,
                           float maximumBurstPause) {
    usesConfiguredFireTiming = useFireTiming;
    configuredMinBurstDuration = minimumBurstDuration;
    configuredMaxBurstDuration = maximumBurstDuration;
    configuredMinBurstPause = minimumBurstPause;
    configuredMaxBurstPause = maximumBurstPause;
  }
  void SetAttachmentFXManagerForTest(idFXManager* effects) {
    attachmentFXManager = effects;
  }
  idFXManager* GetFXMgrForAttachment(const idWeapon* weapon) const {
    (void)weapon;
    return attachmentFXManager;
  }
  idFXManager* GetFXMgrForAttachment() const {
    return attachmentFXManager;
  }
  void SetRunIndexType(runIndexType_t value) { currentRunIndexType = value; }
  void SetInjuredTime(int value) { injuredTime = value; }
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
  unsigned int GetMeleeAnimationFlagsForTest() const {
    return meleeAnimationFlags;
  }
  void SetActionScriptCount(int value) { actionScriptCount = value; }
  int GetActionScriptCount() const { return actionScriptCount; }
  bool IsActionScriptRunning() const { return actionScriptRunning; }
  bool IsPainAllowedAfterActionScript() const {
    return painAllowedAfterActionScript;
  }
  void SetGoreAvailable(const char* goreName, bool available);
  void ConfigureGoreCheckForTest(bool result, bool fullBody,
                                 std::initializer_list<short> joints);
  idAttachmentCollection* GetAttachmentsForGoreTest() {
    return &attachments;
  }
  idPonytail* AddPonytailForGoreTest(int attachmentHandle);
  bool IsGoredFullBodyForTest() const { return goredFullBody; }
  int GetGoreCheckQueryCountForTest() const;
  int GetGoreWalkIKDisableCountForTest() const {
    return goreWalkIKDisableCount;
  }
  int GetGoreAttachmentSnapshotCountForTest() const {
    return goreAttachmentSnapshotCount;
  }
  int GetGoreWeaponFlagClearCountForTest() const {
    return goreWeaponFlagClearCount;
  }
  int GetGorePonytailShutdownCountForTest() const {
    return gorePonytailShutdownCount;
  }
  void SetPlayingEntranceAnimation(bool value) {
    isPlayingEntranceAnim = value;
  }
  bool IsPlayingEntranceAnimation() const { return isPlayingEntranceAnim; }
  const idAIDamageInfo* GetDamageInfoForTest() const {
    return damageInfo.get();
  }
  idAIDamageInfo* GetDamageInfoForTestMutable() { return damageInfo.get(); }
  bool StartStaggeringPainForTest(const idAIDamageInfo& info) {
    return StartStaggeringPain(info);
  }
  void ConfigureStaggerPainRuntimeForTest(
      bool canBecomeInjuredValue, bool hasLongPainAnimationsValue,
      bool staggeringPainActiveValue, float longPainDamageThresholdValue,
      float longPainMinimumIntervalValue, int painEndTimeValue,
      int postureChangeTimeValue) {
    canBecomeInjured = canBecomeInjuredValue;
    hasLongPainAnimations = hasLongPainAnimationsValue;
    staggeringPainActive = staggeringPainActiveValue;
    longPainDamageThreshold = longPainDamageThresholdValue;
    longPainMinInterval = longPainMinimumIntervalValue;
    painEndTime = painEndTimeValue;
    postureChangeTime = postureChangeTimeValue;
  }
  void ConfigureStaggerPainJointGroupsForTest(
      int painHandle, const char* painGroupName, int damageHandle,
      const char* damageGroupName, const char* jointName) {
    staggerPainGroupHandle = painHandle;
    staggerPainGroupName = painGroupName != nullptr ? painGroupName : "";
    staggerDamageGroupHandle = damageHandle;
    staggerDamageGroupName =
        damageGroupName != nullptr ? damageGroupName : "";
    staggerJointName = jointName != nullptr ? jointName : "";
  }
  void ConfigureStaggerPainInjuryGroupForTest(const char* groupName,
                                               int goreLevelIndex);
  void ClearStaggerPainDamageGroupsForTest() {
    staggerDamageGroups.clear();
  }
  void AddStaggerPainDamageGroupForTest(const char* groupName,
                                        bool enablePainAndDeath) {
    idDamageGroup group;
    group.groupName = groupName != nullptr ? groupName : "";
    group.goreLevelInfo.push_back(
        {0.0F, 0, false, enablePainAndDeath});
    staggerDamageGroups.push_back(group);
  }
  void SetStaggerPainModelAxisForTest(const idMat3& value) {
    staggerPainModelAxis = value;
  }
  const idAIStaggerPainObservation& GetStaggerPainObservationForTest() const {
    return staggerPainObservation;
  }
  void ResetStaggerPainObservationForTest() {
    staggerPainObservation = {};
  }
  int GetStaggerPainMostDamagedIndexForTest() const;
  void SetAnimWebEventPendingForTest(aiAnimWeb_t web,
                                     animWebEvent_t eventType);
  bool IsAnimWebEventPendingForTest(aiAnimWeb_t web,
                                    animWebEvent_t eventType) const;
  idAnimator_AnimWeb* GetAnimWebAnimatorForTest(aiAnimWeb_t web);
  const idAnimator_AnimWeb* GetAnimWebAnimatorForTest(
      aiAnimWeb_t web) const;
  void SetAnimWebDeclarationForTest(const idDeclAnimWeb* value);
  void SetAnimWebDeclarationForTest(aiAnimWeb_t web,
                                    const idDeclAnimWeb* value);
  void SetAnimWebNodeAnimationForTest(short subWeb, short state,
                                      const idMD6Anim* animation) {
    if (subWeb < 0 || state < 0) return;
    const int slot = (subWeb + state) %
                     static_cast<int>(animWebState.nodeAnimations.size());
    animWebState.nodeAnimations[static_cast<std::size_t>(slot)].animation =
        animation;
  }
  void ConfigureEmergencyCoverExitForTest(bool usesEmergencyExit,
                                          bool alreadyUsed,
                                          coverAction_t action,
                                          bool hostileStance);
  bool HasUsedEmergencyCoverExitForTest() const;
  coverOrientation_t GetCoverOrientationForTest() const {
    return coverOrientation;
  }
  void ConfigureExtendedClipModelForTest(bool active, int status,
                                         int auxiliaryStatus) {
    extendedClipModelActive = active;
    extendedClipModelStatus = status;
    extendedClipModelAuxiliaryStatus = auxiliaryStatus;
    coreState.extendedClipModel = active;
  }
  bool HasExtendedClipModelForTest() const { return extendedClipModelActive; }
  int GetExtendedClipModelStatusForTest() const {
    return extendedClipModelStatus;
  }
  int GetExtendedClipModelAuxiliaryStatusForTest() const {
    return extendedClipModelAuxiliaryStatus;
  }
  void SetDyingAIRagdollPending(bool value) { dyingAIRagdollPending = value; }
  bool IsDyingAIRagdollPending() const { return dyingAIRagdollPending; }
  void SetInfluenceTrailAllocatedForTest(bool value) {
    influenceTrailAllocated = value;
  }
  bool HasInfluenceTrailForTest() const { return influenceTrailAllocated; }
  void ConfigureOnlineLootState(int disassembleLootBoxCountValue,
                                bool lootedValue, int deathStartTimeValue) {
    disassembleLootBoxCount = disassembleLootBoxCountValue;
    looted = lootedValue;
    deathStartTime = deathStartTimeValue;
  }
  void ConfigureUsableDeathFlags(bool forceDeathState,
                                 bool suppressLootWhenDead) {
    usableDeathState = forceDeathState;
    usableDeathLootSuppressed = suppressLootWhenDead;
  }
  void SetAdditiveAlarmAnimationForTest(unsigned short aliasHandle) {
    additiveAlarmAnimation = aliasHandle;
  }
  void ClearAdditiveFireAnimationsForTest() {
    additiveFireAnimations.Clear();
    additiveCrouchFireAnimations.Clear();
  }
  void AddAdditiveFireAnimationForTest(unsigned short aliasHandle,
                                       bool crouching) {
    if (crouching) {
      additiveCrouchFireAnimations.Append(aliasHandle);
    } else {
      additiveFireAnimations.Append(aliasHandle);
    }
  }
  void SetAdditiveFireAnimationBlendTimeForTest(float seconds) {
    additiveFireAnimationBlendTime = seconds;
  }
  idAnimator_Channel* GetPainAnimatorForTest() const {
    return painAnimator.get();
  }
  idAnimator_Channel* GetShootAnimatorForTest() const {
    return shootAnimator.get();
  }
  void ConfigureGrappleQueryStateForTest(std::uint64_t queryIndex,
                                         int meleeCount, int lastCheck) {
    grappleQueryIndex = queryIndex;
    meleeCountBetweenGrappleChecks = meleeCount;
    lastGrappleCheck = lastCheck;
  }
  std::uint64_t GetGrappleQueryIndexForTest() const {
    return grappleQueryIndex;
  }
  int GetMeleeCountBetweenGrappleChecksForTest() const {
    return meleeCountBetweenGrappleChecks;
  }
  int GetLastGrappleCheckForTest() const { return lastGrappleCheck; }
  void ConfigureHurtSoundForTest(const idSoundShader* sound,
                                 bool damageChannelIsPlaying) {
    hurtSound = sound;
    damageChannelPlaying = damageChannelIsPlaying;
  }
  void ConfigureDeathSoundForTest(const idSoundShader* sound,
                                  bool alreadyPlayed,
                                  const char* painJointGroup) {
    deathSound = sound;
    deathSoundPlayed = alreadyPlayed;
    hasPainJointGroup = painJointGroup != nullptr;
    painJointGroupName = painJointGroup != nullptr ? painJointGroup : "";
  }
  int GetDamageSoundStartCountForTest() const {
    return damageSoundStartCount;
  }
  const idSoundShader* GetLastDamageSoundForTest() const {
    return lastDamageSound;
  }
  bool WasDeathSoundHandledForTest() const { return deathSoundPlayed; }
  const idEntity* GetLastLookedAtEntityForTest() const {
    return lastLookedAtEntity;
  }
  int GetLastLookedAtTimeForTest() const { return lastLookedAtTime; }
  void ConfigureDeadThinkRobotState(bool deathSetupValue,
                                    float timeToCriticalValue,
                                    int lastDamageTickValue) {
    robotDeathSetup = deathSetupValue;
    robotTimeToCritical = timeToCriticalValue;
    droneLastDamageTick = lastDamageTickValue;
  }
  void ConfigureRobotDeathForTest(bool deathSetupValue,
                                  float timeToCriticalValue,
                                  float timeCriticalToExplodeValue,
                                  float stopEffectsDelayValue,
                                  bool usesDroneValue) {
    robotDeathSetup = deathSetupValue;
    robotTimeToCritical = timeToCriticalValue;
    robotTimeCriticalToExplode = timeCriticalToExplodeValue;
    robotStopEffectsDelay = stopEffectsDelayValue;
    usesDrone = usesDroneValue;
  }
  bool IsRobotDeathSetupForTest() const { return robotDeathSetup; }
  int GetRobotCriticalTimestampForTest() const {
    return robotCriticalTimestamp;
  }
  int GetRobotExplodeTimestampForTest() const {
    return robotExplodeTimestamp;
  }
  int GetRobotDeadTimestampForTest() const { return robotDeadTimestamp; }
  idFXManager* GetRobotDeathFXManagerForTest() const {
    return robotDeathFXManager.get();
  }
  idAnimator_Drone* GetRobotDeathDroneAnimatorForTest() const {
    return robotDeathDroneAnimator.get();
  }
  void ConfigureDeathSync(bool enabled, bool blockerA, bool blockerB) {
    deathSyncEnabled = enabled;
    GetAFMutable()->ConfigureSyncBlockers(blockerA, blockerB);
  }
  void SetBehaviorDeclaration(const idDeclAIBehavior* value);
  const idDeclAIBehavior* GetBehaviorDeclarationForTest() const {
    return behaviorDeclaration;
  }
  void ConfigureMeleeIntervals(int minimumClose, int maximumClose,
                               int minimumMedium, int maximumMedium,
                               int minimumLong, int maximumLong) {
    minimumCloseMeleeInterval = minimumClose;
    maximumCloseMeleeInterval = maximumClose;
    minimumMediumMeleeInterval = minimumMedium;
    maximumMediumMeleeInterval = maximumMedium;
    minimumLongMeleeInterval = minimumLong;
    maximumLongMeleeInterval = maximumLong;
  }

private:
  friend class idAIScriptProxy;
  friend class idCheckSurroundingsState;

  void OnAIEvent_Voice(const idAIEventVoice* event);
  void OnAIEvent_Footstep(const idAIEventSound* event);
  void OnAIEvent_Gunfire(const idAIEventSound* event);
  void OnAIEvent_Explosion(const idAIEventSound* event);
  void OnAIEvent_ProjectileImpact(const idAIEventSound* event);
  void OnAIEvent_DeadBodyLand(const idAIEventSound* event);
  void OnAIEvent_DoorMotion(const idAIEventSound* event);
  void OnAIEvent_DeploySound(const idAIEventSound* event);
  void OnAIEvent_Grenade(const idAIEventProjectedSphere* event);
  void OnAIEvent_Vehicle(const idAIEvent_Vehicle* event);
  bool InitAnimation();
  void UpdateTravelFlags();
  void UpdateEffects();
  eventVoid* RecordAnimEvent(const char* eventName, eventVoid* result);
  eventVoid* RecordGameEvent(const char* eventName, eventVoid* result);
  idRecoveredValue RecordCoreCall(const char* methodName);
  void UpdateAimFocus();
  void SetupRobotDead();
  bool FindClosestFocus();
  const idEntity* ResolveDeathAttackerForName(const idEntity* attacker);

  idAIScriptProxy scriptProxy;
  bool voiceOverPlaying{false};
  std::unique_ptr<idAIVoiceController> localVoiceController;
  std::unique_ptr<idAICamera_Control> playerControlCamera;
  std::unique_ptr<idAIWorldState> worldState;
  idDeclAIBehaviorEvents behaviorEventDeclarations;
  bool behaviorEventDeclarationsConfigured{false};
  std::unique_ptr<idAASPositionMgr> aasPositionManager;
  std::unique_ptr<idAIVolatile::idAIAnimationInfo> animationInfo;
  std::unique_ptr<idAIConstant> aiConstants;
  std::unique_ptr<idAIAnimationInitRuntimeState> animationInitState;
  std::unique_ptr<idPresentable> ownedEffectsPresentable;
  std::unique_ptr<idFXManager> effectsFXManager;
  std::unique_ptr<idAIHealth> healthComponent;
  std::unique_ptr<idAIDamageInfo> damageInfo;
  std::unique_ptr<idAIDeathInfo> deathInfo;
  std::unique_ptr<idSpawnFocusWithRequirement> deathFocusOverride;
  std::unique_ptr<idComponentActionScript> componentTimeLine;
  std::unique_ptr<idFormationCoverMgr> dynamicCoverManager;
  std::unique_ptr<idGoreComponent> goreComponent;
  std::array<std::unique_ptr<idAnimator_AnimWeb>, 2> animWebAnimators;
  std::unique_ptr<idFaceMgr> faceManager;
  std::unique_ptr<idAIOrientation> bodyOrientation;
  std::unique_ptr<idAnimator_Drone> animationControllerDroneAnimator;
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
  float focusCompensationTime{-1.0F};
  bool predictTargetMovement{false};
  float testFireAimDot{0.8F};
  float testFireAimDotUpsideDown{0.8F};
  float testFireGangstaAimDot{0.8F};
  bool missTrackAlongGround{false};
  std::uint32_t testFireMuzzleContents{0};
  idVec3 testFireLinearVelocity{};
  idVec3 aimDebugEyeOffset{};
  mutable std::string lastAimDebugText;
  mutable int lastAimDebugLevel{0};
  mutable idVec3 lastAimDebugPosition{};
  mutable int aimDebugCount{0};
  mutable int missTimeDebugPointCount{0};
  mutable int missTimeDebugLineCount{0};
  mutable idVec3 lastMissTimeDebugPoint{};
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
  int delayedVoiceType{30};
  int delayedVoiceTime{-1};
  const idAAS2* aas{nullptr};
  int behaviorWeaponSlot{0};
  bool behaviorHasSyncedWalks{false};
  float bodyFadeTime{0.0F};
  int fadeOutDuration{0};
  int fadeOutTimestamp{-1};
  int stackedTakedownWindows{0};
  int takedownWindowClosed{-1};
  int counterBlockedUntil{-1};
  bool stunnedByPlayerBlock{false};
  encounterGroupRole_t encounterRole{ROLE_NONE};
  bool encounterActive{true};
  idEncounterGroup* aiEventEncounterGroup{nullptr};
  int tauntUseCount{0};
  int grappleActivationCheckTimer{-1};
  bool aimFocusActive{false};
  bool turretAnimationSupported{false};
  bool turretActionSetting{false};
  aiModeRestricFlag_t movementModeRestrictionFlags{0};
  bool forceRelaxedSubWeb{false};
  bool forceCombatSubWeb{false};
  int defaultMovementMode{0};
  float initialMaxMoveSpeed{0.0F};
  float initialAccelRate{0.0F};
  float initialDecelRate{0.0F};
  bool usesConfiguredFireTiming{false};
  float configuredMinBurstDuration{0.0F};
  float configuredMaxBurstDuration{0.0F};
  float configuredMinBurstPause{0.0F};
  float configuredMaxBurstPause{0.0F};
  idFXManager* attachmentFXManager{nullptr};
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
  int lkwBlindEffectCount{0};
  int aiEvaluationTime{0};
  float incomingDamageScale{1.0F};
  bool painAnimationDone{true};
  bool usesSleeping{true};
  float sleepRadius{100.0F};
  std::unique_ptr<idAIFireControl> fireControl;
  int groupStatusCondition{0};
  bool useTraversalClassA{false};
  bool useTraversalClassB{false};
  bool useTraversalClassC{false};
  bool useTraversalClassD{false};
  bool useTraversalClassE{false};
  std::uint32_t aasTravelFlags{0};
  std::string lastTravelDebugText;
  idVec3 lastTravelDebugPosition{};
  int travelDebugCount{0};
  bool deadEffectActive{false};
  int deadEffectDuration{0};
  bool noBodyRemoval{false};
  bool bodyRotationEnabled{true};
  bool suppressBodyRotation{false};
  bool movementBehaviorRunsFacingForward{false};
  int flareHandle{-1};
  aiEffectUpdateType_t flareUpdateType{AIEFFECTUPDATETYPE_NONE};
  idTreeAnimator::tagData_t flareTagData{};
  idTreeAnimator defaultEffectsTreeAnimator;
  idTreeAnimator* effectsTreeAnimator{nullptr};
  idMat3 animationModelAxis{};
  bool corpseAnimationControllerBlocked{false};
  const idLookupTable* flareFadeTable{nullptr};
  float flareFadeOutScale{1.0F};
  float flareFadeTimer{1000.0F};
  bool flareFadeOutEffect{false};
  bool flareHasBeenKilled{false};
  float effectsRobotGlow{0.0F};
  idVec4 effectsFlareColor{};
  int effectsUpdateFXCount{0};
  bool groupCurrentCoverValid{false};
  idVec3 groupCurrentCoverOrigin{};
  int groupLastFiredAtTime{-1};
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
  bool isKampHund{false};
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
  int autoFocusEntityMode{2};
  int autoFocusSurroundingsMode{2};
  int autoFocusMinimumTime{800};
  int autoFocusMaximumTime{1200};
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
  std::unique_ptr<idCheckSurroundingsState> checkSurroundingsState;
  std::unique_ptr<idAIMoveState> moveState;
  idScenePoint* currentScenePoint{nullptr};
  int currentScenePointActivationTime{0};
  const idDeclAIBehavior* behaviorDeclaration{nullptr};
  int defaultPhysicsClipMask{0};
  aiMovePushStatus_t movePushStatus{AIMOVEPUSHSTATUS_PUSH_NONE};
  int movePushEndTime{0};
  int nextAIEventTime{0};
  bool animWebEventPending{false};
  std::unique_ptr<idLegsFSM> actionFSM;
  bool usesGrenades{false};
  bool detectGrenadesInAir{false};
  bool ignoreFriendlyVehicles{false};
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
  idAIPos currentEnemySuspectedPosition{};
  bool canBeBlockedByFriends{true};
  bool hasFriendlyBlockerBindAnimation{false};
  idEntity* fakeEnemy{nullptr};
  int fakeEnemyNumber{-1};
  const idDeclInventory* inventoryDecl{nullptr};
  idEntity* deathTrigger{nullptr};
  idEntity* triggerOnDying{nullptr};
  idEntity* teslaBombFXEntity{nullptr};
  idEntity* focusOverrideEntity{nullptr};
  const idDeclEntityDef* deathSpawnEntityDef{nullptr};
  const void* deathBehaviorBodyEventDecl{nullptr};
  const void* enemyKilledVOEventDecl{nullptr};
  std::uint8_t killedNotificationFlags{0};
  bool goredFullBody{false};
  bool deathUseDeadPhysics{false};
  bool deathUsesTesla{false};
  bool deathVolatileGrenadeFlag{false};
  bool firstAlarm{false};
  bool isCommander{false};
  int deathRemoveTime{0};
  int deathPainGroupHandle{0};
  bool walkIKActive{true};
  bool walkIKDeadKeepLastMods{false};
  int checkpointKilled{-1};
  idMat3 deathModelAxis{};
  idAIDeathSystemObservation deathSystemObservation;
  takedownTypes_t takedownTypes;
  idAITakedownObservation takedownObservation;
  bool cutNextMeleeDamage{false};
  idEntity* droppedItem1{nullptr};
  idEntity* droppedItem2{nullptr};
  idRagdollInfo ragdollInfo;
  idEntity* currentActionNode{nullptr};
  idStr actionNodeGroupName;
  idEntity* stealthKillTrigger{nullptr};
  int dropState{0};
  bool ignoreDamageWhileDropping{false};
  float nonPlayerOneShotChance{0.0F};
  bool ragdollEnabled{true};
  bool ragdollSuppressed{false};
  idVec3 dronePastVelocity{};
  idStr reasonForRagdoll;
  int alertCycleTimestamp{-1};
  bool usesSmoothAnimRelaxed{false};
  bool usesSmoothAnimSearch{false};
  bool usesSmoothAnimCombat{false};
  runIndexType_t currentRunIndexType{ANIMWEBAI_RUNINDEXTYPE_NORMAL};
  int injuredTime{-1};
  const idDeclFacialAnimationSet* currentFacialAnimationSet{nullptr};
  const idDeclFacialAnimationSet* defaultFacialAnimationSet{nullptr};
  int syncedOverrideAnimation{0};
  int syncedTargetSpawnId{0x1FFF};
  int actionScriptCount{0};
  bool painAllowedAfterActionScript{true};
  bool canBecomeInjured{false};
  bool hasLongPainAnimations{false};
  bool staggeringPainActive{false};
  float longPainDamageThreshold{0.0F};
  float longPainMinInterval{0.0F};
  int painEndTime{0};
  int postureChangeTime{0};
  idMat3 staggerPainModelAxis{};
  int staggerPainGroupHandle{0};
  idStr staggerPainGroupName;
  int staggerDamageGroupHandle{0};
  idStr staggerDamageGroupName;
  idStr staggerJointName{"<unknown>"};
  std::vector<idDamageGroup> staggerDamageGroups;
  idAIStaggerPainObservation staggerPainObservation;
  bool isPlayingEntranceAnim{false};
  bool extendedClipModelActive{false};
  int extendedClipModelStatus{0};
  int extendedClipModelAuxiliaryStatus{0};
  bool dyingAIRagdollPending{false};
  bool influenceTrailAllocated{false};
  int disassembleLootBoxCount{0};
  bool looted{false};
  int deathStartTime{0};
  const idSoundShader* hurtSound{nullptr};
  const idSoundShader* deathSound{nullptr};
  const idSoundShader* lastDamageSound{nullptr};
  bool damageChannelPlaying{false};
  bool deathSoundPlayed{false};
  bool hasPainJointGroup{false};
  idStr painJointGroupName;
  int damageSoundStartCount{0};
  const idEntity* lastLookedAtEntity{nullptr};
  int lastLookedAtTime{-1};
  bool robotDeathSetup{false};
  float robotTimeToCritical{0.0F};
  float robotTimeCriticalToExplode{0.0F};
  float robotStopEffectsDelay{0.0F};
  int robotCriticalTimestamp{0};
  int robotExplodeTimestamp{0};
  int robotDeadTimestamp{0};
  std::unique_ptr<idFXManager> robotDeathFXManager;
  std::unique_ptr<idAnimator_Drone> robotDeathDroneAnimator;
  int droneLastDamageTick{0};
  bool deathSyncEnabled{false};
  std::array<idTrajectory_Parabolic, 4> trajectories;
  int animatedEntityJobSyncCount{0};
  unsigned int meleeAnimationFlags{0};
  unsigned int crouchMeleeAnimationFlags{0};
  std::unique_ptr<idFSMManager> fsmManager;
  std::uint32_t defaultPhysicsContents{0};
  bool solidState{false};
  bool takedownRestrictedToBehind{false};
  std::unique_ptr<idAIActionManager> actionManager;
  std::unique_ptr<idAIFSMCallback> fsmCallback;
  float actorIdlePerceptionRadius{-1.0F};
  float actorSearchPerceptionRadius{-1.0F};
  float actorPerceptionRadius{0.0F};
  float configuredFieldOfView{90.0F};
  bool hasScriptObject{false};
  idThread* primaryScriptThread{nullptr};
  int lastSignal{-1};
  int signalCount{0};
  float safeReloadFraction{0.0F};
  int minimumCloseMeleeInterval{-1};
  int maximumCloseMeleeInterval{-1};
  int minimumMediumMeleeInterval{-1};
  int maximumMediumMeleeInterval{-1};
  int minimumLongMeleeInterval{-1};
  int maximumLongMeleeInterval{-1};
  bool usableDeathState{false};
  bool usableDeathLootSuppressed{false};
  unsigned short additiveAlarmAnimation{0xFFFFU};
  std::unique_ptr<idAnimator_Channel> painAnimator;
  std::uint64_t grappleQueryIndex{0};
  int meleeCountBetweenGrappleChecks{0};
  int lastGrappleCheck{0};
  bool immediatePhysicsDormancyGate{false};
  bool animWebTransitionActive{false};
  bool articulatedFigureAtRest{true};
  int previousAlertCycle{ALERTCYCLE_MAX};
  int highestAlertCycle{ALERTCYCLE_NONE};
  int lastSurprisedTime{-1};
  bool alwaysInCombat{false};
  std::unique_ptr<idAIMemory> memory;
  idStr encounterGroupName;
  std::int16_t encounterGroupHandle{-1};
  bool perceptionDeaf{false};
  bool perceptionWhenScripted{false};
  float perceptionHearingStimulus{1.0F};
  float perceptionCombatTimeoutInterval{-1.0F};
  bool shootingSenseUpdateCheat{false};
  float firedAtCosAngleOff{0.0F};
  idEntity* onAlarmTrigger{nullptr};
  idEntity* onDetectEnemyTrigger{nullptr};
  idEntity* onSpotEnemyTrigger{nullptr};
  bool onlyPlayerTriggersOnSpot{false};
  int enemyReactionTime{-1};
  std::uint32_t pendingReactions{0};
  std::uint32_t forbiddenReactions{0};
  idList<unsigned short, 5> additiveFireAnimations;
  idList<unsigned short, 5> additiveCrouchFireAnimations;
  float additiveFireAnimationBlendTime{0.0F};
  std::unique_ptr<idAnimator_Channel> shootAnimator;
  bool usesEmergencyCoverExit{false};
  bool allowHostileStance{false};
  coverOrientation_t coverOrientation{COVERORIENT_NONE};
  bool greetingsEnabled{false};
  mutable std::string lastLookDebugText;
  mutable int lastLookDebugLevel{0};
  mutable int lastLookDebugDuration{0};
  mutable idVec3 lastLookDebugPosition{};
  mutable int lookDebugCount{0};
  bool spawnDetectPlayerCombat{false};
  bool spawnDetectPlayerSearch{false};
  const idSoundShader* spawnAmbientSound{nullptr};
  idList<idStr, 5> spawnMissingArmor;
  idStr spawnEntranceAnimationPath;
  bool spawnPlayerIsSquadLeader{false};
  int spawnPlayerGeneration{0};
  int squadLeaderSpawnId{0x1FFF};
  int spawnAwarenessCycle{ALERTCYCLE_NONE};
  const idAnimStack* spawnAnimationStack{nullptr};
  int spawnAmbientSoundStartCount{0};
  int spawnAmbientSoundChannel{0};
  const idSoundShader* lastSpawnAmbientSound{nullptr};
  unsigned int spawnAmbientSoundFlags{0};
  unsigned int spawnAmbientSoundPeerMask{0};
  idAISpawnCorpseObservation spawnCorpseObservation;
  idAITeslaUpdateState teslaUpdateState;
  idAITeslaUpdateObservation teslaUpdateObservation;
  const void* armorPopOffHeadEventDecl{nullptr};
  idList<idEntityPtr<idEntity>, 5> spawnedDetachables;
  idList<idStr, 5> enabledArmorHitGroups;
  idList<idStr, 5> disabledArmorHitGroups;
  idAIArmorPopOffObservation armorPopOffObservation;
  std::vector<std::unique_ptr<idPonytail>> gorePonytails;
  int goreWalkIKDisableCount{0};
  int goreAttachmentSnapshotCount{0};
  int goreWeaponFlagClearCount{0};
  int gorePonytailShutdownCount{0};
  bool updateDeathsDeactivated{false};
  bool updateDeathsAdditivePainEnabled{false};
  idAIUpdateDeathsPainObservation updateDeathsPainObservation;
  int playerControlPossessionEvent{-1};
  idAngles playerControlViewAngles{};
  idEntity* crosshairAttachedLoot{nullptr};
  trackingParms_t worldLookTrackingParms{};
  trackingParms_t worldAimTrackingParms{};
  bool worldUsesLookScenePoints{true};
  float combatTargetDelayLook{-1.0F};
  float idleTalkPeriod{-1.0F};
  int worldStateAimDivergeCount{0};
  int worldStateAffirmativeResponseCount{0};
  int worldStateLastAffirmativeVoice{-1};
};

static_assert(sizeof(idAI2::idAIVolatile::idAIAnimationInfo) == 0xEB8,
              "idAIAnimationInfo ABI size drift");
static_assert(offsetof(idAI2::idAIVolatile::idAIAnimationInfo,
                       turretAnimator) == 0x480);
static_assert(offsetof(idAI2::idAIVolatile::idAIAnimationInfo,
                       painAnimator) == 0x8E0);
static_assert(offsetof(idAI2::idAIVolatile::idAIAnimationInfo,
                       viewTagData) == 0xE80);
static_assert(offsetof(idAI2::idAIVolatile::idAIAnimationInfo,
                       syncedAnim) == 0xEA8);
static_assert(offsetof(idAI2::idAIVolatile::idAIAnimationInfo,
                       ikEnableTime) == 0xEB0);
static_assert(sizeof(idAI2::idAIVolatile::idAIScriptInfo) == 0x80,
              "idAIScriptInfo ABI size drift");
static_assert(offsetof(idAI2::idAIVolatile::idAIScriptInfo, threads) == 0x10);
static_assert(offsetof(idAI2::idAIVolatile::idAIScriptInfo, fibers) == 0x38);
static_assert(offsetof(idAI2::idAIVolatile::idAIScriptInfo,
                       currentAIEvent) == 0x60);
static_assert(offsetof(idAI2::idAIVolatile::idAIScriptInfo,
                       lastAIEventOriginator) == 0x78);
static_assert(sizeof(idAI2::idAnimWebDefaults) == 0x48,
              "idAnimWebDefaults ABI size drift");
static_assert(sizeof(idAI2::idAIConstant) == 0x138,
              "idAIConstant ABI size drift");
static_assert(offsetof(idAI2::idAIConstant, movement) == 0x20);
static_assert(offsetof(idAI2::idAIConstant, animation) == 0xA0);
static_assert(offsetof(idAI2::idAIConstant, onlineScoring) == 0x130);
static_assert(offsetof(idAI2::idAIConstant, stats) == 0x134);

extern idCVar ai_forceWalk;
extern idCVar ai_enableDormancy;
extern idCVar ai_debugTraversals;
extern idCVar ai_faceFocus;
extern idCVar ai_forceTurnRate;
extern idCVar ai_minTurnRate;
extern idCVar ai_maxTurnRate;
extern idCVar takedownWindoMaxTime;
extern idCVar counterBlockTime;
extern idCVar ai_enableInfluenceTrails;
extern idCVar ai_lootDelay;
