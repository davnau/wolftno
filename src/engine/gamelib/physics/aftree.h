#pragma once

#include "engine/gamelib/physics/clip.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/math/spatialmat.h"
#include "shared/idlib/math/vector.h"
#include "shared/idlib/text/atomicstring.h"

#include <cstddef>
#include <cstdint>

class idAFConstraint;
class idAFTree;
class idCVar;

struct AFBodyPState_t {
  idVec3 worldOrigin;
  idMat3 worldAxis;
  idVec3 atRestOrigin;
  idMat3 atRestAxis;
  idVec6 spatialVelocity;
  idVec6 externalForce;
};

class alignas(16) idAFBody {
public:
  void InverseWorldSpatialInertiaMultiply(idSpatialVec& destination,
                                           const idSpatialVec& vector) const;

  const void* const* vtable;
  idAtomicString name;
  idClipModel* clipModel;
  int clipMask;
  float linearFrictionWater;
  float angularFrictionWater;
  float linearFriction;
  float angularFriction;
  float contactFriction;
  float bouncyness;
  idVec3 frictionDir;
  idVec3 contactMotorDir;
  float contactMotorVelocity;
  float contactMotorForce;
  float mass;
  float invMass;
  idVec3 centerOfMass;
  idMat3 inertiaTensor;
  idMat3 inverseInertiaTensor;
  idAFBody* parent;
  idList<idAFBody*, 71> children;
  idAFConstraint* primaryConstraint;
  idList<idAFConstraint*, 71> constraints;
  idAFTree* tree;
  std::uint8_t fl;
  std::uint8_t recoveredPadF9[3];
  AFBodyPState_t current;
  AFBodyPState_t saved;
  std::uint8_t recoveredPad21C[4];
  idClipQuery motionQuery;
  idSpatialMat inverseWorldSpatialInertia;
  idSpatialMat I;
  idSpatialMat invI;
  idSpatialMat J;
  std::uint8_t recoveredPad288[8];
  idStaticSpatialVec s;
  idStaticSpatialVec totalForce;
  idStaticSpatialVec auxForce;
  idStaticSpatialVec acceleration;
  float* response;
  int* responseIndex;
  int numResponses;
  int maxAuxiliaryIndex;
  int maxSubTreeAuxiliaryIndex;
  std::uint8_t recoveredPad36C[4];
};

class idAFConstraint {
public:
  const void* const* vtable;
  int type;
  std::uint8_t recoveredPad0C[4];
  idAtomicString name;
  idAFBody* body1;
  idAFBody* body2;
  void* physics;
  idStaticSpatialVec lm;
  idSpatialMat J1;
  idSpatialMat J2;
  idStaticSpatialVec c1;
  idStaticSpatialVec c2;
  idStaticSpatialVec lo;
  idStaticSpatialVec hi;
  idStaticSpatialVec e;
  idAFConstraint* boxConstraint;
  int boxIndex[6];
  float boxScale[6];
  idSpatialMat invI;
  idSpatialMat J;
  int firstIndex;
  std::uint8_t fl;
  std::uint8_t recoveredPad1ED[3];
};

class idAFTree {
public:
  void Factor() const;
  void Solve(int auxiliaryIndex) const;
  void Response(const idAFConstraint* constraint, int row,
                int auxiliaryIndex) const;
  void CalculateForces(float timeStep) const;
  void SetMaxSubTreeAuxiliaryIndex();
  void SortBodies();
  void DebugDraw(const idVec4& color) const;

  const idList<idAFBody*, 71>& GetSortedBodies() const {
    return sortedBodies;
  }

  idList<idAFBody*, 71> sortedBodies;

private:
  void SortBodies_r(idList<idAFBody*, 71>& sortedList,
                    idAFBody* body);
};

extern idCVar af_useSymmetry;

static_assert(sizeof(AFBodyPState_t) == 0x90,
              "AF body state ABI drift");
static_assert(sizeof(idAFBody) == 0x370, "idAFBody ABI drift");
static_assert(sizeof(idAFConstraint) == 0x1F0,
              "idAFConstraint ABI drift");
static_assert(sizeof(idAFTree) == 0x18, "idAFTree ABI drift");
static_assert(offsetof(idAFBody, parent) == 0xB0,
              "idAFBody parent offset drift");
static_assert(offsetof(idAFBody, children) == 0xB8,
              "idAFBody children offset drift");
static_assert(offsetof(idAFBody, primaryConstraint) == 0xD0,
              "idAFBody primary constraint offset drift");
static_assert(offsetof(idAFBody, tree) == 0xF0,
              "idAFBody tree offset drift");
static_assert(offsetof(idAFBody, fl) == 0xF8,
              "idAFBody flags offset drift");
static_assert(offsetof(idAFBody, current) == 0xFC,
              "idAFBody current-state offset drift");
static_assert(offsetof(idAFBody, inverseWorldSpatialInertia) == 0x228,
              "idAFBody world inertia offset drift");
static_assert(offsetof(idAFBody, I) == 0x240,
              "idAFBody articulated inertia offset drift");
static_assert(offsetof(idAFBody, invI) == 0x258,
              "idAFBody inverse inertia offset drift");
static_assert(offsetof(idAFBody, J) == 0x270,
              "idAFBody Jacobian offset drift");
static_assert(offsetof(idAFBody, s) == 0x290,
              "idAFBody spatial response offset drift");
static_assert(offsetof(idAFBody, response) == 0x350,
              "idAFBody response buffer offset drift");
static_assert(offsetof(idAFConstraint, lm) == 0x30,
              "idAFConstraint multiplier offset drift");
static_assert(offsetof(idAFConstraint, J1) == 0x60,
              "idAFConstraint J1 offset drift");
static_assert(offsetof(idAFConstraint, invI) == 0x1B8,
              "idAFConstraint inverse offset drift");
static_assert(offsetof(idAFConstraint, fl) == 0x1EC,
              "idAFConstraint flags offset drift");
