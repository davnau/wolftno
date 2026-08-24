#include "engine/models/particles/jobs/particlegen.h"

#include "shared/idlib/parallelism/paralleljoblist.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace {

constexpr float kRandomScale = 0.000030518509f;
constexpr float kDegreesToRadians = 0.017453292f;

std::uint32_t NextParticleRandom(idRandom2& random) {
  random.seed = 1664525u * random.seed + 1013904223u;
  return random.seed;
}

float ParticleUnitRandom(idRandom2& random) {
  return static_cast<float>((NextParticleRandom(random) >> 10) & 0x7fffu) *
         kRandomScale;
}

float ParticleSignedRandom(idRandom2& random) {
  return ParticleUnitRandom(random) * 2.0f - 1.0f;
}

idVec3 NormalizeParticleVector(const idVec3& vector) {
  const float inverseLength = 1.0f / std::sqrt(std::max(
      vector.LengthSqr(), std::numeric_limits<float>::min()));
  return vector * inverseLength;
}

std::uint8_t ParticleByte(float value) {
  return static_cast<std::uint8_t>(
      std::clamp(static_cast<int>(value * 255.0f), 0, 255));
}

std::uint8_t ParticleTextureByte(float value) {
  return static_cast<std::uint8_t>(std::clamp(
      static_cast<int>(std::clamp(value, 0.0f, 1.0f) * 255.5f), 0, 255));
}

std::int16_t QuantizeParticleTexCoord(float value) {
  constexpr float zero = 0.5f;
  constexpr float scale = 0.5f / 32767.0f;
  return static_cast<std::int16_t>(std::clamp(
      static_cast<int>(std::floor((value - zero) / scale + 0.5f)),
      -32768, 32767));
}

static void ComputeTextureAnimationEx(
    const idParticleStage* stage, const idLookupTable* tables,
    particleGen_t* gen, float& width, float& s, float& height, float& t,
    std::uint8_t& frameBlendFrac, bool& wrap) {
  const int animationRows = stage->texAnimation.useRandomRow
                                ? 1
                                : stage->texAnimation.numRows;
  const int frameCount = animationRows * stage->texAnimation.numColumns;
  int startFrame = stage->texAnimation.startFrame;
  if (startFrame < 0) {
    startFrame = frameCount > 1
                     ? static_cast<int>(((NextParticleRandom(gen->random) >> 10) &
                                         0x7fffu) % frameCount)
                     : 0;
  }

  float frame = 0.0f;
  if (stage->texAnimation.type == 0) {
    const float rate = stage->texAnimation.rate.Compute(
        tables, gen->parmVal, gen->random);
    frame = (stage->texAnimation.rate.calcType == PARTICLE_CALC_GENERIC
                 ? gen->totalAge
                 : 1.0f) * rate + startFrame;
  } else if (stage->texAnimation.type == 1) {
    const float rate = stage->texAnimation.rate.Compute(
        tables, gen->parmVal, gen->random);
    frame = std::min(
        (stage->texAnimation.rate.calcType == PARTICLE_CALC_GENERIC
             ? gen->frac
             : 1.0f) * rate + startFrame,
        static_cast<float>(frameCount) - 1.0f);
  } else if (stage->texAnimation.type == 2) {
    frame = static_cast<float>(frameCount) * gen->frac + startFrame;
  }

  const int integerFrame = static_cast<int>(frame);
  const float blend = frame - integerFrame;
  int row = 0;
  if (stage->texAnimation.useRandomRow) {
    if (stage->texAnimation.numRows > 1) {
      row = static_cast<int>(((NextParticleRandom(gen->random) >> 10) &
                              0x7fffu) % stage->texAnimation.numRows);
    }
  } else {
    row = integerFrame % frameCount / stage->texAnimation.numColumns;
  }
  const int column = integerFrame % stage->texAnimation.numColumns;
  width /= stage->texAnimation.numColumns;
  s += column * width - std::floor(column * width);
  height /= stage->texAnimation.numRows;
  t += row * height - std::floor(row * height);
  if (stage->texAnimation.useFrameBlending) {
    frameBlendFrac = ParticleByte(blend);
    wrap = stage->texAnimation.numRows > 1 &&
           !stage->texAnimation.useRandomRow &&
           column == stage->texAnimation.numColumns - 1;
  }
}

} // namespace

float particleMeshDistTri_t::CaluclateArea() {
  const idVec3 first = vertices[0] - vertices[1];
  const idVec3 second = vertices[0] - vertices[2];
  area = std::sqrt(std::max(first.Cross(second).LengthSqr(),
                            std::numeric_limits<float>::min())) *
         0.5f;
  return area;
}

void particleMeshDist_t::operator=(const particleMeshDist_t& other) {
  totalArea = other.totalArea;
  triangles = other.triangles;
  numTriangles = other.numTriangles;
  bounds = other.bounds;
  isCopy = 1;
}

void particleMeshDist_t::ConstructEmpty() {
  triangles = nullptr;
  bounds = {};
  totalArea = 0.0f;
  numTriangles = 0;
  isCopy = 0;
}

idVec3 particleMeshDist_t::GenerateRandomPointOnSurface(idRandom2& random) const {
  if (triangles == nullptr)
    return {};

  float remaining = ParticleUnitRandom(random) * totalArea - triangles[0].area;
  int triangleIndex = 0;
  const int lastTriangle = static_cast<int>(numTriangles) - 1;
  while (triangleIndex < lastTriangle && remaining > 0.0f) {
    ++triangleIndex;
    remaining -= triangles[triangleIndex].area;
  }

  const particleMeshDistTri_t& triangle = triangles[triangleIndex];
  const float root = std::sqrt(ParticleUnitRandom(random));
  const float second = ParticleUnitRandom(random);
  const float firstWeight = 1.0f - root;
  const float secondWeight = (1.0f - second) * root;
  const float thirdWeight = second * root;
  return triangle.vertices[0] * firstWeight +
         triangle.vertices[1] * secondWeight +
         triangle.vertices[2] * thirdWeight;
}

void prtLocation_t::Clear() {
  for (idParticleParm& parm : loc) {
    parm.val0 = 0.0f;
    parm.val1 = 0.0f;
    parm.variance = 0.0f;
    parm.calcType = PARTICLE_CALC_GENERIC;
  }
  diversity = 0;
}

particleMeshDist_t::particleMeshDist_t() { ConstructEmpty(); }

void particleMeshDist_t::Construct(
    std::uint16_t triangleCount, const particleMeshDistTri_t* source) {
  numTriangles = triangleCount;
  totalArea = 0.0f;
  triangles = new particleMeshDistTri_t[triangleCount];
  bounds[0] = {1.0e30f, 1.0e30f, 1.0e30f};
  bounds[1] = {-1.0e30f, -1.0e30f, -1.0e30f};
  for (std::uint16_t index = 0; index < triangleCount; ++index) {
    triangles[index] = source[index];
    for (const idVec3& vertex : triangles[index].vertices) {
      bounds[0].x = std::min(bounds[0].x, vertex.x);
      bounds[0].y = std::min(bounds[0].y, vertex.y);
      bounds[0].z = std::min(bounds[0].z, vertex.z);
      bounds[1].x = std::max(bounds[1].x, vertex.x);
      bounds[1].y = std::max(bounds[1].y, vertex.y);
      bounds[1].z = std::max(bounds[1].z, vertex.z);
    }
    totalArea += triangles[index].area;
  }
}

