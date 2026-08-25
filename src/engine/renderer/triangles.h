#pragma once

#include "engine/renderer/bufferobject.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/drawvert.h"

#include <cstdint>

class idVertexScaleAndBias {
public:
  idVec3 xyzScale;
  idVec3 xyzBias;
  idVec2 stScale;
  idVec2 stBias;
};

class idTriangles {
public:
  ~idTriangles();
  static idTriangles* CreateStandardTriangles(const idTriangles* source);
  static idTriangles* CopyStaticTriSurf(const idTriangles* source);
  static void MergeTriangles(idTriangles* destination,
                             const idTriangles* source);
  static void AllocStaticTriSurfVerts(idTriangles* triangles, int numVerts);
  static void AllocStaticTriSurfIndexes(idTriangles* triangles,
                                        int numIndexes);
  static void UpdateIndexBuffer(idTriangles* triangles);
  static void UpdateVertexBuffer(idTriangles* triangles);
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
