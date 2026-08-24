#pragma once

// NEEDS WORK: this is the renderer-facing declaration required by recovered
// compiler code.  The renderer folder owns the full idMaterial fields and
// implementation and will replace this declaration when that folder is recovered.

#include "engine/decls/decl.h"
#include "engine/renderer/parmblock.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idDeclRenderProg;
class idImage;
class idMaterialMap;

enum materialCoverage_t : int {
  MC_BAD = 0,
  MC_INVISIBLE = 1,
  MC_PERFORATED = 2,
  MC_TRANSLUCENT = 3,
  MC_OPAQUE = 4,
  MC_PERFORATED_FOR_MODELFADE = 5,
};

enum idVirtualTextureFlavor_t : int {
  ID_VTF_VMTR = 0,
  ID_VTF_BAKE = 1,
};

class idMaterial : public idDecl {
public:
  idMaterial();
  ~idMaterial() override;
  idDeclInfo* GetDeclInfo() override;
  void Parse(idParser& source) override;
  void ReloadImages(bool force) const;
  int GetWidth() const;
  int GetHeight() const;
  static bool IsValidMaterialName(const char* name, bool allowUnderscore,
                                  bool allowAmpersand);

  union {
    std::uint8_t recoveredPrefix[32]{};
    struct {
      idVec4 atlasScaleBias;
      std::uint8_t flags0;
      std::uint8_t flags1;
      std::uint8_t alignmentBeforeAutosprite[2];
      int materialAutospriteType;
      int materialDecalProjType;
      std::uint8_t alignmentBeforeParmBlock[4];
    };
  };
  idParmBlock parmBlock{};
  idParmBlock renderParmBlock{};
  int pageSource{};
  int pageFlavor{};
  const char* pageFileName{};
  int physicalPagesPool{};
  std::uint8_t alignmentBeforePrograms[4]{};
  const idDeclRenderProg* progs[9]{};
  float stageSort[9]{};
  float stageSortAdjust{};
  const idMaterialMap* materialMap{};
  void* devVmtr{};
  idVec4 virtualMapping{};
  const idImage* editorImage{};
  materialCoverage_t coverage{MC_BAD};
  std::uint8_t alignmentBeforeCompatible[4]{};
  const idMaterial* compatibleGenericMaterial{};

  static idDeclInfoTemplate<idMaterial> resourceList;
};

const idMaterial* FindMaterialOrAutoMaterial(const char* name);

static_assert(offsetof(idMaterial, parmBlock) == 112,
              "idMaterial parmBlock offset drift");
static_assert(offsetof(idMaterial, renderParmBlock) == 160,
              "idMaterial render parm-block offset drift");
static_assert(offsetof(idMaterial, progs) == 232,
              "idMaterial program offset drift");
static_assert(offsetof(idMaterial, stageSort) == 304,
              "idMaterial stage-sort offset drift");
static_assert(offsetof(idMaterial, coverage) == 384,
              "idMaterial coverage offset drift");
static_assert(sizeof(idMaterial) == 400, "idMaterial ABI drift");
