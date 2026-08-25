#include "engine/models/rendermodelgodrays.h"

#include "engine/framework/coderesource.h"
#include "engine/renderer/declmaterial.h"
#include "engine/renderer/declrenderparm.h"
#include "engine/renderer/image.h"

#include <algorithm>
#include <cmath>
#include <cstring>

idCodeResource<idDeclRenderParm> rpGodRayViewPos("rpGodRayViewPos");
idCodeResource<idDeclRenderParm> rpGodRaysColor("rpGodRaysColor");
idCodeResource<idDeclRenderParm> rpScaleBiasSource("rpScaleBiasSource");
idCodeResource<idDeclRenderParm> rpScaleBiasQuad("rpScaleBiasQuad");
idCodeResource<idDeclRenderParm> rpTransMap("rpTransMap");

namespace {

const idVec3& ModelOrigin(const idRenderModel& model) {
  if (model.useDeferredPosition != 0)
    return model.deferredOrigin;
  return *reinterpret_cast<const idVec3*>(model.recoveredParms);
}

void SetBoundsFromSize(idRenderModelGodRays& model) {
  const idVec3& origin = ModelOrigin(model);
  const float extent = static_cast<float>(model.size);
  model.referenceBounds.b[0] =
      {origin.x - extent, origin.y - extent, origin.z - extent};
  model.referenceBounds.b[1] =
      {origin.x + extent, origin.y + extent, origin.z + extent};
}

const idImage* FindExistingImage(const char* name) {
  for (int index = 0; index < idImage::resourceList.resourceList.Num(); ++index) {
    const auto* image = static_cast<const idImage*>(
        idImage::resourceList.resourceList[index]);
    if (idStr::Cmp(image->GetName(), name) == 0)
      return image;
  }
  return nullptr;
}

idVec4 Transform(const idRenderMatrix& matrix, const idVec4& value) {
  return {
      matrix.m[0] * value.x + matrix.m[1] * value.y +
          matrix.m[2] * value.z + matrix.m[3] * value.w,
      matrix.m[4] * value.x + matrix.m[5] * value.y +
          matrix.m[6] * value.z + matrix.m[7] * value.w,
      matrix.m[8] * value.x + matrix.m[9] * value.y +
          matrix.m[10] * value.z + matrix.m[11] * value.w,
      matrix.m[12] * value.x + matrix.m[13] * value.y +
          matrix.m[14] * value.z + matrix.m[15] * value.w,
  };
}

} // namespace

idRenderModelGodRays::idRenderModelGodRays() {
  isRelativeToCamera = false;
  direction = {0.0f, 0.0f, -1.0f};
  color = {1.0f, 1.0f, 1.0f};
  sourceSize = 512;
  size = 128;
  fadeVal = 0.0f;
  renderModelType = RENDERMODEL_GODRAYS;
  recoveredParms[116] |= 0xc0U;
  recoveredParms[117] |= 0xa1U;
  recoveredParms[118] |= 0x08U;
  constexpr int modelDepthSortLast = 10;
  std::memcpy(recoveredParms + 172, &modelDepthSortLast,
              sizeof(modelDepthSortLast));
  SetBoundsFromSize(*this);
  imgGodrays = FindExistingImage("_black");
}

idRenderModelGodRays::~idRenderModelGodRays() = default;

void idRenderModelGodRays::SetParms(const idMaterial* texture,
                                    bool isRelativeToCamera_,
                                    const idVec3& direction_,
                                    const idVec3& color_, int size_,
                                    int sourceSize_) {
  isRelativeToCamera = isRelativeToCamera_;
  direction = direction_;
  color = color_;
  sourceSize = sourceSize_;
  size = size_;
  SetBoundsFromSize(*this);
  imgGodrays = FindExistingImage("_black");
  if (texture->parmBlock.SetsRenderParm(rpTransMap.Get())) {
    const idImage* image = texture->parmBlock.GetImage(rpTransMap.Get(), nullptr);
    if (image != nullptr)
      imgGodrays = image;
  }
  gameParmBlock.SetImage(rpTransMap.Get(), imgGodrays);
}

bool idRenderModelGodRays::UpdateInView(const idRenderView*,
                                        const idRenderView* nextView,
                                        idRenderModelUpdateTools*) {
  SetBoundsFromSize(*this);
  idRenderMatrix viewProjection{};
  idRenderMatrix::Multiply(nextView->projectionMatrix, nextView->viewMatrix,
                           viewProjection);

  idVec4 worldPosition;
  if (isRelativeToCamera) {
    worldPosition = {direction.x + nextView->inverseViewMatrix.m[3],
                     direction.y + nextView->inverseViewMatrix.m[7],
                     direction.z + nextView->inverseViewMatrix.m[11], 1.0f};
  } else {
    const idVec3& origin = ModelOrigin(*this);
    worldPosition = {origin.x, origin.y, origin.z, 1.0f};
  }

  const idVec4 clipPosition = Transform(viewProjection, worldPosition);
  const float sourceScaleX =
      static_cast<float>(sourceSize) / static_cast<float>(nextView->renderWidth);
  const float sourceScaleY =
      static_cast<float>(sourceSize) / static_cast<float>(nextView->renderHeight);
  bool visible = clipPosition.w >= 1.17549435e-38f;
  float ndcX = 0.0f;
  float ndcY = 0.0f;
  float left = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
  float top = 0.0f;
  if (visible) {
    ndcX = clipPosition.x / clipPosition.w;
    ndcY = clipPosition.y / clipPosition.w;
    left = std::clamp(ndcX - sourceScaleX, -1.0f, 1.0f);
    right = std::clamp(ndcX + sourceScaleX, -1.0f, 1.0f);
    bottom = std::clamp(ndcY - sourceScaleY, -1.0f, 1.0f);
    top = std::clamp(ndcY + sourceScaleY, -1.0f, 1.0f);
    visible = right > -1.0f && left < 1.0f && top > -1.0f && bottom < 1.0f;
  }

  if (!visible) {
    const idVec4 zero{};
    gameParmBlock.SetParm(rpGodRayViewPos.Get(), zero);
    gameParmBlock.SetParm(rpGodRaysColor.Get(), zero);
    gameParmBlock.SetParm(rpScaleBiasSource.Get(), zero);
    gameParmBlock.SetParm(rpScaleBiasSource.Get(), zero);
    CommitThisFrame();
    return false;
  }

  const idVec4 viewPosition = Transform(nextView->viewMatrix, worldPosition);
  const float coverage = ((top - bottom) * (right - left)) /
                         ((sourceScaleY * 2.0f) * (sourceScaleX * 2.0f));
  gameParmBlock.SetParm(rpGodRayViewPos.Get(), viewPosition);
  gameParmBlock.SetParm(
      rpGodRaysColor.Get(),
      idVec4{fadeVal * color.x, fadeVal * color.y, fadeVal * color.z,
             coverage});
  gameParmBlock.SetParm(rpScaleBiasSource.Get(),
                        idVec4{sourceScaleX, sourceScaleY, ndcX, ndcY});
  gameParmBlock.SetParm(
      rpScaleBiasQuad.Get(),
      idVec4{
          static_cast<float>(size) / static_cast<float>(nextView->renderWidth),
          static_cast<float>(size) / static_cast<float>(nextView->renderHeight),
          ndcX * 0.5f + 0.5f, ndcY * 0.5f + 0.5f});
  CommitThisFrame();
  return true;
}
