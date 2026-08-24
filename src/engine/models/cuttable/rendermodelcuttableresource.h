#pragma once

#include "engine/decls/decl.h"
#include "engine/models/cuttable/earcliptriangulate.h"
#include "engine/models/cuttable/triangleinterpolator.h"
#include "engine/models/rendermodel.h"
#include "engine/renderer/bufferobject.h"
#include "engine/renderer/triangles.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/text/str.h"

#include <cstdint>

struct Contour;
class idMaterial;

enum surfTypes_t : int {
  SURFTYPE_INVALID = -1,
  SURFTYPE_NONE = 0,
  SURFTYPE_METAL = 1,
  SURFTYPE_STONE = 2,
  SURFTYPE_FLESH = 3,
  SURFTYPE_WOOD = 4,
  SURFTYPE_CARDBOARD = 5,
  SURFTYPE_LIQUID = 6,
  SURFTYPE_GLASS = 7,
  SURFTYPE_PLASTIC = 8,
  SURFTYPE_ASPHALT = 9,
  SURFTYPE_DIRT = 10,
  SURFTYPE_CONCRETE = 11,
  SURFTYPE_FOLIAGE = 12,
  SURFTYPE_LINOLEUM = 13,
  SURFTYPE_FABRIC = 14,
  SURFTYPE_RUBBER = 15,
  SURFTYPE_ROCK = 16,
  SURFTYPE_STEAM_PIPE = 17,
  SURFTYPE_WATER_PIPE = 18,
  SURFTYPE_ARMOR = 19,
  SURFTYPE_SLUDGE = 20,
  SURFTYPE_IMP_NEST = 21,
  SURFTYPE_HOLLOW_METAL = 22,
  SURFTYPE_FLESH_PLAYER = 23,
  SURFTYPE_FLESH_DARKANGEL = 24,
  SURFTYPE_BLOOD_POOL = 25,
  SURFTYPE_ASH = 26,
  SURFTYPE_FLESH_MUTANT = 27,
  SURFTYPE_THICK_PADDING = 28,
  SURFTYPE_PLAYER_APC_METAL = 29,
  SURFTYPE_RICKETY_METAL = 30,
  SURFTYPE_RICKETY_WOOD = 31,
  SURFTYPE_SHEET_METAL = 32,
  SURFTYPE_PORCELAIN = 33,
  SURFTYPE_PLASTER = 34,
  SURFTYPE_VENT_METAL = 35,
  SURFTYPE_ELECTRONICS = 36,
  SURFTYPE_OIL_PIPE = 37,
  SURFTYPE_PANZER_METAL = 38,
  SURFTYPE_BREAKABLE_ARMOR = 39,
  SURFTYPE_ELECTRIC_SHIELD = 40,
  MAX_NUM_SURFTYPES = 41,
};

struct cuttableGenSurface_t {
  const idMaterial* material = nullptr;
  int indexOffset = 0;
  int numIndices = 0;
  int numVertices = 0;
};

struct idCuttableGenGeometry {
  idList<cuttableGenSurface_t> surfaces;
  idList<std::uint16_t> indices;
  idList<idDrawVert> vertices;
  idList<idVec2> st;
};

class idRenderModelCuttableResource : public idResource {
public:
  idRenderModelCuttableResource();
  ~idRenderModelCuttableResource() override;

  bool ReloadIfStale() override;
  void BuildSurfaces(idRenderModel* model);
  idResourceList* GetResourceList() const override;
  bool GenerateSurfaces(idEarClipTriangulate& triangulator,
                        idCuttableGenGeometry& genGeometry,
                        const idVec3& centerOfMass);
  bool GenerateSurfaces(const Contour* outer, const Contour* inner,
                        idCuttableGenGeometry& genGeometry);
  void LoadResource() override;

  const idMaterial* material;
  const idMaterial* depthOnlyMaterial;
  const idMaterial* surfaceTypeMaterial;
  idStr coverMap;
  idTriangles* triangles;
  idVec2* stData;
  idVertexBuffer* stMap;
  idList<idVec3> contourBound;
  idTriangleInterpolator triangleInterpolator;
  int numInterpolatorTriangles;
  std::uint32_t timestamp;
  float thickness;
  surfTypes_t surfaceType;
  bool isOpaque;

  static idTypedResourceList<idRenderModelCuttableResource> resourceList;

private:
  void FreeCPUData();
  void FreeData();
  static const idMaterial* BuildDepthOnlyMaterial(const char* modelName,
                                                   const char* coverMap);
  bool LoadBinary(const char* fileName);
};

static_assert(sizeof(idRenderModelCuttableResource) == 320,
              "idRenderModelCuttableResource ABI drift");
static_assert(alignof(idRenderModelCuttableResource) == 16,
              "idRenderModelCuttableResource alignment drift");
