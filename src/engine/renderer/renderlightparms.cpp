#include "engine/renderer/renderlightparms.h"

#include "engine/renderer/renderlocal.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/math/mathlib.h"

#include <algorithm>
#include <cmath>
#include <cstring>

const idMaterial* idRenderLightParms::DefaultMaterial() const {
  if (lightClass == LIGHT_BLENDED_ONLY) {
    if (lightType == LIGHT_POINT)
      return tr.mtrDefaultPointBlendLight;
    if (lightType == LIGHT_PARALLEL)
      return tr.mtrDefaultParallelBlendLight;
    return tr.mtrDefaultProjectedBlendLight;
  }

  if (lightClass == LIGHT_SLOWLIGHT_DYNAMIC_ONLY ||
      lightClass == LIGHT_SLOWLIGHT_PRIVATE_ONLY) {
    if (lightType == LIGHT_POINT)
      return tr.mtrDefaultPointSlowLight;
    if (lightType == LIGHT_PARALLEL)
      return tr.mtrDefaultParallelSlowLight;
    return tr.mtrDefaultProjectedSlowLight;
  }

  if (lightType == LIGHT_POINT)
    return tr.mtrDefaultPointLight;
  if (lightType == LIGHT_PARALLEL)
    return tr.mtrDefaultParallelLight;
  return tr.mtrDefaultProjectedLight;
}

void idRenderLightParms::ComputeSpotLightProjectionMatrix(
    idRenderMatrix& localProject) const {
  const float targetLengthSqr = target.x * target.x + target.y * target.y +
                                target.z * target.z;
  const float inverseTargetLength =
      1.0f / std::sqrt(std::max(targetLengthSqr, idMath::FLT_SMALLEST_NON_DENORMAL));
  const idVec3 targetNormal = target * inverseTargetLength;

  const float targetLength = inverseTargetLength * targetLengthSqr;
  const float rightScale =
      (targetLength * 0.5f) /
      (right.x * right.x + right.y * right.y + right.z * right.z);
  const float upScale =
      (targetLength * -0.5f) /
      (up.x * up.x + up.y * up.y + up.z * up.z);

  localProject.m[0] = right.x * rightScale;
  localProject.m[1] = right.y * rightScale;
  localProject.m[2] = right.z * rightScale;
  localProject.m[3] = 0.0f;
  localProject.m[4] = up.x * upScale;
  localProject.m[5] = up.y * upScale;
  localProject.m[6] = up.z * upScale;
  localProject.m[7] = 0.0f;
  localProject.m[12] = targetNormal.x;
  localProject.m[13] = targetNormal.y;
  localProject.m[14] = targetNormal.z;
  localProject.m[15] = 0.0f;

  const float startDistance =
      std::max(targetNormal.x * start.x + targetNormal.y * start.y +
                   targetNormal.z * start.z,
               8.0f);
  const float endDistance =
      std::max(targetNormal.x * end.x + targetNormal.y * end.y +
                   targetNormal.z * end.z,
               16.0f);
  const float depthScale = (endDistance + startDistance) / endDistance;
  localProject.m[8] = targetNormal.x * depthScale;
  localProject.m[9] = targetNormal.y * depthScale;
  localProject.m[10] = targetNormal.z * depthScale;
  localProject.m[11] = -(depthScale * startDistance);

  const float targetW = target.x * localProject.m[12] +
                        target.y * localProject.m[13] +
                        target.z * localProject.m[14] + localProject.m[15];
  const float projectedX =
      (target.x * localProject.m[0] + target.y * localProject.m[1] +
       target.z * localProject.m[2] + localProject.m[3]) /
      targetW;
  const float projectedY =
      (target.x * localProject.m[4] + target.y * localProject.m[5] +
       target.z * localProject.m[6] + localProject.m[7]) /
      targetW;

  const float offsetX = 0.5f - projectedX;
  localProject.m[0] += localProject.m[12] * offsetX;
  localProject.m[1] += localProject.m[13] * offsetX;
  localProject.m[2] += localProject.m[14] * offsetX;
  localProject.m[3] += localProject.m[15] * offsetX;

  const float offsetY = 0.5f - projectedY;
  localProject.m[4] += localProject.m[12] * offsetY;
  localProject.m[5] += localProject.m[13] * offsetY;
  localProject.m[6] += localProject.m[14] * offsetY;
  localProject.m[7] += localProject.m[15] * offsetY;
}

void idRenderLightParms::ComputeWorldToLightMatrix(
    idRenderMatrix& baseLightProject,
    idRenderMatrix& inverseBaseLightProject) const {
  idRenderMatrix localProject;
  if (lightType == LIGHT_SPOT) {
    ComputeSpotLightProjectionMatrix(localProject);
  } else {
    std::memset(&localProject, 0, sizeof(localProject));
    localProject.m[3] = 0.5f;
    localProject.m[7] = 0.5f;
    localProject.m[11] = 0.5f;
    localProject.m[0] = 0.5f / lightRadius.x;
    localProject.m[5] = 0.5f / lightRadius.y;
    localProject.m[10] = 0.5f / lightRadius.z;
    localProject.m[15] = 1.0f;
  }

  idRenderMatrix lightMatrix;
  idRenderMatrix inverseLightMatrix;
  idRenderMatrix::CreateFromOriginAxis(origin, axis, lightMatrix);
  if (!idRenderMatrix::Inverse(lightMatrix, inverseLightMatrix))
    idLib::Warning("lightMatrix invert failed");
  idRenderMatrix::Multiply(localProject, inverseLightMatrix, baseLightProject);
  if (!idRenderMatrix::Inverse(baseLightProject, inverseBaseLightProject))
    idLib::Warning("baseLightProject invert failed");
}

void idRenderLightParms::ComputeLightGlobalOrigin(idVec3& globalOrigin) const {
  if (lightType == LIGHT_PARALLEL) {
    const float lengthSqr = lightCenter.x * lightCenter.x +
                            lightCenter.y * lightCenter.y +
                            lightCenter.z * lightCenter.z;
    const float inverseLength =
        1.0f / std::sqrt(std::max(lengthSqr, idMath::FLT_SMALLEST_NON_DENORMAL));
    idVec3 direction = lightCenter * inverseLength;
    if (inverseLength * lengthSqr == 0.0f)
      direction.z = 1.0f;
    globalOrigin = origin + direction * 100000.0f;
    return;
  }

  if (lightType == LIGHT_SPOT) {
    globalOrigin = origin;
    return;
  }

  globalOrigin = origin + axis[0] * lightCenter.x + axis[1] * lightCenter.y +
                 axis[2] * lightCenter.z;
}