idMat3 ParticleStageAxis(const idParticleStage* stage, int diversity) {
  idMat3 result{};
  result.mat[0].x = 1.0f;
  result.mat[1].y = 1.0f;
  result.mat[2].z = 1.0f;
  const idVec3 axes[2] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
  idRandom2 random{static_cast<std::uint32_t>((diversity & 0x1f) << 10)};
  for (int index = 0; index < 2; ++index) {
    const float range = std::fabs(stage->direction.angleOffsetRange[index]);
    const float angle = ParticleUnitRandom(random) * (range * 2.0f) - range +
                        stage->direction.angleOffsetStart[index];
    if (std::fabs(angle) > std::numeric_limits<float>::epsilon())
      result.RotateSelf(axes[index], angle * kDegreesToRadians);
  }
  return result;
}

idVec3 ParticleOrigin(const particleInput_t* inputs, particleGen_t* gen,
                      idVec3& origin) {
  const idParticleStage* stage = inputs->stage;
  const idLookupTable* tables = inputs->tables;
  const idMat3 modelStageAxis = inputs->stageAxis * inputs->modelAxis;
  const float scaleX = stage->distribution.size[0].Compute(
                           tables, gen->parmVal, gen->random) *
                       inputs->distribScale.x;
  const float scaleY = stage->distribution.size[1].Compute(
                           tables, gen->parmVal, gen->random) *
                       inputs->distribScale.y;
  const float scaleZ = stage->distribution.size[2].Compute(
                           tables, gen->parmVal, gen->random) *
                       inputs->distribScale.z;

  switch (stage->distribution.type) {
  case PDIST_RECT:
    origin = {(stage->distribution.random ? ParticleSignedRandom(gen->random)
                                          : 1.0f) * scaleX,
              (stage->distribution.random ? ParticleSignedRandom(gen->random)
                                          : 1.0f) * scaleY,
              (stage->distribution.random ? ParticleSignedRandom(gen->random)
                                          : 1.0f) * scaleZ};
    break;
  case PDIST_CYLINDER: {
    const float angle = (stage->distribution.random
                             ? ParticleSignedRandom(gen->random)
                             : 1.0f) * 6.2831855f;
    const float vertical = stage->distribution.random
                               ? ParticleSignedRandom(gen->random)
                               : 1.0f;
    origin = {std::cos(angle) * scaleX, std::sin(angle) * scaleY,
              vertical * scaleZ};
    break;
  }
  case PDIST_SPHERE: {
    const idVec3 direction = stage->distribution.random
                                 ? NormalizeParticleVector(
                                       {ParticleSignedRandom(gen->random),
                                        ParticleSignedRandom(gen->random),
                                        ParticleSignedRandom(gen->random)})
                                 : idVec3{1.0f, 1.0f, 1.0f};
    origin = {direction.x * scaleX, direction.y * scaleY,
              direction.z * scaleZ};
    break;
  }
  case PDIST_RECT_SURFACE: {
    idVec3 direction{ParticleSignedRandom(gen->random),
                     ParticleSignedRandom(gen->random),
                     ParticleSignedRandom(gen->random)};
    switch (((NextParticleRandom(gen->random) >> 10) & 0x7fffu) % 6u) {
    case 0: direction.x = 1.0f; break;
    case 1: direction.y = 1.0f; break;
    case 2: direction.z = 1.0f; break;
    case 3: direction.x = -1.0f; break;
    case 4: direction.y = -1.0f; break;
    default: direction.z = -1.0f; break;
    }
    origin = {direction.x * scaleX, direction.y * scaleY,
              direction.z * scaleZ};
    break;
  }
  case PDIST_CYLINDER_SURFACE: {
    const float angle = ParticleSignedRandom(gen->random) * 6.2831855f;
    const idVec3 direction = NormalizeParticleVector(
        {std::cos(angle), std::sin(angle), 0.0f});
    origin = {direction.x * scaleX, direction.y * scaleY,
              ParticleSignedRandom(gen->random) * scaleZ};
    break;
  }
  case PDIST_SPHERE_SURFACE: {
    const idVec3 direction = NormalizeParticleVector(
        {ParticleSignedRandom(gen->random), ParticleSignedRandom(gen->random),
         ParticleSignedRandom(gen->random)});
    origin = {direction.x * scaleX, direction.y * scaleY,
              direction.z * scaleZ};
    break;
  }
  case PDIST_MESH_SURFACE:
    origin = inputs->meshDistribution != nullptr
                 ? inputs->meshDistribution->GenerateRandomPointOnSurface(
                       gen->random)
                 : idVec3{};
    break;
  default:
    origin = {};
    break;
  }

  if (stage->customPath.type == PPATH_HELIX) {
    const float axialSpeed = stage->customPath.parms[4].Compute(
        tables, gen->parmVal, gen->random);
    const float radialSpeed = stage->customPath.parms[3].Compute(
        tables, gen->parmVal, gen->random);
    const float phase = stage->customPath.useRandomSpawn
                            ? ParticleUnitRandom(gen->random) * 6.2831855f
                            : 0.0f;
    const float angle = radialSpeed * gen->cycleAge + phase;
    const float radiusX = stage->customPath.parms[0].Compute(
                              tables, gen->parmVal, gen->random) *
                          inputs->distribScale.x;
    const float radiusY = stage->customPath.parms[1].Compute(
                              tables, gen->parmVal, gen->random) *
                          inputs->distribScale.y;
    const float height = stage->customPath.parms[2].Compute(
        tables, gen->parmVal, gen->random);
    const float spawn = stage->customPath.useRandomSpawn
                            ? ParticleUnitRandom(gen->random) *
                                  height * inputs->distribScale.z
                            : 0.0f;
    origin += {std::cos(angle) * radiusX, std::sin(angle) * radiusY,
               axialSpeed * gen->cycleAge + spawn};
  } else if (stage->customPath.type == PPATH_FLIES) {
    const float radialSpeed = stage->customPath.parms[0].Compute(
        tables, gen->parmVal, gen->random);
    const float axialSpeed = stage->customPath.parms[1].Compute(
        tables, gen->parmVal, gen->random);
    const float firstScale = std::clamp(ParticleUnitRandom(gen->random),
                                        0.4f, 1.0f);
    const float secondScale = std::clamp(ParticleUnitRandom(gen->random),
                                         0.4f, 1.0f);
    const float firstPhase = stage->customPath.useRandomSpawn
                                 ? ParticleUnitRandom(gen->random) * 6.2831855f
                                 : 0.0f;
    const float secondPhase = stage->customPath.useRandomSpawn
                                  ? ParticleUnitRandom(gen->random) * 6.2831855f
                                  : 0.0f;
    const float firstAngle = firstScale * radialSpeed * gen->cycleAge +
                             firstPhase;
    const float secondAngle = secondScale * axialSpeed * gen->cycleAge +
                              secondPhase;
    const float radius = stage->customPath.parms[2].Compute(
                             tables, gen->parmVal, gen->random) *
                         inputs->distribScale.x;
    origin += {std::cos(firstAngle) * std::cos(secondAngle) * radius,
               std::sin(firstAngle) * std::cos(secondAngle) * radius,
               -std::sin(secondAngle) * radius};
  } else if (stage->customPath.type == PPATH_ORBIT) {
    const float speed = stage->customPath.parms[1].Compute(
        tables, gen->parmVal, gen->random);
    const float phase = stage->customPath.useRandomSpawn
                            ? ParticleUnitRandom(gen->random) * 6.2831855f
                            : 0.0f;
    const float radius = stage->customPath.parms[0].Compute(
        tables, gen->parmVal, gen->random);
    idVec3 orbit{std::cos(speed * gen->cycleAge + phase) * radius,
                 std::sin(speed * gen->cycleAge + phase) * radius, 0.0f};
    orbit = NormalizeParticleVector(orbit) * radius;
    origin += orbit;
  } else if (stage->customPath.type == PPATH_DRIP) {
    const float speed = stage->customPath.parms[0].Compute(
        tables, gen->parmVal, gen->random);
    origin.z -= speed * gen->cycleAge;
  }

  const float indexFraction = static_cast<float>(gen->index) /
                              std::max(static_cast<float>(inputs->totalParticles),
                                       1.0f);
  for (int axis = 0; axis < 3; ++axis) {
    origin[axis] += inputs->location->loc[axis].Compute(
        tables, indexFraction, gen->random);
    origin[axis] += stage->spawnLocation.spawnLocation[axis].Compute(
        tables, indexFraction, gen->random);
    origin[axis] += stage->offset.offset[axis].Compute(
        tables, gen->parmVal, gen->random);
  }

  idVec3 speed;
  for (int axis = 0; axis < 3; ++axis) {
    const float friction = std::clamp(stage->friction.friction[axis].Compute(
                                          tables, gen->parmVal, gen->random),
                                      0.0f, 1.0f);
    const float speedFraction = gen->frac -
        friction * 0.5f * gen->frac * gen->parmVal;
    speed[axis] = stage->speed.speed[axis].Compute(
        tables, speedFraction, gen->random);
  }

  idVec3 direction{};
  idVec3 speedMagnitude = speed;
  if (stage->direction.type == PDIR_CONE) {
    const float coneAngle = ParticleSignedRandom(gen->random) *
                            stage->direction.parms[0] * kDegreesToRadians;
    const float radialAngle = ParticleSignedRandom(gen->random) * 3.1415927f;
    direction = {std::cos(radialAngle) * std::sin(coneAngle),
                 std::sin(radialAngle) * std::sin(coneAngle),
                 std::cos(coneAngle)};
  } else if (stage->direction.type == PDIR_OUTWARD) {
    direction = NormalizeParticleVector(origin);
    direction.z += stage->direction.parms[0];
  } else {
    direction = NormalizeParticleVector(speed);
    speedMagnitude = {std::fabs(speed.x), std::fabs(speed.y),
                      std::fabs(speed.z)};
  }
  idVec3 velocity{direction.x * speedMagnitude.x,
                  direction.y * speedMagnitude.y,
                  direction.z * speedMagnitude.z};
  if (stage->direction.type == PDIR_CONE)
    velocity = stage->direction.coneAxis * velocity;
  velocity *= gen->particleLife;
  idVec3 orientationVelocity = velocity;

  idVec3 acceleration;
  for (int axis = 0; axis < 3; ++axis) {
    acceleration[axis] = stage->acceleration.acceleration[axis].Compute(
        tables, gen->parmVal, gen->random);
  }
  if (stage->acceleration.world)
    acceleration = modelStageAxis * acceleration;
  idVec3 gravity{0.0f, 0.0f,
                 -stage->gravity.gravity.Compute(
                     tables, gen->parmVal, gen->random)};
  if (stage->gravity.world)
    gravity = modelStageAxis * gravity;
  const float windBias = stage->systemProperties.windBias.Compute(
      tables, gen->parmVal, gen->random);
  const idVec3 instantaneous = acceleration + gravity * gen->cycleAge +
                               inputs->wind * windBias;
  velocity += instantaneous * gen->cycleAge;
  origin += velocity * gen->cycleAge;
  origin = inputs->stageAxis * origin;
  origin = inputs->globalAxis * origin + inputs->globalOrigin;

  const float depthOffset = stage->orientation.depthOffset.Compute(
      tables, gen->parmVal, gen->random);
  if (std::fabs(depthOffset) > std::numeric_limits<float>::epsilon()) {
    origin += NormalizeParticleVector(inputs->localViewOrg - origin) *
              depthOffset;
  }
  if (!stage->orientation.orientToVelOnly)
    orientationVelocity = velocity;
  return inputs->stageAxis * orientationVelocity + inputs->localVelocity;
}

