#pragma once

#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/containers/list.h"
#include "engine/models/renderworld.h"

#include <cstdint>
#include <string>

class idAAS2;
class idAASVisCache;
class idAIGameState;
class idEncounterGroupManager;
class idDeclEntityDef;
class idDeclDamage;
class idFaction;
struct idEventDef;
class idEntity;
class idPlayer;
class idWaypoint;
enum aimPoint_t : int;

enum aiHoldFireReason_t : int {
  AI_HOLDFIRE_REASON_NONE = 0,
  AI_HOLDFIRE_REASON_FRIENDLY = 1,
  AI_HOLDFIRE_REASON_RANGE = 2,
  AI_HOLDFIRE_REASON_NOREGAIN = 3,
  AI_HOLDFIRE_REASON_SPOTLOST = 4,
  AI_HOLDFIRE_REASON_NOTRACK = 5,
};

struct idPostedEvent {
  const idEventDef* eventDefinition{nullptr};
  int delayMilliseconds{0};
  const idEntity* entityArgument{nullptr};
};

struct idStoppedSound {
  int channel{0};
  unsigned int peerMask{0};
};

struct inputSettings_t {
  int bindset{0};
  float mouseSensScale{1.0F};
  float joySensScale{1.0F};
  float joyPitchSpeed{0.0F};
  float joyYawSpeed{0.0F};
  float mouseSens{0.0F};
  bool absoluteAngles{false};
  bool lookInvert{false};
  idAngles aimAssistAngles{};
};
static_assert(sizeof(inputSettings_t) == 40,
              "inputSettings_t ABI drift");

// Minimal shared game-side contracts used while the recovered Tungsten game
// translation units are brought back online.  These interfaces model the
// retail entity/physics interactions without coupling the game port to the
// Doom 3 BFG reference tree.

class idPhysics {
public:
  virtual ~idPhysics() = default;

  virtual const idBounds& GetAbsBounds() const = 0;
  virtual const idVec3& GetOrigin() const = 0;
  virtual const idMat3& GetAxis() const {
    static const idMat3 identity = [] {
      idMat3 value{};
      value[0] = {1.0F, 0.0F, 0.0F};
      value[1] = {0.0F, 1.0F, 0.0F};
      value[2] = {0.0F, 0.0F, 1.0F};
      return value;
    }();
    return identity;
  }
  virtual const idVec3& GetGravityNormal() const {
    static const idVec3 down{0.0F, 0.0F, -1.0F};
    return down;
  }
  virtual void SetAxis(const idMat3& axis) { (void)axis; }
  virtual void SetOrigin(const idVec3& origin, int bodyId = 0) {
    (void)origin;
    (void)bodyId;
  }
  virtual std::uint32_t GetContents() const = 0;
  virtual void SetContents(std::uint32_t contents) = 0;
  virtual void SetContents(std::uint32_t contents, int bodyId) {
    if (bodyId == 0) SetContents(contents);
  }
  virtual void EnableClip() = 0;
  virtual void DisableClip() = 0;
  virtual void LinkClip() { EnableClip(); }
  virtual void UnlinkClip() { DisableClip(); }
  virtual int GetNumClipModels() const { return 1; }
  virtual int GetClipMask(int bodyId = -1) const {
    (void)bodyId;
    return 0;
  }
  virtual void SetClipMask(int mask, int bodyId = 0) {
    (void)mask;
    (void)bodyId;
  }
  virtual void ApplyImpulse(int bodyId, const idVec3& point,
                            const idVec3& impulse) {
    (void)bodyId;
    (void)point;
    (void)impulse;
  }
  virtual void ApplyForce(int bodyId, const idVec3& point,
                          const idVec3& force) {
    (void)bodyId;
    (void)point;
    (void)force;
  }
  virtual idVec3 GetLinearVelocity(int bodyId = 0) const {
    (void)bodyId;
    return {};
  }
  virtual void SetLinearVelocity(const idVec3& velocity, int bodyId = 0) {
    (void)velocity;
    (void)bodyId;
  }
  virtual void SetAngularVelocity(const idVec3& velocity, int bodyId = 0) {
    (void)velocity;
    (void)bodyId;
  }
  virtual bool IsAIInjuryEligible() const { return false; }
  virtual bool ShouldEnterDormancyImmediately() const { return false; }
};

class idEntity {
public:
  virtual ~idEntity() = default;

