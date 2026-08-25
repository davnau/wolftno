#pragma once

#include "shared/idlib/containers/heaparray.h"

// PDB-derived renderer boundary used by declmaterial.cpp.  Method bodies and
// the remaining virtual-texture internals belong to the corresponding
// virtualtexture/virtualtexturesystem compilands and are intentionally deferred.

#include "shared/idlib/math/vector.h"
#include "shared/idlib/text/atomicstring.h"
#include "shared/idlib/text/str.h"

#include <cstddef>
#include <cstdint>

class idDeclRenderParm;
class idFile;
class idImage;
class idMaterial;
class idVmtrCPU;
class idVMTRData;

enum physicalPagesPool_t : int {
  PHYSICAL_PAGES_POOL_VMTR = 0,
  PHYSICAL_PAGES_POOL_UNIQUE = 1,
  PHYSICAL_PAGES_POOL_UNIQUE_DIFFUSE_ONLY = 2,
  PHYSICAL_PAGES_POOL_UNIQUE_DIFFUSE_ONLY2 = 3,
  PHYSICAL_PAGES_POOL_SHARED_COUNT = 4,
  PHYSICAL_PAGES_POOL_PRT = 5,
  PHYSICAL_PAGES_POOL_MAX = 6,
};

enum pageSource_t : int {
  PAGESOURCE_INVALID = 0,
  PAGESOURCE_ALLOC0 = 1,
  PAGESOURCE_ALLOC1 = 2,
  PAGESOURCE_ALLOC2 = 3,
  PAGESOURCE_ALLOC3 = 4,
  PAGESOURCE_ALLOC4 = 5,
  PAGESOURCE_ALLOC5 = 6,
  PAGESOURCE_ALLOC6 = 7,
  PAGESOURCE_ALLOC7 = 8,
  PAGESOURCE_ALLOC8 = 9,
  PAGESOURCE_ALLOC9 = 10,
  PAGESOURCE_ALLOC10 = 11,
  PAGESOURCE_ALLOC11 = 12,
  PAGESOURCE_ALLOC12 = 13,
  PAGESOURCE_ALLOC13 = 14,
  PAGESOURCE_ALLOC14 = 15,
  PAGESOURCE_TOTAL = 16,
};

enum virtualTextureFlavor_t : int {
  ID_VTF_UNKNOWN = 0,
  ID_VTF_BAKE = 1,
  ID_VTF_VMTR = 2,
};

class idPhysicalPages {
public:
  const idDeclRenderParm* GetFilterParms() const;

  std::uint8_t recoveredPrefix[0xB0]{};
  idImage* physicalPagesImage0{};
  idImage* physicalPagesImage1{};
  idImage* physicalPagesImage2{};
  idImage* physicalMappingsImage{};
  std::uint8_t recoveredSuffix[0x30]{};
};

class idVirtualTexture {
public:
  idVec4 GetFeedbackVec4() const;
  const char* GetBaseName() const;
  void SetMapName(const char* mapName);

  virtualTextureFlavor_t flavor{ID_VTF_UNKNOWN};
  std::uint8_t alignmentAfterFlavor[4]{};
  idVMTRData* vmtrData{};
  std::uint8_t recoveredToPageSource[0x298]{};
  pageSource_t pageSource{PAGESOURCE_INVALID};
  int layoutVersion{};
  int storedNumLevels{};
  std::uint8_t alignmentBeforeFiles[4]{};
  idFile* pageFile{};
  idFile* pageFileInstalled{};
  int minMipLevelInstalled{};
  int VIRTUAL_PAGES_WIDE_LOG2{};
  int VIRTUAL_TEXELS_WIDE{};
  int VIRTUAL_MAPPING_PAGES_WIDE{};
  std::uint8_t recoveredToMapName[0x20]{};
  idStrStatic<128> mapName;
  idPhysicalPages* physicalPages{};
  std::uint8_t recoveredToPageImages[8]{};
  idImage* pageTableImage{};
  idImage* pageMinLodImage{};
  std::uint8_t recoveredSuffix[0x98]{};
};