void ParticleTexCoords(const idParticleStage* stage,
                       const idLookupTable* tables, particleGen_t* gen,
                       const idVec2& sMinMax, const idVec2& tMinMax,
                       int atlasWidth, int atlasHeight,
                       idTransparencyVert* verts) {
  float s = sMinMax.x;
  float width = sMinMax.y;
  float t = tMinMax.x;
  float height = tMinMax.y;
  std::uint8_t frameBlend = 0;
  bool wrap = false;
  if (stage->texAnimation.numColumns > 1 ||
      stage->texAnimation.numRows > 1) {
    ComputeTextureAnimationEx(stage, tables, gen, width, s, height, t,
                              frameBlend, wrap);
  }

  const bool flipS =
      stage->systemProperties.textureFlipS == PTEXTURE_FLIP_ALWAYS ||
      (stage->systemProperties.textureFlipS == PTEXTURE_FLIP_RANDOM &&
       (gen->index & 1) != 0);
  const bool flipT =
      stage->systemProperties.textureFlipT == PTEXTURE_FLIP_ALWAYS ||
      (stage->systemProperties.textureFlipT == PTEXTURE_FLIP_RANDOM &&
       (gen->index & 1) != 0);
  const float nextS = s + width;
  const float secondS = std::fmod(nextS, 1.0f);
  const float nextT = t + height;
  const float secondT = wrap ? std::fmod(nextT, 1.0f) : nextT;
  idVec2 textureS0 = flipS ? idVec2{nextS, s} : idVec2{s, nextS};
  idVec2 textureT0 = flipT ? idVec2{nextT, t} : idVec2{t, nextT};
  idVec2 textureS1 = flipS ? idVec2{secondS + width, secondS}
                           : idVec2{secondS, secondS + width};
  idVec2 textureT1 = flipT ? idVec2{secondT + height, secondT}
                           : idVec2{secondT, secondT + height};

  if ((stage->stageFlags & 4u) != 0) {
    const float halfS = 0.5f / (atlasWidth * stage->atlasScaleBias.x);
    const float halfT = 0.5f / (atlasHeight * stage->atlasScaleBias.y);
    for (int index = 0; index < 2; ++index) {
      textureS0[index] = std::clamp(textureS0[index], halfS, 1.0f - halfS) *
                             stage->atlasScaleBias.x +
                         stage->atlasScaleBias.z;
      textureS1[index] = std::clamp(textureS1[index], halfS, 1.0f - halfS) *
                             stage->atlasScaleBias.x +
                         stage->atlasScaleBias.z;
      textureT0[index] = std::clamp(textureT0[index], halfT, 1.0f - halfT) *
                             stage->atlasScaleBias.y +
                         stage->atlasScaleBias.w;
      textureT1[index] = std::clamp(textureT1[index], halfT, 1.0f - halfT) *
                             stage->atlasScaleBias.y +
                         stage->atlasScaleBias.w;
    }
  }

  const std::int16_t stS[2] = {QuantizeParticleTexCoord(textureS0.x),
                               QuantizeParticleTexCoord(textureS0.y)};
  const std::int16_t stT[2] = {QuantizeParticleTexCoord(textureT0.x),
                               QuantizeParticleTexCoord(textureT0.y)};
  const std::int16_t st1S[2] = {QuantizeParticleTexCoord(textureS1.x),
                                QuantizeParticleTexCoord(textureS1.y)};
  const std::int16_t st1T[2] = {QuantizeParticleTexCoord(textureT1.x),
                                QuantizeParticleTexCoord(textureT1.y)};
  const std::uint8_t unscaledS[2] = {
      ParticleTextureByte(sMinMax.x), ParticleTextureByte(sMinMax.y)};
  const std::uint8_t unscaledT[2] = {
      ParticleTextureByte(tMinMax.x), ParticleTextureByte(tMinMax.y)};
  constexpr int side[4] = {0, 1, 0, 1};
  constexpr int vertical[4] = {0, 0, 1, 1};
  for (int index = 0; index < 4; ++index) {
    verts[index].st[0] = stS[side[index]];
    verts[index].st[1] = stT[vertical[index]];
    verts[index].st1[0] = st1S[side[index]];
    verts[index].st1[1] = st1T[vertical[index]];
    verts[index].tangent[0] = unscaledS[side[index]];
    verts[index].tangent[1] = unscaledT[vertical[index]];
    verts[index].tangent[2] = frameBlend;
  }
}

