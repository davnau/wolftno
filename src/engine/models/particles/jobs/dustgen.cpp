#include "engine/models/particles/jobs/dustgen.h"

#include "shared/idlib/parallelism/paralleljoblist.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace {

constexpr float kRandomScale = 0.000030518509f;
constexpr float kDegreesToRadians = 0.017453292f;

std::uint32_t NextRandom(idRandom2& random) {
  random.seed = 1664525u * random.seed + 1013904223u;
  return random.seed;
}

float UnitRandom(idRandom2& random) {
  return static_cast<float>((NextRandom(random) >> 10) & 0x7fffu) *
         kRandomScale;
}

float SignedRandom(idRandom2& random) {
  return UnitRandom(random) * 2.0f - 1.0f;
}

float SafeLength(const idVec3& value) {
  return std::sqrt(std::max(value.LengthSqr(),
                            std::numeric_limits<float>::min()));
}

std::uint8_t ToByte(float value) {
  return static_cast<std::uint8_t>(
      std::clamp(static_cast<int>(value * 255.0f), 0, 255));
}

std::uint8_t ToTextureByte(float value) {
  value = std::clamp(value, 0.0f, 1.0f);
  return static_cast<std::uint8_t>(
      std::clamp(static_cast<int>(value * 255.5f), 0, 255));
}

std::int16_t QuantizeTexCoord(float value) {
  constexpr float zero = 0.5f;
  constexpr float scale = 0.5f / 32767.0f;
  return static_cast<std::int16_t>(std::clamp(
      static_cast<int>(std::floor((value - zero) / scale + 0.5f)),
      -32768, 32767));
}

void SinCos(float angle, float& sine, float& cosine) {
  sine = std::sin(angle);
  cosine = std::cos(angle);
}

