#define WOLFTNO_RETAIL_CVAR_INLINE_FREEDATA_ABI
#define WOLFTNO_RETAIL_IDMATH_CONSTANT_ABI
#define WOLFTNO_RETAIL_IDROTATION_CLASS_ABI
#define WOLFTNO_RETAIL_IDVEC3_RADIANS_ABI
#include "engine/gamelib/effectphysics/effectphysicsrigidbody.h"

#include "engine/gamelib/effectphysics/effectphysicsproperties.h"
#include "shared/idlib/csystems/cvarsystemlocal.h"
#include "shared/idlib/math/mathlib.h"

#undef WOLFTNO_RETAIL_IDVEC3_RADIANS_ABI
#undef WOLFTNO_RETAIL_IDROTATION_CLASS_ABI
#undef WOLFTNO_RETAIL_IDMATH_CONSTANT_ABI
#undef WOLFTNO_RETAIL_CVAR_INLINE_FREEDATA_ABI

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>

namespace {

int failures;

#define CHECK(expression)                                                      \
  do {                                                                         \
    if (!(expression)) {                                                       \
      std::printf("FAIL line %d: %s\n", __LINE__, #expression);               \
      ++failures;                                                              \
    }                                                                          \
  } while (false)

bool Near(float left, float right, float epsilon = 2.0e-5f) {
  if (std::isnan(left) || std::isnan(right)) {
    return std::isnan(left) && std::isnan(right);
  }
  return std::fabs(left - right) <=
         epsilon * std::max(1.0f, std::max(std::fabs(left), std::fabs(right)));
}

void CheckVec(const idVec3& actual, const idVec3& expected,
              float epsilon = 2.0e-5f) {
  if (!Near(actual.x, expected.x, epsilon) ||
      !Near(actual.y, expected.y, epsilon) ||
      !Near(actual.z, expected.z, epsilon)) {
    std::printf("  actual=(%.9g %.9g %.9g) expected=(%.9g %.9g %.9g)\n",
                actual.x, actual.y, actual.z, expected.x, expected.y,
                expected.z);
  }
  CHECK(Near(actual.x, expected.x, epsilon));
  CHECK(Near(actual.y, expected.y, epsilon));
  CHECK(Near(actual.z, expected.z, epsilon));
}

idMat3 Matrix(const idVec3& row0, const idVec3& row1,
              const idVec3& row2) {
  idMat3 result;
  result.mat[0] = row0;
  result.mat[1] = row1;
  result.mat[2] = row2;
  return result;
}

idMat3 Identity() {
  return Matrix(idVec3{1.0f, 0.0f, 0.0f},
                idVec3{0.0f, 1.0f, 0.0f},
                idVec3{0.0f, 0.0f, 1.0f});
}

idMat3 Transposed(const idMat3& value) {
  return Matrix(idVec3{value.mat[0].x, value.mat[1].x, value.mat[2].x},
                idVec3{value.mat[0].y, value.mat[1].y, value.mat[2].y},
                idVec3{value.mat[0].z, value.mat[1].z, value.mat[2].z});
}

idVec3 MultiplyVector(const idMat3& matrix, const idVec3& vector) {
  return idVec3{
      (vector.x * matrix.mat[0].x + vector.y * matrix.mat[1].x) +
          vector.z * matrix.mat[2].x,
      (vector.x * matrix.mat[0].y + vector.y * matrix.mat[1].y) +
          vector.z * matrix.mat[2].y,
      (vector.x * matrix.mat[0].z + vector.y * matrix.mat[1].z) +
          vector.z * matrix.mat[2].z};
}

idVec3 CrossRef(const idVec3& left, const idVec3& right) {
  return idVec3{left.y * right.z - left.z * right.y,
                left.z * right.x - left.x * right.z,
                left.x * right.y - left.y * right.x};
}

float LengthSquaredRef(const idVec3& value) {
  return (value.y * value.y + value.x * value.x) + value.z * value.z;
}

struct StubState {
  int spCtorCalls;
  int spDtorCalls;
  int addCalls;
  int removeCalls;
  int setCalls;
  int applyCalls;
  int getCalls;
  bool throwRemove;
  idSPObject* object;
  idVec3 vectorArg;
  idMat3 matrixArg;
  idRotation rotationArg;
  const idBounds* boundsArg;
  trace_t result;
};

StubState stub;

void ResetStub() {
  std::memset(&stub, 0, sizeof(stub));
  stub.result.fraction = 1.0f;
  stub.result.endAxis = Identity();
}

idEffectPhysicsProperties MakeProperties(idEffectPhysicsBroadPhase& broadPhase) {
  idEffectPhysicsProperties properties{};
  properties.broadPhase = &broadPhase;
  properties.traceModelIndex = 17;
  properties.clipMask = 0x1234;
  properties.linearFriction = 0.2f;
  properties.angularFriction = 0.3f;
  properties.contactFriction = 0.4f;
  properties.linearFrictionWater = 0.9f;
  properties.angularFrictionWater = 0.8f;
  properties.bouncyness = 0.5f;
  properties.gravityVector = idVec3{0.0f, 0.0f, -9.0f};
  properties.worldCollisionOnly = true;
  properties.simplePointCollision = false;
  properties.crazyBounceChance = 0.0f;
  properties.mass = 2.0f;
  properties.inverseMass = 0.5f;
  properties.inverseInertiaTensor = Identity();
  properties.maxLinearVelocity = 1000.0f;
  properties.maxAngularVelocity = 1000.0f;
  properties.maxSimulationTime = 10;
  return properties;
}

} // namespace

alignas(16) const __m128 idMath::vector_float_smallestNonDenorm =
    _mm_set1_ps(1.17549435e-38f);
const std::uint64_t idCollisionQuery::finalResultsFirstSubmittedOffset = 100;
const std::uint64_t idCollisionQuery::finalResultsLastSubmittedOffset = 200;
const idMat3 mat3_identity = Identity();

idMemLocal* idMem::Singleton() {
  alignas(idMemLocal) static unsigned char storage[sizeof(idMemLocal)];
  return reinterpret_cast<idMemLocal*>(storage);
}

void idStr::FreeData() {}

idCVar::idCVar(const char* cvarName, const char* value, int cvarFlags,
               const char* cvarDescription, argCompletionRef_t) {
  std::memset(this, 0, sizeof(*this));
  name = cvarName;
  resetString = value;
  description = cvarDescription;
  flags = cvarFlags;
  valueFloat = std::strtof(value, nullptr);
  valueInteger = static_cast<int>(valueFloat);
}

idSPObject::idSPObject() {
  ++stub.spCtorCalls;
  id = 0x2468;
}

idSPObject::~idSPObject() noexcept(false) { ++stub.spDtorCalls; }

idEffectPhysicsBroadPhase::idEffectPhysicsBroadPhase() = default;
idEffectPhysicsBroadPhase::~idEffectPhysicsBroadPhase() = default;

idEffectPhysicsProperties::idEffectPhysicsProperties() {
  std::memset(this, 0, sizeof(*this));
}

void idEffectPhysicsBroadPhase::AddObject(idSPObject* inputObject,
                                           const idVec3& origin,
                                           const idMat3& axis,
                                           const idBounds* bounds) {
  ++stub.addCalls;
  stub.object = inputObject;
  stub.vectorArg = origin;
  stub.matrixArg = axis;
  stub.boundsArg = bounds;
}

void idEffectPhysicsBroadPhase::RemoveObject(idSPObject* inputObject) {
  ++stub.removeCalls;
  stub.object = inputObject;
  if (stub.throwRemove) {
    throw std::runtime_error("remove");
  }
}

void idEffectPhysicsBroadPhase::SetObjectPosition(idSPObject* inputObject,
                                                   const idVec3& origin,
                                                   const idMat3& axis,
                                                   const idBounds* bounds) {
  ++stub.setCalls;
  stub.object = inputObject;
  stub.vectorArg = origin;
  stub.matrixArg = axis;
  stub.boundsArg = bounds;
}

void idEffectPhysicsBroadPhase::ApplyMotion(idSPObject* inputObject,
                                             const idVec3& translation,
                                             const idRotation& rotation) {
  ++stub.applyCalls;
  stub.object = inputObject;
  stub.vectorArg = translation;
  stub.rotationArg = rotation;
}

bool idEffectPhysicsBroadPhase::GetMotionResult(trace_t* output,
                                                 idSPObject* inputObject) {
  ++stub.getCalls;
  stub.object = inputObject;
  *output = stub.result;
  return output->fraction < 1.0f;
}

idMat3 idMat3::operator*(const idMat3& right) const {
  idMat3 result{};
  for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 3; ++column) {
      result.mat[row][column] =
          mat[row].x * right.mat[0][column] +
          mat[row].y * right.mat[1][column] +
          mat[row].z * right.mat[2][column];
    }
  }
  return result;
}