void ParticleLeftAndUp(const idParticleStage* const stage,
                       const idLookupTable* tables, particleGen_t* gen,
                       const idVec3& viewUp, const idVec3& viewLeft,
                       float extraRotation, idVec3& left, idVec3& up) {
  if (stage->orientation.type == POR_XYZ) {
    const float initialX = stage->initialRotation.initialAngle[0].Compute(
        tables, gen->parmVal, gen->random);
    const float initialY = stage->initialRotation.initialAngle[1].Compute(
        tables, gen->parmVal, gen->random);
    const float initialZ = stage->initialRotation.initialAngle[2].Compute(
        tables, gen->parmVal, gen->random);
    const float rotateX = stage->rotation.rotation[0].Compute(
        tables, gen->parmVal, gen->random);
    const float rotateY = stage->rotation.rotation[1].Compute(
        tables, gen->parmVal, gen->random);
    const float rotateZ = stage->rotation.rotation[2].Compute(
        tables, gen->parmVal, gen->random);
    const float x = (initialX + rotateX * gen->particleLife) *
                    kDegreesToRadians;
    const float y = (initialY + rotateY * gen->particleLife) *
                    kDegreesToRadians;
    const float z = (initialZ + rotateZ * gen->particleLife) *
                    kDegreesToRadians;
    const float sx = std::sin(x), cx = std::cos(x);
    const float sy = std::sin(y), cy = std::cos(y);
    const float sz = std::sin(z), cz = std::cos(z);
    left = {cz * cy, sz * cy, -sy};
    up = {cy * sx * sz - cz * cx,
          cy * sx * cz + sz * cx,
          sy * sx};
    return;
  }

  const float initial = stage->initialRotation.initialAngle[0].Compute(
      tables, gen->parmVal, gen->random);
  const float rotation = stage->rotation.rotation[0].Compute(
      tables, gen->parmVal, gen->random) * gen->particleLife;
  const float signedRotation =
      (gen->index & 1) != 0 && stage->rotation.allowRotDirOverride
          ? -rotation
          : rotation;
  const float angle = (initial + signedRotation + extraRotation) *
                      kDegreesToRadians;
  const float sine = std::sin(angle);
  const float cosine = std::cos(angle);
  switch (stage->orientation.type) {
  case POR_Z:
    left = {sine, cosine, 0.0f};
    up = {cosine, -sine, 0.0f};
    break;
  case POR_X:
    left = {0.0f, cosine, sine};
    up = {0.0f, -sine, cosine};
    break;
  case POR_Y:
    left = {cosine, 0.0f, sine};
    up = {-sine, 0.0f, cosine};
    break;
  case POR_VIEW:
  default:
    left = viewLeft * cosine + viewUp * sine;
    up = viewUp * cosine - viewLeft * sine;
    break;
  }
}

static int GenerateStaticMeshVerts(const particleInput_t* inputs,
                                   particleGen_t* gen,
                                   particleOutput_t* outputs,
                                   const idVec3& origin,
                                   std::uint8_t alphaScale,
                                   std::uint8_t* genericParm) {
  const idParticleStage* stage = inputs->stage;
  float s = 0.0f, width = 1.0f, t = 0.0f, height = 1.0f;
  std::uint8_t frameBlend = 0;
  bool wrap = false;
  if (stage->texAnimation.numColumns > 1 ||
      stage->texAnimation.numRows > 1) {
    ComputeTextureAnimationEx(stage, inputs->tables, gen, width, s, height,
                              t, frameBlend, wrap);
  }
  const idVec3 meshScale{
      stage->size.size[0].Compute(inputs->tables, gen->parmVal, gen->random) *
          inputs->sizeScale,
      stage->size.size[1].Compute(inputs->tables, gen->parmVal, gen->random) *
          inputs->sizeScale,
      stage->size.size[2].Compute(inputs->tables, gen->parmVal, gen->random) *
          inputs->sizeScale};
  idVec3 left, up;
  ParticleLeftAndUp(stage, inputs->tables, gen, inputs->localViewUp,
                    inputs->localViewLeft, 0.0f, left, up);
  idVec3 forward = NormalizeParticleVector(left.Cross(up));
  idMat3 axis{};
  axis.mat[0] = forward;
  axis.mat[1] = left;
  axis.mat[2] = up;
  axis = axis * inputs->globalAxis;

  const bool flipS =
      stage->systemProperties.textureFlipS == PTEXTURE_FLIP_ALWAYS ||
      (stage->systemProperties.textureFlipS == PTEXTURE_FLIP_RANDOM &&
       (gen->index & 1) != 0);
  const bool flipT =
      stage->systemProperties.textureFlipT == PTEXTURE_FLIP_ALWAYS ||
      (stage->systemProperties.textureFlipT == PTEXTURE_FLIP_RANDOM &&
       (gen->index & 1) != 0);
  for (int index = 0; index < inputs->staticVertsSize; ++index) {
    const idDrawVert& source = inputs->staticVerts[index];
    idTransparencyVert& destination = outputs->verts[index];
    const idVec3 scaled{source.xyz.x * meshScale.x,
                        source.xyz.y * meshScale.y,
                        source.xyz.z * meshScale.z};
    destination.xyz = axis * scaled + origin;
    float textureS = (flipS ? 1.0f - source.st.x : source.st.x) * width + s;
    float textureT = (flipT ? 1.0f - source.st.y : source.st.y) * height + t;
    if ((stage->stageFlags & 4u) != 0) {
      const float halfS = 0.5f /
          (inputs->view->atlasWidth * stage->atlasScaleBias.x);
      const float halfT = 0.5f /
          (inputs->view->atlasHeight * stage->atlasScaleBias.y);
      textureS = std::clamp(textureS, halfS, 1.0f - halfS) *
                     stage->atlasScaleBias.x +
                 stage->atlasScaleBias.z;
      textureT = std::clamp(textureT, halfT, 1.0f - halfT) *
                     stage->atlasScaleBias.y +
                 stage->atlasScaleBias.w;
    }
    destination.st[0] = destination.st1[0] =
        QuantizeParticleTexCoord(textureS);
    destination.st[1] = destination.st1[1] =
        QuantizeParticleTexCoord(textureT);
    std::memcpy(destination.normal, genericParm, 4);
    destination.tangent[0] = ParticleTextureByte(textureS);
    destination.tangent[1] = ParticleTextureByte(textureT);
    destination.tangent[2] = frameBlend;
    destination.tangent[3] = alphaScale;
    for (int channel = 0; channel < 4; ++channel)
      destination.color[channel] = ParticleByte(gen->vertColor[channel]);
  }
  return inputs->staticVertsSize;
}