  virtual void PostSpawn() {}
  virtual void Remove() { removed = true; }
  virtual void OnHide(bool stopFx) {
    (void)stopFx;
    hidden = true;
  }
  virtual void OnShow() { hidden = false; }
  virtual void OnActivate(idEntity* activator) { (void)activator; }
  virtual void KilledNotification(idEntity* victim, idEntity* inflictor,
                                  const idDeclDamage* damageDecl,
                                  float damage = 0.0F,
                                  bool killedByDualWield = false,
                                  bool coverKill = false) {
    (void)victim;
    (void)inflictor;
    (void)damageDecl;
    (void)damage;
    (void)killedByDualWield;
    (void)coverKill;
  }
  virtual void PostEventMS(const idEventDef* eventDefinition,
                           int delayMilliseconds) {
    postedEvents.Append({eventDefinition, delayMilliseconds});
  }
  virtual void PostEventMS(const idEventDef* eventDefinition,
                           int delayMilliseconds,
                           const idEntity* entityArgument) {
    postedEvents.Append(
        {eventDefinition, delayMilliseconds, entityArgument});
  }
  virtual const void* GetAIGroupIdentity() const { return nullptr; }
  virtual bool ProvidesDynamicCover() const { return false; }
  virtual bool IsWeaponStaticProp() const { return false; }
  virtual idFaction* GetFaction() const { return nullptr; }
  virtual bool IsDead() const { return false; }
  virtual void GetAimPoint(aimPoint_t point, idVec3* result) const {
    (void)point;
    *result = physics != nullptr ? physics->GetOrigin() : idVec3{};
  }
  virtual idVec3 GetLinearVelocity() const { return {}; }
  virtual bool ShouldLeaveDormancy() const { return leaveDormancy; }
  virtual void GetSoundTransform(idVec3* origin, idMat3* axis) const {
    if (physics != nullptr) {
      *origin = physics->GetOrigin();
      *axis = physics->GetAxis();
    } else {
      *origin = {};
      *axis = {};
    }
  }
  virtual bool ShouldEnterDormancy() const { return hidden; }
  virtual void Unbind() { ++unbindCount; }
  virtual void StopSound(int channel, unsigned int peerMask) {
    stoppedSounds.Append({channel, peerMask});
  }
  virtual void SetOrigin(const idVec3& origin) {
    if (physics != nullptr) physics->SetOrigin(origin, 0);
  }
  virtual void SetAxis(const idMat3& axis) {
    if (physics != nullptr) physics->SetAxis(axis);
  }
  virtual void BindToJoint(idEntity* master, short jointIndex,
                           bool orientated) {
    bindMaster = master;
    bindJoint = jointIndex;
    bindOrientated = orientated;
    ++bindCount;
  }
  virtual bool IsMoveableProp() const { return false; }
  virtual void SetWhoDroppedMe(idEntity* entity) { whoDroppedMe = entity; }
  virtual void DropToFloor(bool activate) {
    lastDropToFloorActivate = activate;
    ++dropToFloorCount;
  }
  virtual void SetDeathSpawnedFlag() { entityFlags |= 4U; }

  idMat3* GetPresentableAxis();
  const idMat3* GetPresentableAxis() const { return &presentableAxis; }
  void SetPresentableAxis(const idMat3& value) { presentableAxis = value; }
  const char* GetEntityDefName();
  const char* PeekEntityDefName() const { return entityDefName.c_str(); }
  const idVec3* GetPresentableLeftDir();
  const idVec3* GetPresentableUpDir();
  idEntity* GetTarget(int index);
  void AddTarget(idEntity* target) { targets.Append(target); }
  void ClearTargets() { targets.Clear(); }

  idPhysics* GetPhysics() const { return physics; }
  void SetPhysics(idPhysics* value) { physics = value; }

  const char* GetName() const { return name.c_str(); }
  void SetName(const char* value) { name = value != nullptr ? value : ""; }
  void SetEntityDefName(const char* value) {
    entityDefName = value != nullptr ? value : "";
  }
  int GetEntityNumber() const { return entityNumber; }
  void SetEntityNumber(int value) { entityNumber = value; }