void ParticleLeftAndUpForDust(const idParticleStage& stage,
                              const idLookupTable* tables,
                              particleGen_t& gen, const idVec3& viewUp,
                              const idVec3& viewLeft, float extraRotation,
                              idVec3& left, idVec3& up) {
  if (stage.orientation.type == POR_XYZ) {
    const float initialX = stage.initialRotation.initialAngle[0].Compute(
        tables, gen.parmVal, gen.random);
    const float initialY = stage.initialRotation.initialAngle[1].Compute(
        tables, gen.parmVal, gen.random);
    const float initialZ = stage.initialRotation.initialAngle[2].Compute(
        tables, gen.parmVal, gen.random);
    const float rotateX = stage.rotation.rotation[0].Compute(
        tables, gen.parmVal, gen.random);
    const float rotateY = stage.rotation.rotation[1].Compute(
        tables, gen.parmVal, gen.random);
    const float rotateZ = stage.rotation.rotation[2].Compute(
        tables, gen.parmVal, gen.random);
    float sx, cx, sy, cy, sz, cz;
    SinCos((initialX + rotateX * gen.particleLife) * kDegreesToRadians,
           sx, cx);
    SinCos((initialY + rotateY * gen.particleLife) * kDegreesToRadians,
           sy, cy);
    SinCos((initialZ + rotateZ * gen.particleLife) * kDegreesToRadians,
           sz, cz);
    left = {cz * sy, sz * sy, -cy};
    up = {cy * sx * sz - cz * cx,
          cy * sx * cz + sz * cx,
          sy * sx};
    return;
  }

  const float initial = stage.initialRotation.initialAngle[0].Compute(
      tables, gen.parmVal, gen.random);
  const float rotation = stage.rotation.rotation[0].Compute(
      tables, gen.parmVal, gen.random) * gen.particleLife;
  const float angle = initial +
      (((gen.index & 1) != 0 && stage.rotation.allowRotDirOverride)
           ? -rotation : rotation) + extraRotation;
  float sine, cosine;
  SinCos(angle * kDegreesToRadians, sine, cosine);
  switch (stage.orientation.type) {
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

void ComputeTextureAnimation(const idParticleStage& stage,
                             const idLookupTable* tables,
                             particleGen_t& gen, float& width, float& s,
                             float& height, float& t,
                             std::uint8_t& frameBlend, bool& wrap) {
  const int animationRows = stage.texAnimation.useRandomRow
                                ? 1 : stage.texAnimation.numRows;
  const int frameCount = animationRows * stage.texAnimation.numColumns;
  int startFrame = stage.texAnimation.startFrame;
  if (startFrame < 0)
    startFrame = frameCount > 1
                     ? static_cast<int>(((NextRandom(gen.random) >> 10) &
                                         0x7fffu) % frameCount)
                     : 0;

  float frame = 0.0f;
  if (stage.texAnimation.type == 0) {
    const float rate = stage.texAnimation.rate.Compute(
        tables, gen.parmVal, gen.random);
    frame = (stage.texAnimation.rate.calcType == PARTICLE_CALC_GENERIC
                 ? gen.totalAge : 1.0f) * rate + startFrame;
  } else if (stage.texAnimation.type == 1) {
    const float rate = stage.texAnimation.rate.Compute(
        tables, gen.parmVal, gen.random);
    frame = std::min((stage.texAnimation.rate.calcType ==
                              PARTICLE_CALC_GENERIC ? gen.frac : 1.0f) * rate +
                         startFrame,
                     static_cast<float>(frameCount) - 1.0f);
  } else if (stage.texAnimation.type == 2) {
    frame = static_cast<float>(frameCount) * gen.frac + startFrame;
  }

  const int integerFrame = static_cast<int>(frame);
  const float blend = frame - integerFrame;
  int row;
  if (stage.texAnimation.useRandomRow) {
    row = stage.texAnimation.numRows > 1
              ? static_cast<int>(((NextRandom(gen.random) >> 10) & 0x7fffu) %
                                 stage.texAnimation.numRows)
              : 0;
  } else {
    row = integerFrame % frameCount / stage.texAnimation.numColumns;
  }
  const int column = integerFrame % stage.texAnimation.numColumns;
  width /= stage.texAnimation.numColumns;
  s += column * width - std::floor(column * width);
  height /= stage.texAnimation.numRows;
  t += row * height - std::floor(row * height);
  if (stage.texAnimation.useFrameBlending) {
    frameBlend = ToByte(blend);
    wrap = stage.texAnimation.numRows > 1 &&
           !stage.texAnimation.useRandomRow &&
           column == stage.texAnimation.numColumns - 1;
  }
}

void ParticleTexCoordsForDust(const idParticleStage& stage,
                              const idLookupTable* tables,
                              particleGen_t& gen, const idVec2& sMinMax,
                              const idVec2& tMinMax, int atlasWidth,
                              int atlasHeight, idTransparencyVert* verts) {
  float s = sMinMax.x;
  float width = sMinMax.y;
  float t = tMinMax.x;
  float height = tMinMax.y;
  std::uint8_t frameBlend = 0;
  bool wrap = false;
  if (stage.texAnimation.numColumns > 1 || stage.texAnimation.numRows > 1)
    ComputeTextureAnimation(stage, tables, gen, width, s, height, t,
                            frameBlend, wrap);

  const bool flipS = stage.systemProperties.textureFlipS ==
                         PTEXTURE_FLIP_ALWAYS ||
                     (stage.systemProperties.textureFlipS ==
                          PTEXTURE_FLIP_RANDOM && (gen.index & 1) != 0);
  const bool flipT = stage.systemProperties.textureFlipT ==
                         PTEXTURE_FLIP_ALWAYS ||
                     (stage.systemProperties.textureFlipT ==
                          PTEXTURE_FLIP_RANDOM && (gen.index & 1) != 0);
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

  if ((stage.stageFlags & 4u) != 0) {
    const float halfS = 0.5f / (atlasWidth * stage.atlasScaleBias.x);
    const float halfT = 0.5f / (atlasHeight * stage.atlasScaleBias.y);
    for (int i = 0; i < 2; ++i) {
      textureS0[i] = std::clamp(textureS0[i], halfS, 1.0f - halfS) *
                         stage.atlasScaleBias.x + stage.atlasScaleBias.z;
      textureS1[i] = std::clamp(textureS1[i], halfS, 1.0f - halfS) *
                         stage.atlasScaleBias.x + stage.atlasScaleBias.z;
      textureT0[i] = std::clamp(textureT0[i], halfT, 1.0f - halfT) *
                         stage.atlasScaleBias.y + stage.atlasScaleBias.w;
      textureT1[i] = std::clamp(textureT1[i], halfT, 1.0f - halfT) *
                         stage.atlasScaleBias.y + stage.atlasScaleBias.w;
    }
  }

  const std::int16_t stS[2] = {QuantizeTexCoord(textureS0.x),
                               QuantizeTexCoord(textureS0.y)};
  const std::int16_t stT[2] = {QuantizeTexCoord(textureT0.x),
                               QuantizeTexCoord(textureT0.y)};
  const std::int16_t st1S[2] = {QuantizeTexCoord(textureS1.x),
                                QuantizeTexCoord(textureS1.y)};
  const std::int16_t st1T[2] = {QuantizeTexCoord(textureT1.x),
                                QuantizeTexCoord(textureT1.y)};
  const std::uint8_t unscaledS[2] = {ToTextureByte(sMinMax.x),
                                    ToTextureByte(sMinMax.y)};
  const std::uint8_t unscaledT[2] = {ToTextureByte(tMinMax.x),
                                    ToTextureByte(tMinMax.y)};
  constexpr int side[4] = {0, 1, 0, 1};
  constexpr int vertical[4] = {0, 0, 1, 1};
  for (int i = 0; i < 4; ++i) {
    verts[i].st[0] = stS[side[i]];
    verts[i].st[1] = stT[vertical[i]];
    verts[i].st1[0] = st1S[side[i]];
    verts[i].st1[1] = st1T[vertical[i]];
    verts[i].tangent[0] = unscaledS[side[i]];
    verts[i].tangent[1] = unscaledT[vertical[i]];
    verts[i].tangent[2] = frameBlend;
  }
}

} // namespace

float idParticleParm::EvalGeneric(float min, float max,
                                  idRandom2& rnd) const {
  const float value = UnitRandom(rnd) * (max - min) + min;
  return SignedRandom(rnd) * (value * variance) + value;
}

float idParticleParm::IntegrateMinMax(float min, float max, idRandom2& rnd,
                                      float frac) const {
  const float sampled = UnitRandom(rnd) * (max - min) + min;
  const float value = (0.5f * (sampled - sampled) * frac + sampled) * frac;
  return SignedRandom(rnd) * (value * variance) + value;
}

float idParticleParm::EvalTableWithVariance(const idLookupTable* tables,
                                            int idx, float frac,
                                            idRandom2& rnd) const {
  if (idx < 0 || tables == nullptr)
    return 0.0f;
  const float value = const_cast<idLookupTable&>(tables[idx]).TableLookup(
      frac, false);
  return SignedRandom(rnd) * (value * variance) + value;
}

float idParticleParm::Compute(const idLookupTable* tables, float frac,
                              idRandom2& random) const {
  switch (calcType) {
    case PARTICLE_CALC_CURVE_GENERIC:
      return EvalTableWithVariance(tables, tableIdx, frac, random) * val0 +
             val1;
    case PARTICLE_CALC_GENERIC:
      return EvalGeneric(val0, val1, random);
    case PARTICLE_CALC_CURVE_MOD_CURVE: {
      const float first = tableIdx >= 0 && tables != nullptr
          ? const_cast<idLookupTable&>(tables[tableIdx]).TableLookup(frac,
                                                                    false)
          : 0.0f;
      const float second = table2Idx >= 0 && tables != nullptr
          ? const_cast<idLookupTable&>(tables[table2Idx]).TableLookup(frac,
                                                                     false)
          : 0.0f;
      return first * second;
    }
    case PARTICLE_CALC_CURVE_ADD_CURVE: {
      const float first = tableIdx >= 0 && tables != nullptr
          ? const_cast<idLookupTable&>(tables[tableIdx]).TableLookup(frac,
                                                                    false)
          : 0.0f;
      const float second = table2Idx >= 0 && tables != nullptr
          ? const_cast<idLookupTable&>(tables[table2Idx]).TableLookup(frac,
                                                                     false)
          : 0.0f;
      return first + second;
    }
    case PARTICLE_CALC_PARAMETRIC_EVAL: {
      const float value = (val1 - val0) * frac + val0;
      return SignedRandom(random) * (value * variance) + value;
    }
    case PARTICLE_CALC_PARAMETRIC_INTEGRATE: {
      const float value = ((val1 - val0) * frac * 0.5f + val0) * frac;
      return SignedRandom(random) * (value * variance) + value;
    }
    case PARTICLE_CALC_PARAMETRIC_INTEGRATE_MINMAX:
      return IntegrateMinMax(val0, val1, random, frac);
    default:
      return 0.0f;
  }
}

void DustGenJob(const deferredDustGenParms_t* parms) {
  float sine, cosine;
  SinCos(-parms->viewUp.z * parms->parms.particleViewTilt, sine, cosine);
  const idVec3 adjustedViewUp = parms->viewUp * cosine -
                                parms->viewFwd * sine;
  particleGen_t gen{};
  for (int index = 0; index < parms->numParticles; ++index) {
    dustParticle_t& particle = parms->particles[index];
    if (particle.state != 0 || particle.fade > 0.5f) {
      idVec3 force{0.0f, 0.0f, -parms->parms.gravity};
      const int sphereCount = parms->visibleInfluenceSpheres->numSpheres;
      for (int sphereIndex = 0; sphereIndex < sphereCount; ++sphereIndex) {
        const influenceSphere_t& sphere =
            parms->visibleInfluenceSpheres->spheres[sphereIndex];
        const idVec3 delta = sphere.center - particle.origin;
        const float distance = SafeLength(delta);
        if (sphere.outerRadius > distance) {
          if (SafeLength(sphere.directionalForce) > 0.0f) {
            force += sphere.directionalForce * particle.influenceForce;
          } else {
            force += delta * ((1.0f / distance) *
                              ((1.0f - sphere.outerRadius) *
                               particle.influenceForce));
          }
        }
      }
      particle.velocity += force * parms->timeDelta;
      particle.velocity *= 1.0f - parms->parms.dampning;
    }

    const float speed = SafeLength(particle.velocity);
    if (speed > particle.maxVelocity)
      particle.velocity *= particle.maxVelocity / speed;

    const idVec3 oldOrigin = particle.origin;
    particle.origin += particle.velocity * parms->timeDelta;
    if (particle.initial.z > particle.origin.z) {
      particle.origin.z = particle.initial.z;
      particle.velocity.z = 0.0f;
    }

    const float movement = SafeLength(particle.origin - oldOrigin);
    const float viewDistance = SafeLength(parms->viewOrg - particle.origin);
    const float fadeStep = parms->timeDelta / parms->parms.fadeouttime;
    const float effectiveMovement = std::max(movement - 0.001f, 0.0f);
    particle.energy += effectiveMovement / particle.maxVelocity * fadeStep;
    particle.rotation += effectiveMovement *
                             parms->parms.energyRotationSpeed *
                             particle.rotationDirection +
                         parms->timeDelta * particle.rotationSpeed;
    if (particle.state == 1) {
      particle.fade -= fadeStep;
      if (particle.fade < 0.0f) {
        particle.state = 0;
        particle.fade = 0.0f;
        particle.energy = 0.0f;
        particle.rotation = 0.0f;
        particle.origin = particle.initial;
        particle.velocity = {0.0f, 0.0f, 0.0f};
      }
    } else {
      const idVec2 planarDelta{particle.origin.x - particle.initial.x,
                               particle.origin.y - particle.initial.y};
      if (particle.fade > 0.5f &&
          planarDelta.LengthSqr() >
              parms->parms.distanceBeforeFadeout *
                  parms->parms.distanceBeforeFadeout)
        particle.state = 1;
      particle.fade += parms->timeDelta / parms->parms.fadeintime;
      particle.fade = std::min(particle.fade, 1.0f);
    }

    particle.lifeTime = std::fmod(
        std::min(parms->parms.movementAnimation * movement +
                     parms->timeDelta * 0.05f,
                 parms->timeDelta * parms->parms.maxAnimationSpeed) +
            particle.lifeTime,
        1.0f);
    const float parmFraction = particle.state != 0
                                   ? (1.0f - particle.fade) + particle.lifeTime
                                   : particle.lifeTime;
    gen.index = index;
    gen.cycleAge = parmFraction;
    gen.totalAge = parmFraction;
    gen.particleLife = 1.0f;
    gen.frac = parmFraction;
    gen.parmVal = parmFraction;

    gen.vertColor.x = parms->stage->colorAttributes.baseColor[0].Compute(
        parms->tables, parmFraction, gen.random);
    gen.vertColor.y = parms->stage->colorAttributes.baseColor[1].Compute(
        parms->tables, parmFraction, gen.random);
    gen.vertColor.z = parms->stage->colorAttributes.baseColor[2].Compute(
        parms->tables, parmFraction, gen.random);
    gen.vertColor.w = parms->stage->colorAttributes.baseColor[3].Compute(
        parms->tables, parmFraction, gen.random) * particle.fade *
                      particle.baseAlpha;

    idVec3 left, up;
    ParticleLeftAndUpForDust(*parms->stage, parms->tables, gen,
                             adjustedViewUp, parms->viewLeft,
                             particle.rotation, left, up);
    const float heightScale =
        (std::min(std::max(std::fabs(parms->viewOrg.z - particle.origin.z) -
                               parms->parms.heightScaleOffset,
                           0.0f),
                  parms->parms.heightScaleRange) /
             parms->parms.heightScaleRange *
             parms->parms.heightScaleAmount) +
        1.0f;
    const float size = parms->stage->size.size[0].Compute(
                           parms->tables, parmFraction, gen.random) *
                       (particle.energySizeIncrase * particle.energy + 1.0f) *
                       heightScale;
    const float aspect = parms->stage->size.aspectRatio.Compute(
                             parms->tables, parmFraction, gen.random) * size;
    const idVec3 scaledUp = up * size;
    const idVec3 scaledLeft = left * aspect;
    idTransparencyVert* verts = parms->verts + index * 4;
    verts[0].xyz = particle.origin + scaledUp - scaledLeft;
    verts[1].xyz = particle.origin + scaledUp + scaledLeft;
    verts[2].xyz = particle.origin - scaledUp - scaledLeft;
    verts[3].xyz = particle.origin - scaledUp + scaledLeft;

    std::uint8_t generic[4];
    for (int channel = 0; channel < 4; ++channel)
      generic[channel] = ToByte(
          parms->stage->genericParm.genericParm[channel].Compute(
              parms->tables, parmFraction, gen.random));
    const std::uint8_t color[4] = {ToByte(gen.vertColor.x),
                                   ToByte(gen.vertColor.y),
                                   ToByte(gen.vertColor.z),
                                   ToByte(gen.vertColor.w)};
    const std::uint8_t softAlpha = ToByte(
        parms->stage->colorAttributes.softParticleAlphaScale);
    for (int corner = 0; corner < 4; ++corner) {
      std::memcpy(verts[corner].normal, generic, sizeof(generic));
      verts[corner].tangent[3] = softAlpha;
      std::memcpy(verts[corner].color, color, sizeof(color));
    }

    parms->depths[index] = viewDistance;
    ParticleTexCoordsForDust(*parms->stage, parms->tables, gen,
                             {0.0f, 1.0f}, {0.0f, 1.0f},
                             static_cast<int>(parms->transSortWidth),
                             static_cast<int>(parms->transSortHeight), verts);
  }
}

idParallelJobRegistration register_DustGenJob(
    reinterpret_cast<jobRun_t>(DustGenJob), "DustGenJob");

