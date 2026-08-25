#pragma once

#include "decl.h"
#include "shared/idlib/color.h"
#include "shared/idlib/lookuptable.h"

class idMaterial;
class idDeclParticle;

class idDeclTable final : public idDecl {
public:
  idDeclTable();
  ~idDeclTable() override;
  void FreeData() override;
  const char* DefaultDefinition() const override;
  idDeclInfo* GetDeclInfo() const override;
  void Parse(idParser& source) override;
  float TableLookup(float value, bool fastSearch = true) const;
  idLookupTable& LookupTable() { return *table; }
  const idLookupTable& LookupTable() const { return *table; }

  float left;
  float right;
  idLookupTable* table;
  static idDeclInfoTemplate<idDeclTable> resourceList;
};

struct idTrailParm {
  float start;
  float end;
  const idDeclTable* table;
  float GetValue(float fraction) const;
};

class idDeclTrailBeam final : public idDeclTypeInfo {
public:
  struct trail_t {
    const idMaterial* material;
    idColor color;
    float duration;
    float fadeInDuration;
    float fadeOutDuration;
    float fadeInLength;
    float fadeOutLength;
    float speed;
    float textureRepeat;
    idTrailParm textureScrollSpeed;
    idTrailParm width;
  };
  struct quad_t {
    const idDeclParticle* particle;
    float separation;
    float velocityScale;
  };

  idDeclTrailBeam();
  idDeclInfo* GetDeclInfo() const override;
  trail_t trailProperties;
  quad_t quadProperties;
  static idDeclInfoTemplate<idDeclTrailBeam> resourceList;
};

