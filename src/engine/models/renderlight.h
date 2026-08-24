#pragma once

#include "engine/models/renderworld.h"
#include "engine/models/serializer.h"
#include "engine/renderer/renderlightparms.h"
#include "shared/idlib/text/atomicstring.h"

class idDeclRenderParm;

struct idRenderIndex {
  unsigned index = 0;
};

struct idRenderLightCommitted {
  const char* name = nullptr;
  idRenderWorldCommitted* world = nullptr;
  idRenderIndex index{};
  std::uint32_t recoveredPad20{};
  idRenderLightParms r{};
  std::uint8_t recoveredPad280[8]{};
  idStaticParmBlock<32> renderParmBlock;
  idList<void*> references;
  bool needsReferences = false;
  std::uint8_t recoveredPad1129[3]{};
  idBounds globalReferenceBounds{};
  idVec3 globalLightOrigin{};
  idVec3 currentColor{};
  idVec4 maxVisibleRange{};
  idRenderMatrix baseLightProject{};
  idRenderMatrix inverseBaseLightProject{};
  const idMaterial* lightMtr = nullptr;
  bool castsShadows = false;
  std::uint8_t recoveredTail[7]{};
};

class idRenderLight {
public:
  idRenderLight();
  ~idRenderLight();

  int GetIndex() const;
  idStaticParmBlock<32>* GetParmBlock();
  void SetParm(const idDeclRenderParm* parm, const idVec4& value);
  void CommitThisFrame();
  void Delete();
  void Unlink();
  void SerializeSnapshot(idSerializer* serializer);
  void Commit();

  idAtomicString name;
  bool deleteOnSync = false;
  bool unlinked = true;
  bool needWriteToSnapshot = true;
  bool skipAutoReplication = false;
  int serializedChangeCount = 1;
  idRenderLightParms g;
  idStaticParmBlock<32> gameParmBlock;
  idRenderWorldLocal* world = nullptr;
  idRenderLightCommitted* committed = nullptr;
};

static_assert(sizeof(idRenderIndex) == 4, "idRenderIndex ABI drift");
static_assert(offsetof(idRenderLightCommitted, r) == 24,
              "idRenderLightCommitted parms offset drift");
static_assert(offsetof(idRenderLightCommitted, renderParmBlock) == 288,
              "idRenderLightCommitted parm block offset drift");
static_assert(offsetof(idRenderLightCommitted, lightMtr) == 1328,
              "idRenderLightCommitted material offset drift");
static_assert(sizeof(idRenderLightCommitted) == 1344,
              "idRenderLightCommitted ABI drift");
static_assert(offsetof(idRenderLight, g) == 16,
              "idRenderLight parms offset drift");
static_assert(offsetof(idRenderLight, gameParmBlock) == 272,
              "idRenderLight parm block offset drift");
static_assert(offsetof(idRenderLight, committed) == 1096,
              "idRenderLight committed offset drift");
static_assert(sizeof(idRenderLight) == 1104, "idRenderLight ABI drift");
