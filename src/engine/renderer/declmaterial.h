#pragma once

// NEEDS WORK: this is the renderer-facing declaration required by recovered
// compiler code.  The renderer folder owns the full idMaterial fields and
// implementation and will replace this declaration when that folder is recovered.

#include "engine/decls/decl.h"
#include "engine/renderer/parmblock.h"

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

  std::uint8_t recoveredPrefix[32]{};
  idParmBlock parmBlock;
  std::uint8_t recoveredTail[240]{};

  static idDeclInfoTemplate<idMaterial> resourceList;
};

const idMaterial* FindMaterialOrAutoMaterial(const char* name);

static_assert(offsetof(idMaterial, parmBlock) == 112,
              "idMaterial parmBlock offset drift");
static_assert(sizeof(idMaterial) == 400, "idMaterial ABI drift");
