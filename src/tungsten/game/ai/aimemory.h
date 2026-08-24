#pragma once

#include "tungsten/game/ai/ai2_actions.h"

#include <array>

enum aiVarOp_t { AIVAR_SET, AIVAR_ADD, AIVAR_SUBTRACT };
enum aiMelee_t { AIMELEE_CLOSE, AIMELEE_MEDIUM, AIMELEE_LONG };

struct idDeclAIBehavior {
  struct runTypeBehaviors_t {
    int scrambleRunIndex{0};
    int farRunIndex{0};
    int gunDownIndex{0};
    int narrowIndex{0};
    int upStairsIndex{0};
    int downStairsIndex{0};
    int duckIndex{0};
  } runTypeBehaviors;
  struct idleBehaviors_t {
    int gunDownIndex{0};
    int narrowIndex{0};
    int duckIndex{0};
  } idleBehaviors;
  int minAimDodgeInterval{0};
  int maxAimDodgeInterval{0};
  bool disablePainAfterActionScript{false};
};

class idInfoPath : public idEntity {
public:
  void SetNextPathPoint(idInfoPath* value) { nextPathPoint = value; }
  idInfoPath* GetNextPathPoint() const { return nextPathPoint; }
private:
  idInfoPath* nextPathPoint{nullptr};
};

class idScenePoint_UserGroup : public idEntity {};

class idVolume_Restriction : public idEntity {
public:
  idVolume_Restriction() {
    bounds[0] = {-100000.0F, -100000.0F, -100000.0F};
    bounds[1] = {100000.0F, 100000.0F, 100000.0F};
  }
  void SetActive(bool value) { active = value; }
  bool IsActive() const { return active; }
  void SetBounds(const idBounds& value) { bounds = value; }
  bool ContainsPoint(const idVec3& point) const {
    return point.x >= bounds[0].x && point.x <= bounds[1].x &&
           point.y >= bounds[0].y && point.y <= bounds[1].y &&
           point.z >= bounds[0].z && point.z <= bounds[1].z;
  }
private:
  bool active{true};
  idBounds bounds{};
};

class idVolume_ScenePoints : public idEntity {
public:
  void AddScenePoint(idScenePoint* value) {
    if (value != nullptr) scenePoints.Append(value);
  }
  void GetScenePoints(idList<idEntityPtr<idScenePoint>, 5>* output,
                      scenePointType_t type,
                      idVolume_Restriction* restriction,
                      idScenePoint_UserGroup* userGroup) const;
private:
  idList<idScenePoint*, 5> scenePoints;
};

class idVolume_LocationVoice : public idEntity {
public:
  bool CheckActivation(idAI2* ai, int currentTime,
                       const idVec3* suspectedPosition) {
    (void)ai;
    (void)suspectedPosition;
    if (!enabled || currentTime < nextActivationTime) return false;
    ++activationCount;
    nextActivationTime = reuseDelay < 0.0F
                             ? currentTime
                             : currentTime + static_cast<int>(reuseDelay * 1000.0F);
    return true;
  }
  void SetReuseDelay(float value) { reuseDelay = value; }
  int GetActivationCount() const { return activationCount; }
  int nextActivationTime{0};
  float reuseDelay{0.0F};
  bool enabled{true};
private:
  int activationCount{0};
};

class idAIMemory {
public:
  struct idScenePointList {
    void Clear();
    idList<idEntityPtr<idScenePoint>, 5> scenePointList;
    bool hasSearchedForScenePoints{false};
  };

  idAIMemory();
  ~idAIMemory();