idQuat idMat3::ToQuat() const {
  idQuat result{};
  const float trace = mat[0].x + mat[1].y + mat[2].z;
  if (trace > 0.0f) {
    const float scale = std::sqrt(trace + 1.0f);
    result.w = scale * 0.5f;
    const float inverse = 0.5f / scale;
    result.x = (mat[2].y - mat[1].z) * inverse;
    result.y = (mat[0].z - mat[2].x) * inverse;
    result.z = (mat[1].x - mat[0].y) * inverse;
  } else {
    int i = mat[1].y > mat[0].x ? 1 : 0;
    if (mat[2].z > mat[i][i]) {
      i = 2;
    }
    const int j = (i + 1) % 3;
    const int k = (j + 1) % 3;
    const float scale =
        std::sqrt(std::max(0.0f, mat[i][i] - mat[j][j] - mat[k][k] + 1.0f));
    result[i] = scale * 0.5f;
    const float inverse = scale > 0.0f ? 0.5f / scale : 0.0f;
    result.w = (mat[k][j] - mat[j][k]) * inverse;
    result[j] = (mat[j][i] + mat[i][j]) * inverse;
    result[k] = (mat[k][i] + mat[i][k]) * inverse;
  }
  return result;
}

idQuat& idQuat::Normalize() {
  const float length = std::sqrt(x * x + y * y + z * z + w * w);
  if (length > 0.0f) {
    const float inverse = 1.0f / length;
    x *= inverse;
    y *= inverse;
    z *= inverse;
    w *= inverse;
  }
  return *this;
}