  bool IsHidden() const { return hidden; }
  void SetHiddenForTest(bool value) { hidden = value; }
  bool IsRemoved() const { return removed; }
  void SetShouldLeaveDormancy(bool value) { leaveDormancy = value; }
  int GetPostedEventCount() const { return postedEvents.Num(); }
  const idPostedEvent& GetPostedEvent(int index) const {
    return postedEvents[index];
  }
  void ClearPostedEvents() { postedEvents.Clear(); }
  int GetUnbindCountForTest() const { return unbindCount; }
  int GetStoppedSoundCountForTest() const { return stoppedSounds.Num(); }
  int GetStoppedSoundChannelForTest(int index) const {
    return stoppedSounds[index].channel;
  }
  unsigned int GetStoppedSoundPeerMaskForTest(int index) const {
    return stoppedSounds[index].peerMask;
  }
  int GetBindCountForTest() const { return bindCount; }
  idEntity* GetBindMasterForTest() const { return bindMaster; }
  short GetBindJointForTest() const { return bindJoint; }
  bool GetBindOrientatedForTest() const { return bindOrientated; }
  idEntity* GetWhoDroppedMeForTest() const { return whoDroppedMe; }
  int GetDropToFloorCountForTest() const { return dropToFloorCount; }
  bool GetLastDropToFloorActivateForTest() const {
    return lastDropToFloorActivate;
  }
  unsigned int GetEntityFlagsForTest() const { return entityFlags; }

protected:
  idPhysics* physics{nullptr};
  std::string name;
  std::string entityDefName;
  bool hidden{false};
  bool removed{false};
  idMat3 presentableAxis{};
  int entityNumber{-1};
  bool leaveDormancy{false};
  idList<idEntity*, 5> targets;
  idList<idPostedEvent, 5> postedEvents;
  int unbindCount{0};
  idList<idStoppedSound, 5> stoppedSounds;
  idEntity* bindMaster{nullptr};
  short bindJoint{-1};
  bool bindOrientated{false};
  int bindCount{0};
  idEntity* whoDroppedMe{nullptr};
  int dropToFloorCount{0};
  bool lastDropToFloorActivate{false};
  unsigned int entityFlags{0};
};

class idGameLocal {
public:
  virtual ~idGameLocal() = default;