  const idAIPos* DebugGetEnemyTrailPos(int index) const;
  bool CheckSoundAlarm(int currentTime);
  void SetAIVarExpression(const char* name, aiVarOp_t operation,
                          const char* value);
  float GetScriptedAggro(int currentTime) const;
  void SetNextMeleeTime(aiMelee_t meleeType, int currentTime);
  int GetNextMeleeTime(aiMelee_t meleeType) const;
  void ClearLastHeldScenePoint();
  idScenePoint* GetLastHeldScenePoint() const;
  void ActivateCurScenePoint(idAI2* user, int currentTime);
  void SetCurLookScenePoint(idAI2* user, int currentTime,
                            idScenePoint* scenePoint);
  void ActivateCurLookScenePoint(idAI2* user, int currentTime);
  void SetTakeScenePointTimeStamp(int currentTime);
  void SetActivateScenePointTimeStamp(int currentTime);
  void ClearScenePointTimeStamps();
  void FirstThink(const idDeclAIBehavior* behavior);
  void ClearOldestTrailPos();
  void ClearAllTrailPositions(int currentTime);
  void CheckVoiceVolumes(idAI2* ai, int currentTime,
                         const idVec3* suspectedPosition);
  void ReportInjured();
  void TriggerPulled();
  bool HasOverrideScriptScenePointOfType(scenePointType_t type) const;
  bool HasOverrideScriptScenePointNotOfType(scenePointType_t type) const;
  void UpdateScenePointHistory(idScenePoint* scenePoint);
  float GetScenePointHistoryScore(idScenePoint* scenePoint) const;
  void UpdateLookScenePointHistory(idScenePoint* scenePoint);
  bool IsLastHeldLookScenePoint(idScenePoint* scenePoint) const;
  float GetLookScenePointHistoryScore(idScenePoint* scenePoint) const;
  void SetOverrideScenePoint(idScenePoint* scenePoint);
  void AddEnemyTrailPos(int currentTime, const idVec3& enemyPosition);
  bool GetOldestTrailPos(idAIPos* trail) const;
  void SetLastDamage(idEntity* attacker, idEntity* inflictor);
  void SetCurrentPath(const idInfoPath* path);
  void ClearScenePointLists();
  void SetRestrictionVolume(idVolume_Restriction* volume);
  bool HasRestrictionVolume();
  void SetRestrictionPos(const idVec3& position);
  void RemoveScenePointVolume(idVolume_ScenePoints* volume);
  void SetCurScenePoint(idAI2* user, int currentTime,
                        idScenePoint* scenePoint);
  void SetScenePointUserGroup(idScenePoint_UserGroup* group);
  bool CheckIfDirtyScenePoint(idScenePoint* dirtyScenePoint);
  bool CheckIfDirtyLookScenePoint(idScenePoint* dirtyLookScenePoint);
  void GetScenePoints(idList<idEntityPtr<idScenePoint>, 5>* output,
                      scenePointType_t type);
  void AddScenePointVolume(idVolume_ScenePoints* volume);
  idEntity* GetActionEntity() const;
  void SetActionEntity(idEntity* value) { actionEntity = value; }

  void SetScriptedAggro(const idEntity* entity, float aggression, int endTime);
  void SetProjectileInfo(const idAIProjectileInfo* info);
  idRecoveredValue GetEnemy(...);
  idRecoveredValue GetFriendly(...);
  idRecoveredValue GetFriendlyBlocker(...);
  idRecoveredValue GetLastAttacker(...);
  idRecoveredValue ShouldAvoid(...);
  idRecoveredValue SetAvoidEntity(...);
  idRecoveredValue SetSpottedEnemyTrigger(...);
  idRecoveredValue GetSpottedEnemyTrigger(...);
  idRecoveredValue SetDetectedEnemyTrigger(...);
  idRecoveredValue GetDetectedEnemyTrigger(...);
  idRecoveredValue SetTriggeredInteraction(...);