idQuat& idQuat::Slerp(const idQuat& from, const idQuat& to, float fraction) {
  if (fraction <= 0.0f) {
    return *this = from;
  }
  if (fraction >= 1.0f) {
    return *this = to;
  }
  float cosine = from * to;
  float sign = 1.0f;
  if (cosine < 0.0f) {
    cosine = -cosine;
    sign = -1.0f;
  }
  float fromScale;
  float toScale;
  if (1.0f - cosine <= 1.0e-6f) {
    fromScale = 1.0f - fraction;
    toScale = fraction;
  } else {
    const float angle = std::acos(std::clamp(cosine, -1.0f, 1.0f));
    const float inverseSine = 1.0f / std::sin(angle);
    fromScale = std::sin((1.0f - fraction) * angle) * inverseSine;
    toScale = std::sin(fraction * angle) * inverseSine;
  }
  *this = from * fromScale + to * (toScale * sign);
  return *this;
}

idMat3 idQuat::ToMat3() const {
  const float x2 = x + x;
  const float y2 = y + y;
  const float z2 = z + z;
  const float xx = x * x2;
  const float xy = x * y2;
  const float xz = x * z2;
  const float yy = y * y2;
  const float yz = y * z2;
  const float zz = z * z2;
  const float wx = w * x2;
  const float wy = w * y2;
  const float wz = w * z2;
  return Matrix(idVec3{1.0f - (yy + zz), xy - wz, xz + wy},
                idVec3{xy + wz, 1.0f - (xx + zz), yz - wx},
                idVec3{xz - wy, yz + wx, 1.0f - (xx + yy)});
}

idRotation::idRotation()
    : origin{0.0f, 0.0f, 0.0f}, vec{0.0f, 0.0f, 1.0f}, angle(0.0f),
      axis(Identity()), axisValid(false) {}

const idMat3& idRotation::ToMat3() const {
  if (!axisValid) {
    const float halfAngle = angle * 0.01745329252f * 0.5f;
    const idQuat value{vec.x * std::sin(halfAngle),
                       vec.y * std::sin(halfAngle),
                       vec.z * std::sin(halfAngle), std::cos(halfAngle)};
    axis = value.ToMat3();
    axisValid = true;
  }
  return axis;
}

