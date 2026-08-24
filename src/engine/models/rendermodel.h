#pragma once

#include "engine/renderer/triangles.h"
#include "engine/renderer/parmblock.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/math/vector.h"
#include "shared/idlib/text/atomicstring.h"
#include "shared/idlib/text/str.h"

#include <cstdint>

class idConstantBuffer;
class idJointBuffer;
class idMaterial;
class idParmBlock;
class idRenderModelUpdateTools;
class idRenderView;
class idRenderModel;

struct idRenderModelParmLinks {
  std::uint8_t prefix[184];
  idRenderModel* imposterParent;
  idRenderModel* imposterNext;
  std::uint8_t viewport[16];
};

static_assert(offsetof(idRenderModelParmLinks, imposterParent) == 184,
              "idRenderModelParms imposterParent offset drift");
static_assert(offsetof(idRenderModelParmLinks, imposterNext) == 192,
              "idRenderModelParms imposterNext offset drift");
static_assert(sizeof(idRenderModelParmLinks) == 216,
              "idRenderModelParms ABI drift");

enum renderModelType_t : int {
  RENDERMODEL_DEFAULT = 0,
  RENDERMODEL_CLOTH = 1,
  RENDERMODEL_DETAIL = 2,
  RENDERMODEL_DETAILEDIT = 3,
  RENDERMODEL_DISCRETEANIMATION = 4,
  RENDERMODEL_MANYBOXES = 5,
  RENDERMODEL_FLARE = 6,
  RENDERMODEL_FOLIAGE = 8,
  RENDERMODEL_GLASS = 10,
  RENDERMODEL_EFFECTS = 11,
  RENDERMODEL_PARTICLE = 12,
  RENDERMODEL_STATIC = 14,
  RENDERMODEL_WATERSURFACE = 16,
  RENDERMODEL_BEAM = 18,
  RENDERMODEL_DECAL = 19,
  RENDERMODEL_GUI = 20,
  RENDERMODEL_IMPOSTER = 21,
  RENDERMODEL_OCEAN = 23,
  RENDERMODEL_LOD = 25,
  RENDERMODEL_CHAIN = 26,
  RENDERMODEL_CUTTABLEBEAMS = 27,
  RENDERMODEL_CUTTABLEDYNAMIC = 28,
  RENDERMODEL_CUTTABLESURFACE = 29,
  RENDERMODEL_GODRAYS = 30,
  RENDERMODEL_DUST = 31,
  RENDERMODEL_CABLE = 32,
  RENDERMODEL_TYPE_MAX = 33,
};

struct idRenderModelSurface {
  const idMaterial* material = nullptr;
  int materialNum = 0;
  std::uint64_t extraGLState = 0;
  idTriangles* geometry = nullptr;
  bool geometryIsReference = false;
  std::uint8_t referenceMask = 0;
  int binaryModelId = 0;
  idJointBuffer* joints = nullptr;
  idVertexBuffer* morphMap = nullptr;
  idVertexBuffer* stMap = nullptr;
  idConstantBuffer* surfaceConstants = nullptr;
  idParmBlock* parms = nullptr;
  idVec4 skinOffsets{};
  float morphMultiplier = 0.0f;
};

static_assert(sizeof(idRenderModelSurface) == 104,
              "idRenderModelSurface ABI drift");

// Full method bodies belong to models/rendermodel.cpp. This declaration exposes
// the recovered surface API needed by subclasses while that compiland is ported.
class idRenderModel {
public:
  idRenderModel();
  virtual ~idRenderModel();

  virtual void Save(idFile* file);
  virtual bool Load(idFile* file);
  virtual bool UpdateInView(const idRenderView* currentView,
                            const idRenderView* nextView,
                            idRenderModelUpdateTools* updateTools);
  void SetName(const char* modelName);
  void FinishSurfaces();
  void FreeSurfaces();
  void AddSurface(const idRenderModelSurface& surface);
  void CommitThisFrame();

  idAtomicString name;
  const char* debugName = nullptr;
  idVec3 deferredOrigin{};
  idMat3 deferredAxis{};
  std::uint8_t useDeferredPosition = 0;
  std::uint8_t deferredPositionInitialized = 0;
  std::uint8_t deleteOnSync = 0;
  std::uint8_t unlinked = 0;
  std::uint8_t forceUpdateInView = 0;
  std::uint8_t recoveredFlagPadding[3]{};
  idBounds referenceBounds{};
  union {
    std::uint8_t recoveredParms[216]{};
    idRenderModelParmLinks recoveredParmLinks;
  };
  idStaticParmBlock<32> gameParmBlock;
  int decalLastUpdateTime = 0;
  int decalUpdateTime = 0;
  void* world = nullptr;
  void* committed = nullptr;
  idList<idRenderModelSurface, 85> surfaces;
  std::uint8_t recoveredDecalData[984]{};
  renderModelType_t renderModelType = RENDERMODEL_DEFAULT;
  std::uint32_t recoveredModelTail = 0;
};

static_assert(offsetof(idRenderModel, referenceBounds) == 80,
              "idRenderModel bounds offset drift");
static_assert(offsetof(idRenderModel, surfaces) == 1160,
              "idRenderModel surfaces offset drift");
static_assert(offsetof(idRenderModel, renderModelType) == 2168,
              "idRenderModel type offset drift");
static_assert(sizeof(idRenderModel) == 2176, "idRenderModel ABI drift");