  void SetCurrentTime(int value) { currentTime = value; }
  void SetSoundAlarmTime(int value) { timeToSoundAlarm = value; }
  void SetEnemyTrailUpdatePeriod(int value) { enemyTrailUpdatePeriod = value; }
  void AddVoiceVolume(idVolume_LocationVoice* value) {
    voiceVolumes.Append(value);
  }
  void SetCurScenePointDirty(bool value) { curScenePointDirty = value; }
  void SetCurLookScenePointDirty(bool value) {
    curLookScenePointDirty = value;
  }
  const idStr& GetAIVarName() const { return varName; }
  const idStr& GetAIVarValue() const { return varValue; }
  aiVarOp_t GetAIVarOperation() const { return varOp; }
  int GetInjuredTime() const { return injuredTime; }
  int GetLastDamageTime() const { return lastDamageTime; }
  idEntity* GetLastAttackerEntity() const { return lastAttacker; }
  idEntity* GetLastInflictorEntity() const { return lastInflictor; }
  const idInfoPath* GetCurrentPath() const;
  const idInfoPath* GetNextPath() const { return nextPath; }
  const idAIPos& GetRestrictionPos() const { return restrictionPos; }
  int GetCurScenePointTakeTime() const { return curScenePointTakeTime; }
  int GetCurScenePointActivateTime() const {
    return curScenePointActivateTime;
  }
  int GetBlockedShots() const { return numShotsThisPullBlocked; }
  void SetBlockedShots(int total, int consecutive) {
    numShotsThisPullBlocked = total;
    numShotsThisPullBlockedConsecutive = consecutive;
  }

  const idEntity* scriptedAggroEntity{nullptr};
  float scriptedAggression{0.0F};
  int scriptedAggroEndTime{0};
  idAIProjectileInfo projectileInfo;

private:
  idScenePointList& ListForType(scenePointType_t type);
  const idScenePointList& ListForType(scenePointType_t type) const;
  static bool Contains(const idList<idEntityPtr<idScenePoint>, 5>& list,
                       idScenePoint* point);

  std::array<idAIPos, 5> enemyTrailPositions{};
  int enemyTrailFinger{0};
  int enemyTrailUpdatePeriod{500};
  int nextEnemyTrailTime{0};
  int timeToSoundAlarm{-1};
  idStr varName;
  aiVarOp_t varOp{AIVAR_SET};
  idStr varValue;
  int nextCloseMeleeTime{0};
  int nextMediumMeleeTime{0};
  int nextLongMeleeTime{0};
  std::array<idScenePoint*, 5> scenePointHistory{};
  std::array<idScenePoint*, 5> lookScenePointHistory{};
  idScenePoint* curScenePoint{nullptr};
  idScenePoint* curLookScenePoint{nullptr};
  bool curScenePointActivated{false};
  bool curLookScenePointActivated{false};
  int curScenePointTakeTime{-1};
  int curScenePointActivateTime{-1};
  int spawnTime{0};
  int nextDodgeTime{0};
  idList<idVolume_LocationVoice*, 5> voiceVolumes;
  int nextVoiceVolumeCheckTime{0};
  int injuredTime{-1};
  int numShotsThisPullBlocked{0};
  int numShotsThisPullBlockedConsecutive{0};
  idScenePoint* overrideScenePoint{nullptr};
  idScenePoint* keptOverrideScenePoint{nullptr};
  idEntity* lastAttacker{nullptr};
  idEntity* lastInflictor{nullptr};
  int lastDamageTime{-1};
  const idInfoPath* currentPath{nullptr};
  const idInfoPath* nextPath{nullptr};
  idScenePointList combatScenePoints;
  idScenePointList searchScenePoints;
  idScenePointList roamScenePoints;
  idScenePointList lookScenePoints;
  idVolume_Restriction* restrictionVolume{nullptr};
  bool restrictionVolumeActive{false};
  idAIPos restrictionPos{};
  idList<idVolume_ScenePoints*, 5> scenePointVolumes;
  idScenePoint_UserGroup* scenePointUserGroup{nullptr};
  bool curScenePointDirty{false};
  bool curLookScenePointDirty{false};
  int currentTime{0};
  idEntity* actionEntity{nullptr};
};
