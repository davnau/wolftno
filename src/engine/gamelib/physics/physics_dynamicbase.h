#pragma once

#include "engine/gamelib/physics/physics.h"

class idPhysics_DynamicBase : public idPhysics {
public:
  idPhysics_DynamicBase();
  ~idPhysics_DynamicBase() override;

  void SetClipModel(idClipModel*, float, int, bool) override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  idClipModel* GetClipModel(int) const override;
  int GetNumClipModels() const override;
#else
  idClipModel* GetClipModel(int) override;
  int GetNumClipModels() override;
#endif
  void SetMass(float, int) override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  float GetMass(int) const override;
#else
  float GetMass(int) override;
#endif
  void SetContents(int, int) override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  int GetContents(int) const override;
#else
  int GetContents(int) override;
#endif
  void SetClipMask(int, int) override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  int GetClipMask(int) const override;
  const idBounds& GetBounds(int) const override;
  const idBounds& GetAbsBounds(int) const override;
#else
  int GetClipMask(int) override;
  const idBounds& GetBounds(int) override;
  const idBounds& GetAbsBounds(int) override;
#endif
  void SetOrigin(const idVec3&, int) override;
  void SetAxis(const idMat3&, int) override;
  void Translate(const idVec3&, int) override;
  void Rotate(const idRotation&, int) override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  const idVec3& GetOrigin(int) const override;
  const idMat3& GetAxis(int) const override;
#else
  const idVec3& GetOrigin(int) override;
  const idMat3& GetAxis(int) override;
#endif
  const idVec3& GetLocalOrigin(int) override;
  const idMat3& GetLocalAxis(int) override;
  void SetLinearVelocity(const idVec3&, int) override;
  void SetAngularVelocity(const idVec3&, int) override;
  idVec3 GetLinearVelocity(int) const override;
  idVec3 GetAngularVelocity(int) override;
  void SetGravity(const idVec3&) override;
  const idVec3& GetGravity() override;
  const idVec3& GetGravityNormal() override;
  void SetWaterLevel(float, int, bool, int, float) override;
  float GetWaterLevel(int) override;
  void SetWaterViscosity(float, int) override;
  float GetWaterViscosity(int) override;
  int GetWaterEntNum() override;
  float GetWaterSurfaceWrldHeight() override;
  float GetBuoyancyScale() override;
  void GetImpactInfo(int, const idVec3&, impactInfo_t&) const override;
  void ApplyImpulse(int, const idVec3&, const idVec3&) override;
  void ApplyForce(int, const idVec3&, const idVec3&) override;
  void Activate() override;
  void PutToRest() override;
  bool IsAtRest() const override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  bool IsPushable(int) const override;
#else
  bool IsPushable(int) override;
#endif
  void SaveState() override;
  void RestoreState() override;
  bool Evaluate(int, int) override;
  void UpdateTime(int) override;
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_CONST_ABI)
  void ClipTranslation(trace_t&, const idVec3&,
                       const idClipModel*) const override;
  void ClipRotation(trace_t&, const idRotation&,
                    const idClipModel*) const override;
  int ClipContents(const idClipModel*, int) const override;
#else
  void ClipTranslation(trace_t&, const idVec3&, const idClipModel*) override;
  void ClipRotation(trace_t&, const idRotation&, const idClipModel*) override;
  int ClipContents(const idClipModel*, int) override;
#endif
  void DisableClip() override;
  void EnableClip() override;
  void UnlinkClip() override;
  void LinkClip() override;
  bool EvaluateContacts() override;
  int GetNumContacts() override;
  const contactInfo_t& GetContact(int) override;
  void ClearContacts() override;
  void AddContactPhysics(idPhysics*) override;
  void RemoveContactPhysics(idPhysics*) override;
  int GetNumContactPhysics() override;
  idPhysics* GetContactPhysics(int) override;
  void ActivateContactPhysics() override;
  bool HasGroundContacts() override;
  bool IsGroundEntity(int) override;
  bool IsGroundClipModel(int, int) override;
  void SetPushed(int) override;
  idVec3 GetPushedLinearVelocity(int) const override;
  idVec3 GetPushedAngularVelocity(int) override;
  void SetMaster(bool, const idVec3&, const idMat3&, bindFlags_t) override;
  void SetLocalOrigin(const idVec3&, int) override;
  void SetLocalAxis(const idMat3&, int) override;
  int GetBlockingEntityNum() override;
  int GetLinearEndTime() override;
  int GetAngularEndTime() override;
  virtual bool IsOutsideWorld();
#if defined(WOLFTNO_RETAIL_PHYSICS_ACTOR_GRAVITY_SLOT_ABI)
  virtual const idMat3& GetGravityAxis() const = 0;
#endif

protected:
  void AddGroundContacts(const idClipModel* model, int mask);
  void AddContactPhysicsForContacts();

  int clipMask;
  idVec3 gravityVector;
  idVec3 gravityNormal;
  idList<contactInfo_t, 77> contacts;
  idList<int, 77> contactPhysicsIds;
  float waterLevel;
  float waterViscosity;
};

static_assert(sizeof(idPhysics_DynamicBase) == 0x98,
              "idPhysics_DynamicBase ABI drift");