static int GenerateAimedVerts(const particleInput_t* inputs,
                              particleGen_t* gen,
                              particleOutput_t* outputs,
                              const idVec3& origin,
                              const idVec3& velocity) {
  const idParticleStage* stage = inputs->stage;
  const float size = stage->size.size[0].Compute(
                         inputs->tables, gen->parmVal, gen->random) *
                     inputs->sizeScale;
  const float length = stage->size.aspectRatio.Compute(
                           inputs->tables, gen->parmVal, gen->random) *
                       size;
  idVec3 along = NormalizeParticleVector(velocity);
  idVec3 left = NormalizeParticleVector(along.Cross(inputs->view->viewFwd));
  if (left.LengthSqr() <= std::numeric_limits<float>::min())
    left = inputs->localViewLeft;
  const idVec3 halfWidth = left * size;
  const idVec3 halfLength = along * length;
  ParticleTexCoords(stage, inputs->tables, gen, {0.0f, 1.0f}, {0.0f, 1.0f},
                    inputs->view->atlasWidth, inputs->view->atlasHeight,
                    outputs->verts);
  outputs->verts[0].xyz = origin - halfWidth + halfLength;
  outputs->verts[1].xyz = origin + halfWidth + halfLength;
  outputs->verts[2].xyz = origin - halfWidth - halfLength;
  outputs->verts[3].xyz = origin + halfWidth - halfLength;
  if (outputs->quadDepth != nullptr) {
    *outputs->quadDepth = (origin - inputs->view->viewOrg).x *
                              inputs->view->viewFwd.x +
                          (origin - inputs->view->viewOrg).y *
                              inputs->view->viewFwd.y +
                          (origin - inputs->view->viewOrg).z *
                              inputs->view->viewFwd.z;
  }
  return 4;
}

static int GenerateTrailVerts(const particleInput_t* inputs,
                              particleGen_t* gen,
                              particleOutput_t* outputs,
                              const idVec3& origin) {
  const idParticleStage* stage = inputs->stage;
  const int segments = stage->orientation.numTrails + 1;
  const float segmentTime = stage->orientation.segmentLength != 0.0f
                                ? stage->orientation.segmentLength
                                : 0.5f;
  const particleGen_t saved = *gen;
  idVec3 previousOrigin = origin;
  idVec3 previousLeft{};
  for (int segment = 0; segment < segments; ++segment) {
    *gen = saved;
    gen->cycleAge = saved.cycleAge -
                    (segment + 1) * segmentTime / segments;
    gen->frac = gen->particleLife != 0.0f
                    ? gen->cycleAge / gen->particleLife
                    : 0.0f;
    idVec3 oldOrigin;
    ParticleOrigin(inputs, gen, oldOrigin);
    idVec3 up = NormalizeParticleVector(previousOrigin - oldOrigin);
    idVec3 left = NormalizeParticleVector(up.Cross(inputs->view->viewFwd));
    const float size = stage->size.size[0].Compute(
                           inputs->tables, gen->parmVal, gen->random) *
                       inputs->sizeScale;
    left *= size;
    idTransparencyVert* verts = outputs->verts + segment * 4;
    if (segment != 0)
      left = previousLeft;
    verts[0].xyz = previousOrigin - left;
    verts[1].xyz = previousOrigin + left;
    verts[2].xyz = oldOrigin - left;
    verts[3].xyz = oldOrigin + left;
    ParticleTexCoords(stage, inputs->tables, gen, {0.0f, 1.0f},
                      {static_cast<float>(segment) / segments,
                       1.0f / segments},
                      inputs->view->atlasWidth, inputs->view->atlasHeight,
                      verts);
    std::memcpy(verts[0].color, outputs->verts[0].color, 4);
    std::memcpy(verts[1].color, outputs->verts[0].color, 4);
    std::memcpy(verts[2].color, outputs->verts[0].color, 4);
    std::memcpy(verts[3].color, outputs->verts[0].color, 4);
    if (outputs->quadDepth != nullptr) {
      outputs->quadDepth[segment] =
          (previousOrigin - inputs->view->viewOrg).x *
              inputs->view->viewFwd.x +
          (previousOrigin - inputs->view->viewOrg).y *
              inputs->view->viewFwd.y +
          (previousOrigin - inputs->view->viewOrg).z *
              inputs->view->viewFwd.z;
    }
    previousOrigin = oldOrigin;
    previousLeft = left;
  }
  *gen = saved;
  return segments * 4;
}