const idVec3& idVec3::RotateSelf(
    const idVec3& rotationAxis,
    idTypesafeNumber<float, RadiansUnique_t> angleRadians) {
  const float halfAngle = static_cast<float>(angleRadians) * 0.5f;
  const idQuat rotation{rotationAxis.x * std::sin(halfAngle),
                        rotationAxis.y * std::sin(halfAngle),
                        rotationAxis.z * std::sin(halfAngle),
                        std::cos(halfAngle)};
  *this = rotation * *this;
  return *this;
}

idVec3 idQuat::operator*(const idVec3& vector) const {
  const idVec3 quaternionVector{x, y, z};
  const idVec3 first = CrossRef(quaternionVector, vector);
  const idVec3 second = CrossRef(quaternionVector, first);
  return vector + first * (2.0f * w) + second * 2.0f;
}

struct EffectPhysicsRigidBodyValidation {
  static void Cap(const idEffectPhysicsRigidBody& body, float deltaTime,
                  idEffectPhysicsRigidBody::effectRBState_t& state) {
    body.CapVelocity(deltaTime, state);
  }

  static void Evolve(const idEffectPhysicsRigidBody& body, float deltaTime,
                     float unused,
                     const idEffectPhysicsRigidBody::effectRBState_t& state,
                     idEffectPhysicsRigidBody::effectRBDelta_t& delta) {
    body.Evolve(deltaTime, unused, state, delta);
  }

  static void Collision(
      const idEffectPhysicsRigidBody& body, trace_t& trace,
      const impactInfo_t& impact, float decay,
      idEffectPhysicsRigidBody::effectRBState_t& state,
      idVec3& normalVelocity, idVec3& impulse) {
    body.CollisionImpulse(trace, impact, decay, state, normalVelocity,
                          impulse);
  }
};

