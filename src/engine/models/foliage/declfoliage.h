#pragma once

#include "engine/decls/decl.h"
#include "shared/idlib/math/vector.h"

class idMaterial;

class idDeclFoliage : public idDecl {
public:
  void List() override;

  std::uint16_t quadWidth = 0;
  std::uint16_t quadHeight = 0;
  float widthVariance = 0.0f;
  float heightVariance = 0.0f;
  idVec3 colorVariance{};
  bool rndFlipHoriz = false;
  std::uint8_t pad105[3]{};
  float swayMagnitude = 0.0f;
  const idMaterial* material = nullptr;
  std::int32_t shape = 0;
  std::uint32_t pad124 = 0;
  idList<const idMaterial*, 5> stampMaterials;
  float stampScale = 0.0f;
  float stampParms[4]{};
  float stampCovers[4]{};
  std::uint32_t pad188 = 0;
  idAtomicString stampBlendMode;
  bool isOpaque = false;
  bool isReferenceType = false;
  bool isSinglePlacement = false;
  std::uint8_t tailPad[5]{};
};

static_assert(sizeof(idDeclFoliage) == 208);