static int GenerateQuadVerts(const particleInput_t* inputs,
                             particleGen_t* gen,
                             particleOutput_t* outputs,
                             const idVec3& origin) {
  const idParticleStage* stage = inputs->stage;
  ParticleTexCoords(stage, inputs->tables, gen, {0.0f, 1.0f}, {0.0f, 1.0f},
                    inputs->view->atlasWidth, inputs->view->atlasHeight,
                    outputs->verts);
  const float width = stage->size.size[0].Compute(
                          inputs->tables, gen->parmVal, gen->random) *
                      inputs->sizeScale;
  const float height = stage->size.aspectRatio.Compute(
                           inputs->tables, gen->parmVal, gen->random) *
                       width;
  idVec3 left, up;
  ParticleLeftAndUp(stage, inputs->tables, gen, inputs->localViewUp,
                    inputs->localViewLeft, 0.0f, left, up);
  if (stage->orientation.type == POR_XYZ) {
    left = inputs->globalAxis * left;
    up = inputs->globalAxis * up;
  }
  if (stage->orientation.world) {
    const idMat3 modelStageAxis = inputs->stageAxis * inputs->modelAxis;
    left = modelStageAxis * (inputs->globalAxis * left);
    up = modelStageAxis * (inputs->globalAxis * up);
  } else {
    left = inputs->globalAxis * left;
    up = inputs->globalAxis * up;
  }
  left *= width;
  up *= height;
  const idVec3 pivot = left * stage->pivot.pivotOffset.x +
                       up * stage->pivot.pivotOffset.y;
  outputs->verts[0].xyz = origin - left + up + pivot;
  outputs->verts[1].xyz = origin + left + up + pivot;
  outputs->verts[2].xyz = origin - left - up + pivot;
  outputs->verts[3].xyz = origin + left - up + pivot;
  if (stage->orientation.type != POR_VIEW &&
      stage->orientation.viewFade < 1.0f) {
    const idVec3 viewDirection = NormalizeParticleVector(
        origin + pivot - inputs->localViewOrg);
    const idVec3 normal = NormalizeParticleVector(left.Cross(up));
    const float fade = std::max(
        (1.0f - std::fabs(normal.x * viewDirection.x +
                          normal.y * viewDirection.y +
                          normal.z * viewDirection.z)) -
            stage->orientation.viewFade,
        0.0f) /
        (1.0f - stage->orientation.viewFade);
    for (int vertex = 0; vertex < 4; ++vertex) {
      for (int channel = 0; channel < 4; ++channel) {
        const float faded = (1.0f - fade) * gen->vertColor[channel] +
            inputs->entityColor[channel] *
                stage->colorAttributes.fadeColor[channel] * fade;
        outputs->verts[vertex].color[channel] = ParticleByte(faded);
      }
    }
  }
  if (outputs->quadDepth != nullptr) {
    const idVec3 delta = origin - inputs->view->viewOrg;
    *outputs->quadDepth = delta.x * inputs->view->viewFwd.x +
                          delta.y * inputs->view->viewFwd.y +
                          delta.z * inputs->view->viewFwd.z;
  }
  return 4;
}

static int CreateParticle(const particleInput_t* inputs, particleGen_t* gen,
                          particleOutput_t* outputs) {
  const idParticleStage* stage = inputs->stage;
  idVec3 origin;
  const idVec3 velocity = ParticleOrigin(inputs, gen, origin);
  float fade = 1.0f;
  if (stage->colorAttributes.fadeInFraction > gen->frac)
    fade = gen->frac / stage->colorAttributes.fadeInFraction;
  if (stage->colorAttributes.fadeOutFraction > 1.0f - gen->frac) {
    fade *= (1.0f - gen->frac) /
            stage->colorAttributes.fadeOutFraction;
  }
  if (stage->colorAttributes.fadeIndexFraction != 0.0f) {
    const float indexFraction =
        static_cast<float>(inputs->totalParticles - gen->index) /
        inputs->totalParticles;
    if (stage->colorAttributes.fadeIndexFraction > indexFraction) {
      fade *= indexFraction / stage->colorAttributes.fadeIndexFraction;
    }
  }
  const float brightness = stage->colorAttributes.brightness.Compute(
      inputs->tables, gen->parmVal, gen->random) *
      std::max(stage->colorAttributes.minShadowVal, inputs->shadow);
  const float entityBlend = stage->colorAttributes.entityColorBlendVal;
  const idVec4 entityMultiplier{
      (inputs->entityColor.x - 1.0f) * entityBlend + 1.0f,
      (inputs->entityColor.y - 1.0f) * entityBlend + 1.0f,
      (inputs->entityColor.z - 1.0f) * entityBlend + 1.0f,
      (inputs->entityColor.w - 1.0f) * entityBlend + 1.0f};
  const idVec4 systemFade = (stage->stageFlags & 1u) != 0
                                ? idVec4{1.0f, 1.0f, 1.0f, inputs->fade}
                                : idVec4{inputs->fade, inputs->fade,
                                         inputs->fade, 1.0f};
  for (int channel = 0; channel < 4; ++channel) {
    const float base = stage->colorAttributes.baseColor[channel].Compute(
        inputs->tables, gen->parmVal, gen->random);
    gen->vertColor[channel] =
        (base * fade + stage->colorAttributes.fadeColor[channel] *
                           (1.0f - fade)) *
        entityMultiplier[channel] * systemFade[channel] *
        (channel == 3 ? 1.0f : brightness);
  }
  std::uint8_t color[4];
  bool visible = false;
  for (int channel = 0; channel < 4; ++channel) {
    color[channel] = ParticleByte(gen->vertColor[channel]);
    visible = visible || color[channel] != 0;
  }
  if (!visible)
    return 0;

  std::memset(outputs->verts, 0, sizeof(idTransparencyVert) * 4);
  std::uint8_t genericParm[4];
  for (int channel = 0; channel < 4; ++channel) {
    genericParm[channel] = ParticleByte(
        stage->genericParm.genericParm[channel].Compute(
            inputs->tables, gen->parmVal, gen->random));
  }
  float alphaValue = inputs->alphaScaleOverride;
  if (alphaValue >= 1.0f)
    alphaValue = stage->colorAttributes.softParticleAlphaScale;
  const std::uint8_t alphaScale = ParticleByte(alphaValue);
  for (int vertex = 0; vertex < 4; ++vertex) {
    std::memcpy(outputs->verts[vertex].color, color, 4);
    std::memcpy(outputs->verts[vertex].normal, genericParm, 4);
    outputs->verts[vertex].tangent[3] = alphaScale;
  }
  if (stage->staticData != nullptr) {
    return GenerateStaticMeshVerts(inputs, gen, outputs, origin, alphaScale,
                                   genericParm);
  }
  if (stage->orientation.type == POR_TRAIL)
    return GenerateTrailVerts(inputs, gen, outputs, origin);
  if (stage->orientation.type == POR_AIMED)
    return GenerateAimedVerts(inputs, gen, outputs, origin, velocity);
  return GenerateQuadVerts(inputs, gen, outputs, origin);
}

namespace {

idMat3 IdentityParticleAxis() {
  idMat3 axis{};
  axis.mat[0].x = 1.0f;
  axis.mat[1].y = 1.0f;
  axis.mat[2].z = 1.0f;
  return axis;
}

void CopyGeneratedVerts(const deferredParticleGenParms_t* parms,
                        int destinationOffset,
                        const idTransparencyVert* source, int count) {
  if (count > 0) {
    std::memcpy(parms->verts + destinationOffset, source,
                sizeof(idTransparencyVert) * static_cast<std::size_t>(count));
  }
}

int ParticleVertsPerInstance(const deferredParticleGenParms_t* parms) {
  if (parms->stage->staticData != nullptr && parms->staticVerts != nullptr)
    return parms->staticVertsSize;
  return 4 * (parms->stage->orientation.type == POR_TRAIL
                  ? parms->stage->orientation.numTrails + 1
                  : 1);
}

void AverageGeneratedDepths(const deferredParticleGenParms_t* parms,
                            int firstVertex, int generatedVertices) {
  if (parms->quadDepth == nullptr || generatedVertices <= 0)
    return;
  const int firstQuad = firstVertex / 4;
  const int quadCount = generatedVertices / 4;
  float total = 0.0f;
  for (int index = 0; index < quadCount; ++index)
    total += parms->quadDepth[firstQuad + index];
  const float average = total / quadCount;
  for (int index = 0; index < quadCount; ++index)
    parms->quadDepth[firstQuad + index] = average;
}

} // namespace