namespace {

void TestConstructorAndDestructor() {
  ResetStub();
  alignas(idEffectPhysicsRigidBody)
      unsigned char storage[sizeof(idEffectPhysicsRigidBody)];
  std::memset(storage, 0xA5, sizeof(storage));
  auto* body = new (storage) idEffectPhysicsRigidBody;
  CHECK(stub.spCtorCalls == 1);
  CHECK(body->properties == nullptr);
  CHECK(body->SPObject.id == 0x2468);
  CHECK(body->simulationTime == 0.0f);
  for (std::size_t index = 0; index < sizeof(storage); ++index) {
    const bool propertiesByte = index < 8;
    const bool spIdByte = index >= 0x98 && index < 0x140;
    const bool simulationTimeByte = index >= 0x144 && index < 0x148;
    if (!propertiesByte && !spIdByte && !simulationTimeByte) {
      CHECK(storage[index] == 0xA5);
    }
  }
  body->~idEffectPhysicsRigidBody();
  CHECK(stub.spDtorCalls == 1);

  ResetStub();
  idEffectPhysicsBroadPhase broadPhase{};
  idEffectPhysicsProperties properties = MakeProperties(broadPhase);
  std::memset(storage, 0, sizeof(storage));
  body = new (storage) idEffectPhysicsRigidBody;
  body->properties = &properties;
  body->enabled = true;
  body->active = true;
  stub.throwRemove = true;
  try {
    body->~idEffectPhysicsRigidBody();
    CHECK(false);
  } catch (const std::runtime_error&) {
    CHECK(stub.removeCalls == 1);
    CHECK(stub.spDtorCalls == 1);
    CHECK(!body->enabled);
    CHECK(body->properties == &properties);
    CHECK(body->active);
  }
}

void TestSetupDisableAndImpulse() {
  ResetStub();
  idEffectPhysicsBroadPhase broadPhase{};
  idEffectPhysicsProperties properties = MakeProperties(broadPhase);
  idEffectPhysicsRigidBody body;
  body.Setup(&properties);
  CHECK(body.properties == &properties);
  CHECK(body.enabled && !body.active && !body.settled);
  CHECK(body.SPObject.traceModelIndex == 17);
  CHECK(body.SPObject.clipMask == 0x1234);
  CHECK(body.SPObject.motion.worldCollisionOnly);
  CHECK(stub.addCalls == 1 && stub.object == &body.SPObject);
  CHECK(stub.boundsArg == nullptr);
  CheckVec(stub.vectorArg, idVec3{0.0f, 0.0f, 0.0f});
  const idMat3 identity = Identity();
  CHECK(std::memcmp(&stub.matrixArg, &identity, sizeof(idMat3)) == 0);

  body.currentState.position = idVec3{1.0f, 2.0f, 3.0f};
  body.currentState.linearMomentum = idVec3{4.0f, 5.0f, 6.0f};
  body.currentState.angularMomentum = idVec3{7.0f, 8.0f, 9.0f};
  body.settled = true;
  body.ApplyImpulse(idVec3{3.0f, 5.0f, 7.0f},
                    idVec3{-2.0f, 4.0f, 6.0f});
  CHECK(!body.settled);
  CheckVec(body.currentState.linearMomentum, idVec3{2.0f, 9.0f, 12.0f});
  CheckVec(body.currentState.angularMomentum,
           idVec3{9.0f, -12.0f, 23.0f});

  body.Disable();
  CHECK(!body.enabled && stub.removeCalls == 1);
  body.Disable();
  CHECK(stub.removeCalls == 1);
}

void TestCapVelocity() {
  ResetStub();
  idEffectPhysicsBroadPhase broadPhase{};
  idEffectPhysicsProperties properties = MakeProperties(broadPhase);
  properties.inverseMass = 0.5f;
  properties.maxLinearVelocity = 2.0f;
  properties.maxAngularVelocity = 3.0f;
  properties.inverseInertiaTensor =
      Matrix(idVec3{2.0f, 0.0f, 0.0f},
             idVec3{0.0f, 3.0f, 0.0f},
             idVec3{0.0f, 0.0f, 4.0f});
  idEffectPhysicsRigidBody body;
  body.properties = &properties;
  idEffectPhysicsRigidBody::effectRBState_t state{};
  state.orientation = Identity();
  state.linearMomentum = idVec3{6.0f, 8.0f, 0.0f};
  state.angularMomentum = idVec3{2.0f, 0.0f, 0.0f};
  EffectPhysicsRigidBodyValidation::Cap(body, 99.0f, state);
  CheckVec(state.linearMomentum, idVec3{2.4f, 3.2f, 0.0f});
  CheckVec(state.angularMomentum, idVec3{1.5f, 0.0f, 0.0f});

  state.linearMomentum = idVec3{1.0f, 2.0f, 3.0f};
  state.angularMomentum = idVec3{0.1f, 0.2f, 0.3f};
  properties.maxLinearVelocity = 100.0f;
  properties.maxAngularVelocity = 100.0f;
  EffectPhysicsRigidBodyValidation::Cap(body, -4.0f, state);
  CheckVec(state.linearMomentum, idVec3{1.0f, 2.0f, 3.0f});
  CheckVec(state.angularMomentum, idVec3{0.1f, 0.2f, 0.3f});
}

void TestEvolve() {
  ResetStub();
  idEffectPhysicsBroadPhase broadPhase{};
  idEffectPhysicsProperties properties = MakeProperties(broadPhase);
  properties.inverseMass = 0.25f;
  properties.linearFriction = 0.2f;
  properties.angularFriction = 0.3f;
  properties.inverseInertiaTensor = Identity();
  idEffectPhysicsRigidBody body;
  body.properties = &properties;
  idEffectPhysicsRigidBody::effectRBState_t state{};
  state.position = idVec3{4.0f, 5.0f, 6.0f};
  state.orientation = Identity();
  state.linearMomentum = idVec3{8.0f, -4.0f, 2.0f};
  state.angularMomentum = idVec3{0.0f, 0.0f, 2.0f};
  state.externalForce = idVec3{3.0f, 4.0f, 5.0f};
  state.externalTorque = idVec3{6.0f, 7.0f, 8.0f};
  idEffectPhysicsRigidBody::effectRBDelta_t first{};
  idEffectPhysicsRigidBody::effectRBDelta_t second{};
  EffectPhysicsRigidBodyValidation::Evolve(body, 0.5f, -100.0f, state,
                                            first);
  EffectPhysicsRigidBodyValidation::Evolve(body, 0.5f, 100.0f, state,
                                            second);
  CHECK(std::memcmp(&first, &second, sizeof(first)) == 0);
  CheckVec(first.translation, idVec3{1.0f, -0.5f, 0.25f});
  CheckVec(first.rotation.origin, state.position);
  CheckVec(first.rotation.vec, idVec3{0.0f, 0.0f, 1.0f});
  CHECK(Near(first.rotation.angle, -57.295776f));
  CHECK(!first.rotation.axisValid);
  CheckVec(first.linearForce, idVec3{0.7f, 2.4f, 2.3f});
  CheckVec(first.angularForce, idVec3{3.0f, 3.5f, 3.7f});

  state.angularMomentum = idVec3{0.0f, 0.0f, 0.0f};
  EffectPhysicsRigidBodyValidation::Evolve(body, 1.0f, 0.0f, state, first);
  CheckVec(first.rotation.vec, idVec3{0.0f, 0.0f, 1.0f});
  CHECK(first.rotation.angle == 0.0f);
}

void TestCollisionImpulse() {
  ResetStub();
  idEffectPhysicsBroadPhase broadPhase{};
  idEffectPhysicsProperties properties = MakeProperties(broadPhase);
  properties.inverseMass = 0.5f;
  properties.bouncyness = 0.5f;
  properties.contactFriction = 0.75f;
  properties.simplePointCollision = true;
  properties.inverseInertiaTensor = Identity();
  idEffectPhysicsRigidBody body;
  body.properties = &properties;
  idEffectPhysicsRigidBody::effectRBState_t state{};
  state.orientation = Identity();
  state.linearMomentum = idVec3{0.0f, -4.0f, 0.0f};
  trace_t trace{};
  trace.c.normal = idVec3{0.0f, 1.0f, 0.0f};
  trace.c.point = idVec3{2.0f, 0.0f, 0.0f};
  impactInfo_t impact{};
  impact.invMass = 0.5f;
  impact.invInertiaTensor = Identity();
  idVec3 normalVelocity{};
  idVec3 impulse{99.0f, 99.0f, 99.0f};
  EffectPhysicsRigidBodyValidation::Collision(
      body, trace, impact, 0.0f, state, normalVelocity, impulse);
  CheckVec(normalVelocity, idVec3{0.0f, 2.0f, 0.0f});
  CheckVec(impulse, idVec3{0.0f, 3.0f, 0.0f});
  CheckVec(state.linearMomentum, idVec3{0.0f, -1.0f, 0.0f});
  CheckVec(state.angularMomentum, idVec3{0.0f, 0.0f, 6.0f});

  properties.inverseMass = -2.0f;
  properties.contactFriction = 0.0f;
  impact.invMass = 0.0f;
  state.linearMomentum = idVec3{0.0f, 0.0f, 0.0f};
  state.angularMomentum = idVec3{0.0f, 0.0f, 0.0f};
  impulse = idVec3{7.0f, 8.0f, 9.0f};
  EffectPhysicsRigidBodyValidation::Collision(
      body, trace, impact, 0.0f, state, normalVelocity, impulse);
  CheckVec(impulse, idVec3{7.0f, 8.0f, 9.0f});
  CheckVec(state.linearMomentum, impulse);
  CheckVec(state.angularMomentum, idVec3{0.0f, -18.0f, 16.0f});
}

void TestUpdate() {
  ResetStub();
  idEffectPhysicsBroadPhase broadPhase{};
  idEffectPhysicsProperties properties = MakeProperties(broadPhase);
  properties.linearFriction = 0.0f;
  properties.angularFriction = 0.0f;
  properties.gravityVector = idVec3{0.0f, 0.0f, -10.0f};
  properties.mass = 2.0f;
  properties.inverseMass = 0.5f;
  idEffectPhysicsRigidBody body;
  body.properties = &properties;
  body.currentState = {};
  body.currentState.orientation = Identity();
  body.currentState.orientationFiltered = Identity();
  body.currentState.linearMomentum = idVec3{2.0f, 0.0f, 0.0f};
  body.simulationTimeFirstContact = -1.0f;
  body.active = false;
  body.settled = false;
  body.enabled = true;
  idEffectPhysicsCollision collision{};
  collision.collisionSurfaceType = 77;
  body.Update(0.25f, collision);
  CHECK(collision.collisionSurfaceType == 77 && stub.applyCalls == 0);

  body.active = true;
  body.SPObject.query.offset = 50;
  body.Update(0.25f, collision);
  CHECK(stub.getCalls == 0 && stub.applyCalls == 1);
  CheckVec(stub.vectorArg, idVec3{0.25f, 0.0f, -0.625f});
  CheckVec(body.currentState.position, idVec3{0.25f, 0.0f, -0.625f});
  CheckVec(body.currentState.linearMomentum, idVec3{2.0f, 0.0f, -5.0f});
  CHECK(Near(body.simulationTime, 0.25f));
  CHECK(collision.collisionSurfaceType == 0);

  ResetStub();
  body.currentState.position = idVec3{0.0f, 0.0f, 0.0f};
  body.currentState.positionFiltered = idVec3{10.0f, 20.0f, 30.0f};
  body.currentState.orientation = Identity();
  body.currentState.orientationFiltered = Identity();
  body.currentState.linearMomentum = idVec3{0.0f, 0.0f, 0.0f};
  body.currentState.angularMomentum = idVec3{0.0f, 0.0f, 0.0f};
  body.currentState.externalForce = idVec3{1.0f, 2.0f, 3.0f};
  body.currentState.externalTorque = idVec3{4.0f, 5.0f, 6.0f};
  body.SPObject.origin = idVec3{5.0f, 6.0f, 7.0f};
  body.SPObject.axis = Identity();
  body.SPObject.query.offset = 150;
  stub.result.fraction = 1.0f;
  body.Update(0.0f, collision);
  CHECK(stub.getCalls == 1 && stub.applyCalls == 1 && stub.setCalls == 0);
  CheckVec(body.currentState.position, idVec3{5.0f, 6.0f, 7.0f});
  CheckVec(body.currentState.positionFiltered,
           idVec3{5.5f, 7.4f, 9.3f});
  CheckVec(body.currentState.externalForce, idVec3{0.0f, 0.0f, 0.0f});
  CheckVec(body.currentState.externalTorque, idVec3{0.0f, 0.0f, 0.0f});

  ResetStub();
  body.currentState.position = idVec3{0.0f, 0.0f, 0.0f};
  body.currentState.positionFiltered = idVec3{0.0f, 0.0f, 0.0f};
  body.currentState.orientation = Identity();
  body.currentState.orientationFiltered = Identity();
  body.currentState.linearMomentum = idVec3{0.0f, -0.1f, 0.0f};
  body.currentState.angularMomentum = idVec3{0.0f, 0.0f, 0.0f};
  body.SPObject.origin = idVec3{0.0f, 0.0f, 0.0f};
  body.SPObject.axis = Identity();
  body.SPObject.query.offset = 150;
  body.simulationTime = 2.0f;
  body.simulationTimeFirstContact = -1.0f;
  properties.gravityVector = idVec3{0.0f, 0.0f, 0.0f};
  properties.simplePointCollision = true;
  properties.contactFriction = 0.0f;
  stub.result.fraction = 0.5f;
  stub.result.c.normal = idVec3{0.0f, 1.0f, 0.0f};
  stub.result.c.point = idVec3{1.0f, 2.0f, 3.0f};
  stub.result.c.surfaceType = 9;
  stub.result.c.physicsId = 2;
  body.Update(0.0f, collision);
  CHECK(stub.setCalls == 1);
  CHECK(body.simulationTimeFirstContact == 2.0f);
  CHECK(body.settled);
  CHECK(!body.SPObject.motion.valid);
  CHECK(collision.collisionSurfaceType == 0);
  CheckVec(body.currentState.linearMomentum, idVec3{0.0f, 0.0f, 0.0f});
}

} // namespace

int main() {
  TestConstructorAndDestructor();
  TestSetupDisableAndImpulse();
  TestCapVelocity();
  TestEvolve();
  TestCollisionImpulse();
  TestUpdate();
  if (failures != 0) {
    std::printf("effectphysicsrigidbody validation failed: %d\n", failures);
    return 1;
  }
  std::puts("effectphysicsrigidbody validation passed");
  return 0;
}