  virtual void ChangeAreaTravelFlags(const idBounds& bounds,
                                     std::uint32_t areaContents,
                                     std::uint32_t travelFlags,
                                     bool enabled,
                                     const char* debugName) = 0;
  virtual int GetFrameNum() const { return 0; }
  virtual int GetAlarmTimestamp() const { return -1; }
  virtual std::uint32_t GetRandomSeed() const { return 0; }
  virtual void SetRandomSeed(std::uint32_t seed) { (void)seed; }
  virtual int GetGameHz(int gameTimeType) const {
    (void)gameTimeType;
    const int millisecondsPerFrame = GetGameMsPerFrame();
    return millisecondsPerFrame > 0 ? 1000 / millisecondsPerFrame : 0;
  }
  virtual int GetGameMsPerFrame() const { return 16; }
  virtual int GetGameMsPerRealSec() const { return 1000; }
  virtual int GetGameTimeMs() const { return GetFrameNum() * GetGameMsPerFrame(); }
  virtual const idVec3& GetGravity() const {
    static const idVec3 gravity{0.0F, 0.0F, -1066.0F};
    return gravity;
  }
  virtual bool TestGrappler(int spawnIndex, int timestamp,
                            float rangeToEnemy) {
    (void)spawnIndex;
    (void)timestamp;
    (void)rangeToEnemy;
    return true;
  }
  virtual int GetNextAITauntTime() const { return 0; }
  virtual void SetNextAITauntTime(int value) { (void)value; }
  virtual idEntity* GetEntityByNumber(int entityNumber) const {
    (void)entityNumber;
    return nullptr;
  }
  virtual idRenderWorld* GetRenderWorld() const { return nullptr; }
  virtual idAASVisCache* GetAASVisCache() { return nullptr; }
  virtual idAIGameState* GetAIGameState() { return nullptr; }
  virtual int GetChallengeMode() const;
  virtual idEntity* GetDebugPlayer() const { return nullptr; }
  virtual idPlayer* GetPlayer(int index) const {
    (void)index;
    return nullptr;
  }
  virtual int NumAAS() const { return 0; }
  virtual idAAS2* GetAAS(int index) const {
    (void)index;
    return nullptr;
  }
  virtual float GetAIIncomingDamageScale() const { return 1.0F; }
  virtual std::uint64_t SubmitTranslationQuery(
      const idVec3& start, const idVec3& end, int clipSize,
      const idMat3& axis, int contentsMask, int passEntityNumber,
      bool asynchronous, const char* debugName) {
    (void)start;
    (void)end;
    (void)clipSize;
    (void)axis;
    (void)contentsMask;
    (void)passEntityNumber;
    (void)asynchronous;
    (void)debugName;
    return 0;
  }
  virtual idEncounterGroupManager* GetEncounterGroupManager() {
    return nullptr;
  }
  virtual int GetCurrentCheckpoint() const { return -1; }
  virtual void RemoveAIWorldStateUser(idEntity* entity) { (void)entity; }
  virtual void RemoveAIEventReceiver(idEntity* entity) { (void)entity; }
  virtual void RemoveAIEntityEvents(const idEntity* entity) { (void)entity; }
  virtual void AddAIBehaviorBodyEvent(const void* declaration,
                                      int currentTime,
                                      const idEntity* originator,
                                      const idEntity* instigator,
                                      const idVec3& origin, int delay) {
    (void)declaration;
    (void)currentTime;
    (void)originator;
    (void)instigator;
    (void)origin;
    (void)delay;
  }
  virtual void AddAIEvent(const void* declaration, int currentTime,
                          const idEntity* originator,
                          const idEntity* instigator,
                          const idVec3& origin, int delay) {
    (void)declaration;
    (void)currentTime;
    (void)originator;
    (void)instigator;
    (void)origin;
    (void)delay;
  }
  virtual idEntity* SpawnEntityFromDef(const idDeclEntityDef* declaration) {
    (void)declaration;
    return nullptr;
  }
  virtual void AddUpdateSlot(int spawnIndex, int timestamp, int slotType,
                             float range, bool alarming, bool dying,
                             bool spotted, bool killedByTakedown) {
    (void)spawnIndex;
    (void)timestamp;
    (void)slotType;
    (void)range;
    (void)alarming;
    (void)dying;
    (void)spotted;
    (void)killedByTakedown;
  }
  virtual void UpdateGlobalLensFlarePosition(int handle,
                                             const idVec3& origin,
                                             const idMat3& axis,
                                             bool bindOrigin,
                                             bool bindAxis) {
    (void)handle;
    (void)origin;
    (void)axis;
    (void)bindOrigin;
    (void)bindAxis;
  }
  virtual void SetGlobalLensFlareColor(int handle,
                                       const idVec4& color) {
    (void)handle;
    (void)color;
  }
  void AddDyingAI(idEntity* entity) {
    if (entity == nullptr) return;
    for (int i = 0; i < dyingAI.Num(); ++i)
      if (dyingAI[i] == entity) return;
    dyingAI.Append(entity);
  }
  void RemoveDyingAI(idEntity* entity) {
    for (int i = 0; i < dyingAI.Num(); ++i) {
      if (dyingAI[i] == entity) {
        dyingAI.RemoveIndex(i);
        return;
      }
    }
  }
  bool HasDyingAI(const idEntity* entity) const {
    for (int i = 0; i < dyingAI.Num(); ++i)
      if (dyingAI[i] == entity) return true;
    return false;
  }
  std::intptr_t GetDebugEntity(...);
  std::intptr_t GetTensionManager(...);

  idList<idWaypoint*, 107> waypointEntities;

private:
  idList<idEntity*, 5> dyingAI;
};

inline idGameLocal* gameLocal = nullptr;

constexpr std::uint32_t AAS_AREA_CONTENTS_OBSTACLE = 0x800u;
constexpr std::uint32_t CONTENTS_DYNAMIC_OBSTACLE = 0x20000u;
constexpr std::uint32_t AAS_TFL_INVALID = 1u;

class idCVar {
public:
  idCVar(const char* cvarName, const char* defaultValue, int cvarFlags,
         const char* cvarDescription)
      : name(cvarName != nullptr ? cvarName : ""),
        valueString(defaultValue != nullptr ? defaultValue : ""),
        flags(cvarFlags),
        description(cvarDescription != nullptr ? cvarDescription : "") {
    try {
      valueFloat = std::stof(valueString);
      valueInteger = static_cast<int>(valueFloat);
    } catch (...) {
      valueInteger = 0;
      valueFloat = 0.0F;
    }
  }

  int GetInteger() const { return valueInteger; }
  float GetFloat() const { return valueFloat; }
  void SetInteger(int value) {
    valueInteger = value;
    valueFloat = static_cast<float>(value);
    valueString = std::to_string(value);
  }
  void SetFloat(float value) {
    valueFloat = value;
    valueInteger = static_cast<int>(value);
    valueString = std::to_string(value);
  }

  std::string name;
  std::string valueString;
  int valueInteger{0};
  float valueFloat{0.0F};
  int flags{0};
  std::string description;
};