int GenParticleStage(const deferredParticleGenParms_t* parms, int index,
                     int vertOffset, const prtLocation_t* location) {
  const idParticleStage* stage = parms->stage;
  if (stage == nullptr || index < 0 ||
      index >= parms->numModelParticleParms ||
      stage->systemProperties.material == nullptr || stage->cycleMsec == 0 ||
      (stage->stageFlags & 0x10u) != 0) {
    return 0;
  }
  const modelParticleParms_t& model = parms->modelParticleParms[index];
  particleInput_t inputs{};
  inputs.stage = stage;
  inputs.location = location;
  inputs.tables = parms->tables;
  inputs.view = parms->renderView;
  inputs.staticVerts = parms->staticVerts;
  inputs.meshDistribution = &parms->meshDistribution;
  inputs.staticVertsSize = parms->staticVertsSize;
  inputs.modelAxis = model.axis;
  inputs.stageAxis = ParticleStageAxis(
      stage, (stage->systemProperties.diversity + location->diversity +
              model.diversity) & 0x7fff);
  const float lodFraction = std::clamp(
      static_cast<float>(model.lod) * stage->lodParms.lerpAmount, 0.0f, 1.0f);
  inputs.totalParticles = static_cast<int>(
      stage->systemProperties.totalParticles +
      (stage->lodParms.totalParticles - stage->systemProperties.totalParticles) *
          lodFraction);
  inputs.globalOrigin = (stage->stageFlags & 2u) != 0 ? model.origin : idVec3{};
  inputs.globalAxis = IdentityParticleAxis();
  if ((stage->stageFlags & 2u) != 0) {
    inputs.globalAxis.mat[0] = model.axis.mat[0] * model.scale.x;
    inputs.globalAxis.mat[1] = model.axis.mat[1] * model.scale.y;
    inputs.globalAxis.mat[2] = model.axis.mat[2] * model.scale.z;
    inputs.localViewOrg = parms->renderView->viewOrg;
  } else {
    const idVec3 viewDelta = parms->renderView->viewOrg - model.origin;
    inputs.localViewOrg = {viewDelta.x * model.axis.mat[0].x +
                               viewDelta.y * model.axis.mat[0].y +
                               viewDelta.z * model.axis.mat[0].z,
                           viewDelta.x * model.axis.mat[1].x +
                               viewDelta.y * model.axis.mat[1].y +
                               viewDelta.z * model.axis.mat[1].z,
                           viewDelta.x * model.axis.mat[2].x +
                               viewDelta.y * model.axis.mat[2].y +
                               viewDelta.z * model.axis.mat[2].z};
  }
  inputs.localVelocity = NormalizeParticleVector(model.velocity);
  inputs.localViewLeft = model.axis * parms->renderView->viewLeft;
  inputs.localViewUp = model.axis * parms->renderView->viewUp;
  inputs.fade = model.coverage;
  inputs.alphaScaleOverride = model.alphaScaleOverride;
  inputs.shadow = stage->colorAttributes.useGlobalShadows ? model.shadow : 1.0f;
  inputs.distribScale = model.distribScale;
  inputs.sizeScale = (stage->lodParms.sizeScale - 1.0f) * lodFraction + 1.0f;
  inputs.entityColor = model.color;
  const idVec3 windDirection = NormalizeParticleVector(
      {model.wind.x, model.wind.y, model.wind.z});
  inputs.wind = model.axis * windDirection * model.wind.w;
  inputs.diversity = (stage->systemProperties.diversity +
                      location->diversity + model.diversity) & 0x7fff;

  const int cycleMsec = static_cast<int>(
      (stage->maxParticleLife + parms->deadTime) * 1000.0f);
  if (cycleMsec <= 0 || inputs.totalParticles <= 0)
    return 0;
  const int currentMsec = static_cast<int>(
      (parms->renderView->renderTime * 0.001f -
       (stage->systemProperties.timeOffset + model.timeOffset)) * 1000.0f);
  const int spawnSpan = static_cast<int>(
      stage->systemProperties.spawnBunching * stage->bunchTime * 1000.0f);
  const int startOffset = vertOffset;
  const int requiredVerts = ParticleVertsPerInstance(parms);
  std::uint32_t baseSeed = static_cast<std::uint32_t>(inputs.diversity);
  for (int order = 0; order < inputs.totalParticles; ++order) {
    int particleIndex = order;
    if (stage->systemProperties.sortType == 1)
      particleIndex = inputs.totalParticles - order - 1;
    idRandom2 random{baseSeed};
    for (int step = 0; step <= particleIndex; ++step)
      NextParticleRandom(random);
    const int ageMsec = currentMsec -
        spawnSpan * particleIndex / inputs.totalParticles;
    if (ageMsec < 0)
      continue;
    const int cycle = ageMsec / cycleMsec;
    if (stage->systemProperties.cycles != 0 &&
        cycle >= stage->systemProperties.cycles) {
      continue;
    }
    const int cycleAgeMsec = ageMsec % cycleMsec;
    if (model.stopTime != 0 &&
        parms->renderView->renderTime - cycleAgeMsec >= model.stopTime) {
      continue;
    }
    const float life = stage->systemProperties.particleLife.Compute(
        parms->tables,
        static_cast<float>(particleIndex) / inputs.totalParticles, random);
    if (cycleAgeMsec > static_cast<int>(life * 1000.0f))
      continue;
    if (vertOffset + requiredVerts > parms->maxVertsToGen)
      break;
    particleGen_t gen{};
    gen.index = particleIndex;
    gen.random = random;
    gen.originalRandom = random;
    gen.particleLife = life;
    gen.totalAge = ageMsec * 0.001f;
    gen.cycleAge = cycleAgeMsec * 0.001f;
    gen.frac = life != 0.0f ? gen.cycleAge / life : 0.0f;
    gen.parmVal = stage->systemProperties.useSysTime != 0
                      ? stage->systemProperties.useSysTime *
                            (parms->renderView->renderTime * 0.001f)
                      : gen.frac;
    gen.vertColor = {1.0f, 1.0f, 1.0f, 1.0f};
    alignas(16) idTransparencyVert localVerts[128]{};
    particleOutput_t output{localVerts,
        parms->quadDepth != nullptr ? &parms->quadDepth[vertOffset / 4]
                                    : nullptr};
    const int generated = CreateParticle(&inputs, &gen, &output);
    CopyGeneratedVerts(parms, vertOffset, localVerts, generated);
    vertOffset += generated;
    baseSeed = random.seed;
  }
  if (stage->systemProperties.sortType == 1 ||
      stage->systemProperties.sortType == 2) {
    AverageGeneratedDepths(parms, startOffset, vertOffset - startOffset);
  }
  return vertOffset - startOffset;
}

