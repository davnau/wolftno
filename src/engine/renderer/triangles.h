#pragma once

#include "engine/renderer/bufferobject.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/drawvert.h"

#include <cstdint>

template <typename T, int DefaultTag> class idList;
class idDeclSkins;
class idFile;
class idMaterial;
struct skinRemap_t;
struct sourceSurface_t;

class idVertexScaleAndBias {
public:
  idVec3 xyzScale;
  idVec3 xyzBias;
  idVec2 stScale;
  idVec2 stBias;
};

class idTriangles {
public:
  idTriangles() = default;
  idTriangles(int numVerts, int numIndexes);
  ~idTriangles();
  idTriangles* CreateStandardTriangles() const;
  static idTriangles* CreateStandardTriangles(const idTriangles* source);
  static idTriangles* CopyStaticTriSurf(const idTriangles* source);
  void MergeTriangles(const idTriangles* source);
  static void MergeTriangles(idTriangles* destination,
                             const idTriangles* source);
  static void AllocStaticTriSurfVerts(idTriangles* triangles, int numVerts);
  static void AllocStaticTriSurfIndexes(idTriangles* triangles,
                                        int numIndexes);
  static void UpdateIndexBuffer(idTriangles* triangles);
  static void UpdateVertexBuffer(idTriangles* triangles);
  void ReadFromFile(idFile* file);
  void WriteToFile(idFile* file) const;
  idBounds BoundsFromVertScan() const;
  void VmtrMapTexCoordsWithSkins(
      idList<sourceSurface_t, 5>* sourceSurfaces, int renderSurface,
      const idMaterial* originalMaterial, const idDeclSkins* skins,
      idList<skinRemap_t, 5>* skinRemaps,
      idList<const idMaterial*, 5>* usedMaterials);
  void CreateBounds();
  float GetSurfaceArea() const;
  void FreeCPUData() {
    delete[] verts;
    delete[] indexes;
    verts = nullptr;
    indexes = nullptr;
  }

  idBounds bounds;
  bool generateNormals{};
  int numIndexes{};
  int numVerts{};
  int detailOffset{};
  bool allowGpuHosting{};
  idVertexScaleAndBias vsb{};
  unsigned cpuVertexMask{};
  idDrawVert* verts{};
  std::uint16_t* indexes{};
  unsigned vertexMask{};
  idVertexBuffer vertexBuffer;
  idIndexBuffer indexBuffer;
};

static_assert(sizeof(idVertexScaleAndBias) == 40,
              "idVertexScaleAndBias ABI drift");
static_assert(offsetof(idTriangles, verts) == 88, "idTriangles verts offset drift");
static_assert(offsetof(idTriangles, vertexBuffer) == 112,
              "idTriangles vertex buffer offset drift");
static_assert(sizeof(idTriangles) == 160, "idTriangles ABI drift");
