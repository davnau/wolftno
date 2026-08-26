#pragma once

#include "engine/decls/decl.h"
#include "engine/models/rendermodel.h"
#include "engine/models/skinremap.h"

#include <cstddef>
#include <cstdint>

struct sourceSurface_t {
  const idMaterial* mtr{};
  int mtrChecksum{};
  int renderSurface{};
  int firstVertex{};
  int lastVertex{};
};

class idStaticModelSurface : public idRenderModelSurface {
public:
  idList<skinRemap_t, 5> skinRemaps;
};

class idStaticModel : public idResource {
public:
  static idTypedResourceList<idStaticModel> resourceList;

  idStaticModel();
  ~idStaticModel() override;
  idResourceList* GetResourceList() const override;
  void PristineLoadResource();
  void MakeDefaultModel(float minimum, float maximum);
  void MakeSimpleCube(float minimum, float maximum);
  void MakeExtrudeBox();
  void AddSurface(const idStaticModelSurface& surface);
  int CPUMemory() const;
  int GPUMemory() const;
  void List() const override;

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
  struct idStats {
    int totalCPUMem{};
    int totalGPUMem{};
    int inUse{};
    int totalVerts{};
    int totalTriangles{};
    int totalSurfaces{};
  };

  virtual ~idStaticModelManager() = default;
  static void operator delete(void* pointer) noexcept {
    idMem::Singleton()->Free(pointer, ALIGN_16);
  }
  static void operator delete(void* pointer, std::size_t) noexcept {
    operator delete(pointer);
  }
  virtual void Init() {}
  virtual const idStaticModel* FindStaticModel(const char* name,
                                               bool load) = 0;
  virtual void AddStaticModel(idStaticModel*) {}
  virtual void ReloadModels() {}
  virtual void CalculateStats(idStats&) {}
};

extern idStaticModelManager* staticModelManager;

static_assert(sizeof(idStaticModelSurface) == 128,
              "idStaticModelSurface ABI drift");
static_assert(sizeof(sourceSurface_t) == 24,
              "sourceSurface_t ABI drift");
static_assert(offsetof(sourceSurface_t, mtrChecksum) == 8,
              "sourceSurface_t checksum offset drift");
static_assert(offsetof(sourceSurface_t, lastVertex) == 20,
              "sourceSurface_t last-vertex offset drift");
static_assert(offsetof(idStaticModelSurface, skinRemaps) == 104,
              "idStaticModelSurface remap offset drift");
static_assert(sizeof(idStaticModel) == 440, "idStaticModel ABI drift");
static_assert(sizeof(idStaticModelManager::idStats) == 24,
              "idStaticModelManager::idStats ABI drift");
static_assert(offsetof(idStaticModelManager::idStats, totalCPUMem) == 0x0,
              "idStaticModelManager::idStats CPU-memory offset drift");
static_assert(offsetof(idStaticModelManager::idStats, inUse) == 0x8,
              "idStaticModelManager::idStats in-use offset drift");
static_assert(offsetof(idStaticModelManager::idStats, totalVerts) == 0xC,
              "idStaticModelManager::idStats vertex offset drift");
static_assert(offsetof(idStaticModelManager::idStats, totalTriangles) == 0x10,
              "idStaticModelManager::idStats triangle offset drift");
static_assert(offsetof(idStaticModelManager::idStats, totalSurfaces) == 0x14,
              "idStaticModelManager::idStats surface offset drift");
static_assert(offsetof(idStaticModel, binaryFileName) == 52,
              "idStaticModel filename offset drift");
static_assert(offsetof(idStaticModel, referenceBounds) == 324,
              "idStaticModel bounds offset drift");
static_assert(offsetof(idStaticModel, surfaces) == 368,
              "idStaticModel surfaces offset drift");