int GenEffectStage(const deferredParticleGenParms_t* parms, int index,
                   int vertOffset, const prtLocation_t* location) {
  const idParticleStage* stage = parms->stage;
  if (stage == nullptr || index < 0 ||
      index >= parms->numEffectParticleParms)
    return 0;
  const effectParticleParms_t& effect = parms->effectParticleParms[index];
  particleInput_t inputs{};
  inputs.stage = stage;
  inputs.location = location;
  inputs.tables = parms->tables;
  inputs.view = parms->renderView;
  inputs.staticVerts = parms->staticVerts;
  inputs.meshDistribution = &parms->meshDistribution;
  inputs.staticVertsSize = parms->staticVertsSize;
  inputs.totalParticles = stage->systemProperties.totalParticles;
  inputs.modelAxis = effect.axis;
  inputs.globalAxis = effect.axis;
  inputs.stageAxis = ParticleStageAxis(
      stage, (stage->systemProperties.diversity + location->diversity +
              effect.diversity) & 0x7fff);
  inputs.localVelocity = NormalizeParticleVector(effect.axis * effect.velocity);
  inputs.localViewLeft = effect.axis * parms->renderView->viewLeft;
  inputs.localViewUp = effect.axis * parms->renderView->viewUp;
  inputs.localViewOrg = parms->renderView->viewOrg;
  inputs.fade = 1.0f;
  inputs.alphaScaleOverride = 1.0f;
  inputs.shadow = effect.shadow;
  inputs.distribScale = {1.0f, 1.0f, 1.0f};
  inputs.sizeScale = 1.0f;
  inputs.entityColor = {
      static_cast<float>(effect.color & 0xffu) / 255.0f,
      static_cast<float>((effect.color >> 8) & 0xffu) / 255.0f,
      static_cast<float>((effect.color >> 16) & 0xffu) / 255.0f, 1.0f};
  inputs.wind = effect.wind;
  inputs.diversity = (stage->systemProperties.diversity +
                      location->diversity + effect.diversity) & 0x7fff;

  const int startOffset = vertOffset;
  const int requiredVerts = ParticleVertsPerInstance(parms);
  const int spawnSpan = static_cast<int>(
      stage->systemProperties.spawnBunching * stage->bunchTime * 1000.0f);
  std::uint32_t seed = static_cast<std::uint32_t>(inputs.diversity);
  for (int order = 0; order < effect.numParticles; ++order) {
    const int particleIndex = stage->systemProperties.sortType == 1
                                  ? effect.numParticles - order - 1
                                  : order;
    idRandom2 seedState{seed};
    NextParticleRandom(seedState);
    seed = seedState.seed;
    idRandom2 random{seed};
    const int spawnTime = effect.startTime +
        spawnSpan * particleIndex /
            std::max<int>(stage->systemProperties.totalParticles, 1);
    const int ageMsec = parms->renderView->renderTime - spawnTime;
    const float life = stage->systemProperties.particleLife.Compute(
        parms->tables,
        static_cast<float>(particleIndex) /
            std::max(effect.numParticles, 1),
        random);
    if (ageMsec < 0 || ageMsec >= static_cast<int>(life * 1000.0f))
      continue;
    if (vertOffset + requiredVerts > parms->maxVertsToGen)
      break;
    const float movementTime =
        (parms->renderView->renderTime - parms->renderView->deltaTime -
         effect.currTime) * 0.001f;
    inputs.globalOrigin = effect.origin + effect.velocity * movementTime;
    particleGen_t gen{};
    gen.index = particleIndex;
    gen.random = random;
    gen.originalRandom = random;
    gen.particleLife = life;
    gen.totalAge = ageMsec * 0.001f;
    gen.cycleAge = gen.totalAge;
    gen.frac = life != 0.0f ? gen.totalAge / life : 0.0f;
    gen.parmVal = gen.frac;
    gen.vertColor = {1.0f, 1.0f, 1.0f, 1.0f};
    alignas(16) idTransparencyVert localVerts[128]{};
    particleOutput_t output{localVerts,
        parms->quadDepth != nullptr ? &parms->quadDepth[vertOffset / 4]
                                    : nullptr};
    const int generated = CreateParticle(&inputs, &gen, &output);
    CopyGeneratedVerts(parms, vertOffset, localVerts, generated);
    vertOffset += generated;
  }
  if (stage->systemProperties.sortType == 1)
    AverageGeneratedDepths(parms, startOffset, vertOffset - startOffset);
  return vertOffset - startOffset;
}

void ParticleGenJob(const deferredParticleGenParms_t* allParms) {
  int accumulatedVerts = 0;
  for (int jobIndex = 0; jobIndex < allParms->numJobs; ++jobIndex) {
    const deferredParticleGenParms_t& parms = allParms[jobIndex];
    if (parms.quadsUsed != nullptr)
      *parms.quadsUsed = 0;
    if (parms.maxVertsToGen <= 0)
      continue;
    if (!allParms->mergedSurfaces)
      accumulatedVerts = 0;
    prtLocation_t empty{};
    empty.Clear();
    for (idParticleParm& parm : empty.loc) {
      parm.tableIdx = -1;
      parm.table2Idx = -1;
    }
    const int locationPasses = parms.numLocations + 1;
    for (int locationIndex = 0; locationIndex < locationPasses;
         ++locationIndex) {
      const prtLocation_t* location =
          locationIndex == 0 ? &empty : &parms.location[locationIndex - 1];
      for (int index = 0; index < parms.numModelParticleParms; ++index) {
        accumulatedVerts += GenParticleStage(
            &parms, index, accumulatedVerts, location);
      }
      for (int index = 0; index < parms.numEffectParticleParms; ++index) {
        accumulatedVerts += GenEffectStage(
            &parms, index, accumulatedVerts, location);
      }
    }
    if (!allParms->mergedSurfaces || jobIndex == allParms->numJobs - 1) {
      for (int index = accumulatedVerts; index < parms.maxVertsToGen; ++index)
        parms.verts[index] = {};
      if (parms.quadDepth != nullptr) {
        for (int index = accumulatedVerts / 4;
             index < parms.maxVertsToGen / 4; ++index) {
          parms.quadDepth[index] = 0.0f;
        }
      }
    }
    if (parms.quadsUsed != nullptr)
      *parms.quadsUsed = accumulatedVerts / 4;
  }
  if (allParms->quadsUsed != nullptr)
    *allParms->quadsUsed = accumulatedVerts / 4;
}

idParallelJobRegistration register_ParticleGenJob(
    reinterpret_cast<jobRun_t>(ParticleGenJob), "ParticleGenJob");

particleMeshDist_t::~particleMeshDist_t() {
  if (isCopy == 0)
    delete[] triangles;
  ConstructEmpty();
}
