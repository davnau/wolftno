#pragma once

#include "tungsten/game/ai/ai2_actions.h"
#include "tungsten/game/ai/aibehaviors.h"
#include "tungsten/game/ai/aidamageinfo.h"
#include "tungsten/game/ai/aimemory.h"
#include "tungsten/game/ai/aimovestate.h"
#include "tungsten/game/ai/rotationtracker.h"
#include "tungsten/game/clientgame/presentable/presentable.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/filesystem/file.h"

#include <algorithm>
#include <cstring>

enum invalidAliasHandle_t { INVALID_ALIAS_HANDLE = -1 };

template <typename T, typename InvalidEnum, T InvalidValue>
struct idHandle {
  T value{InvalidValue};
  bool operator==(const idHandle& other) const { return value == other.value; }
};

class idAction_MoveToPoint {
public:
  ~idAction_MoveToPoint();
};

class idAIActionManager {
public:
  ~idAIActionManager();
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

class idGoreComponent {};

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

class idAIFSMCallback {
public:
  virtual ~idAIFSMCallback();
  idRecoveredValue OnError(...);
  idRecoveredValue OnRestart(...);
  idRecoveredValue OnTransition(...);
};

class idAnimatedEntity : public idEntity {
public:
  idTreeAnimator* GetTreeAnimatorFromPresentable() const;
  idRecoveredValue SetModelByName(...);
  void SetPresentable(idPresentable* value) { presentable = value; }
  void SetAimPoint(const idVec3& value) { aimPoint = value; }
  const idVec3& GetAimPoint() const { return aimPoint; }
  void SetHeadTrackBlendTime(int value) { headTrackBlendTime = value; }
  int GetHeadTrackBlendTime() const { return headTrackBlendTime; }

private:
  idVec3 aimPoint{};
  int headTrackBlendTime{500};
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

  idRecoveredValue HasFocus(...);
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

class idFSMManager {
public:
  struct idFSMStack { idFSMStack(); };
  virtual ~idFSMManager();
};

class idFXManager {
public:
  void SetExternalTransform(const idVec3& position, const idMat3& rotation);
  const idVec3& GetExternalPosition() const { return externalPosition; }
  const idMat3& GetExternalRotation() const { return externalRotation; }
  bool HasExternalPositionAndRotation() const {
    return hasExternalPositionAndRotation;
  }

private:
  idMat3 externalRotation{};
  idVec3 externalPosition{};
  bool hasExternalPositionAndRotation{false};
};

class idLegsFSM {
public:
  ~idLegsFSM();
  int GetStatus() const { return status; }
  void SetStatus(int value) { status = value; }

private:
  int status{0};
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
struct idTrajectory_Parabolic { int value{0}; };
class idAIAction {};
class idEntitySpriteAnimator {};
class idProxyMove {};
class idTarget_AlarmSystem {};
class idTarget_FakeEnemy {};
class idVolume_CustomDeath {};

enum invalidAIEvent_t { INVALID_AI_EVENT = -1 };

template <int JointCount>
class idAnimator_TurretNxN {
public:
  virtual ~idAnimator_TurretNxN();
  idRecoveredValue SetParameters(...);
};

template <typename T, int Count>
class idArray {
public:
  ~idArray();
  T values[Count]{};
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
