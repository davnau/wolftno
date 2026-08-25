#pragma once

#include "engine/decls/decl.h"
#include "engine/models/rendermodel.h"
#include "engine/models/skinremap.h"

#include <cstddef>
#include <cstdint>

struct sourceSurface_t {};

struct idStaticModelSurface : idRenderModelSurface {
  idList<skinRemap_t, 5> skinRemaps;
};

class idStaticModel : public idResource {
public:
  idStaticModel();
  ~idStaticModel() override;
  void PristineLoadResource();

  bool pristine{};
  bool cmodel{};
  bool discardReferencePosition{};
  bool isReloadable{};
  char binaryFileName[256]{};
  std::uint32_t binaryTimeStamp{};
  std::uint32_t sourceTimeStamp{};
  int reloadCount{};
  bool modelIsAutosprite{};
  bool usesTransparencySort{};
  std::uint8_t recoveredBoundsPadding[2]{};
  idBounds referenceBounds{};
  idVec3 referencePosition{};
  bool sharedSurfaces{};
  std::uint8_t recoveredFlagsPadding[3]{};
  int modelLoadFlags{};
  idList<idStaticModelSurface, 84> surfaces;
  idList<sourceSurface_t, 5> sourceSurfaces;
  idList<const idMaterial*, 5> materials;
};

class idStaticModelManager {
public:
  virtual ~idStaticModelManager() = default;
  virtual void Init() {}
  virtual const idStaticModel* FindStaticModel(const char* name,
                                               bool load) = 0;
  virtual void AddStaticModel(idStaticModel*) {}
  virtual void ReloadModels() {}
  virtual void CalculateStats(void*) {}
};

extern idStaticModelManager* staticModelManager;

static_assert(sizeof(idStaticModelSurface) == 128,
              "idStaticModelSurface ABI drift");
static_assert(offsetof(idStaticModelSurface, skinRemaps) == 104,
              "idStaticModelSurface remap offset drift");
static_assert(sizeof(idStaticModel) == 440, "idStaticModel ABI drift");
static_assert(offsetof(idStaticModel, binaryFileName) == 52,
              "idStaticModel filename offset drift");
static_assert(offsetof(idStaticModel, referenceBounds) == 324,
              "idStaticModel bounds offset drift");
static_assert(offsetof(idStaticModel, surfaces) == 368,
              "idStaticModel surfaces offset drift");