class idVirtualMaterial {
public:
  idAtomicString materialName;
  unsigned materialChecksum{};
  unsigned cpuVmtrTimeStamp{};
  int allocX{};
  int allocY{};
  int allocWidth{};
  int allocHeight{};
  int flags{};
  std::uint8_t alignmentBeforePointers[4]{};
  const idMaterial* material{};
  const idVmtrCPU* cpuVmtr{};
  idVec4 scaleAndBias{};
  idVMTRData* vmtrData{};
};

class idPixelPackBuffer;
class idRenderDestination;

class idVirtualTextureSystem {
public:
  // NEEDS_WORK(engine/renderer/virtualtexture): lifecycle implementation is
  // owned by the virtual-texture folder; these are its retail-facing calls.
  void Init();
  void Shutdown();
  idVirtualTexture* VirtualTextureForMaterial(const idMaterial* material);
  idVirtualMaterial* VirtualMaterialForMaterial(const idMaterial* material);
  void UpdateFilterParms(bool forceUpdateFilterParms);
  void PushFeedback(const unsigned char* feedback, int width, int height,
                    int pitch, bool usePBO);
  void FinishFeedback(bool force, int maxTimeMilliseconds);

  idVirtualTexture vts[PAGESOURCE_TOTAL];
  idVirtualTexture* vtPtrs[PAGESOURCE_TOTAL]{};
  // The intervening members are owned by the virtual-texture recovery pass.
  // Preserve their retail footprint so the renderer-facing feedback members
  // retain the PDB offsets used by Render.obj.
  std::uint8_t recoveredToFeedbackBuffer[30120]{};
  idHeapArray<unsigned char> feedbackBuffer;
  int feedbackBufferPitch{};
  std::uint32_t alignmentBeforeFeedbackObject{};
  idPixelPackBuffer* feedbackBufferObject{};
  idRenderDestination* renderDestFeedback[3]{};
  std::uint8_t recoveredTail[1168]{};
};

extern idVirtualTextureSystem virtualTextureSystem;

// NEEDS_WORK(engine/renderer/virtualtexture): recover the preloader internals
// with its owning folder. RenderSystem_init only requires this lifecycle API.
class idVirtualTexturePreloader {
public:
  void SpawnThread();
  void DestroyThread();
};

extern idVirtualTexturePreloader virtualTexturePreloader;

static_assert(offsetof(idPhysicalPages, physicalPagesImage0) == 0xB0,
              "idPhysicalPages image offset drift");
static_assert(offsetof(idPhysicalPages, physicalMappingsImage) == 0xC8,
              "idPhysicalPages mapping image offset drift");
static_assert(offsetof(idVirtualTexture, pageSource) == 0x2A8,
              "idVirtualTexture page-source offset drift");
static_assert(offsetof(idVirtualTexture, pageFile) == 0x2B8,
              "idVirtualTexture page-file offset drift");
static_assert(offsetof(idVirtualTexture, VIRTUAL_MAPPING_PAGES_WIDE) == 0x2D4,
              "idVirtualTexture mapping width offset drift");
static_assert(offsetof(idVirtualTexture, mapName) == 0x2F8,
              "idVirtualTexture map-name offset drift");
static_assert(offsetof(idVirtualTexture, physicalPages) == 0x3A0,
              "idVirtualTexture physical-pages offset drift");
static_assert(offsetof(idVirtualTexture, pageTableImage) == 0x3B0,
              "idVirtualTexture page-table offset drift");
static_assert(sizeof(idVirtualTexture) == 0x458,
              "idVirtualTexture ABI drift");
static_assert(offsetof(idVirtualMaterial, scaleAndBias) == 0x38,
              "idVirtualMaterial mapping offset drift");
static_assert(offsetof(idVirtualTextureSystem, feedbackBuffer) == 48040,
              "virtual-texture feedback buffer offset drift");
static_assert(offsetof(idVirtualTextureSystem, feedbackBufferObject) == 48064,
              "virtual-texture feedback PBO offset drift");
static_assert(offsetof(idVirtualTextureSystem, renderDestFeedback) == 48072,
              "virtual-texture feedback destination offset drift");
static_assert(sizeof(idVirtualTextureSystem) == 49264,
              "virtual-texture system ABI drift");
static_assert(sizeof(idVirtualMaterial) == 0x50,
              "idVirtualMaterial ABI drift");
