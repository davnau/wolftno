#pragma once

#include "tungsten/game/clientgame/presentable/presentable.h"

#include <string>
#include <vector>

class idPresentableActor {
public:
  void* GetActorModifierManager();
  void SetActorModifierManager(void* value) { modifierManager=value; }
private: void* modifierManager{nullptr};
};

class idPresentableAI : public idPresentable {
public:
  idPresentableAI();
  ~idPresentableAI() override;
  void InternalSplatterBlood(const idVec3& position,float amount);
  void InternalPlayAdditivePain(const std::string& animation,float weight);
  bool InternalLootBody(int playerIndex);
  void Interpolate(float fraction);
  void UpdateInfluenceTrail(const idVec3& position);
  bool ShouldSerializeRenderModelParms()const;
  void SetDefaults();
  std::vector<std::uint8_t> Serialize()const;
  void SplatterBlood(const idVec3& position,float amount);
  void PlayAdditivePain(const std::string& animation,float weight);
  bool LootBody(int playerIndex);
  void ClientThink();
  bool PredictHitScanHit(const idVec3& start,const idVec3& direction,float range)const;

  int GetBloodSplatterCount()const{return bloodSplatterCount;} const std::string& GetPainAnimation()const{return painAnimation;} bool IsLooted()const{return looted;} int InfluenceTrailSize()const{return static_cast<int>(influenceTrail.size());}
private:
  std::vector<idVec3> influenceTrail; std::string painAnimation; float painWeight{0}; int bloodSplatterCount{0}; int lootedBy{-1}; bool looted{false};
};

extern idCVar presentable_ai_serialize_render_parms;
extern idCVar ai_influenceForce;
