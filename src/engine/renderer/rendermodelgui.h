#pragma once

// NEEDS_WORK(engine/renderer): PDB-derived GUI renderer boundary.  The renderer
// folder owns these method bodies and the complete class declaration.

#include "shared/idlib/geometry/drawvert.h"
#include "shared/idlib/math/vector.h"

#include <cstdint>

class idMaterial;
class idDeclRenderParm;

class idRenderModelGui {
public:
  void DrawStretchPic(float x, float y, float z, float width, float height,
                      float s1, float t1, float s2, float t2,
                      const idMaterial* material);
  void DrawStretchPic(const idVec4& topLeft, const idVec4& topRight,
                      const idVec4& bottomRight, const idVec4& bottomLeft,
                      const idMaterial* material, float z);
  void DrawTris(const idDrawVert* vertices, int numVertices,
                const std::uint16_t* indices, int numIndices,
                const idMaterial* material);
  idDrawVert* AllocTris(int numVertices, const std::uint16_t* indices,
                        int numIndices, const idMaterial* material);
  void SetGLState(std::uint64_t state);
  float GetVirtualWidth() const;
  float GetVirtualHeight() const;
  void SetParm(const idDeclRenderParm* parm, float value);

  std::uint32_t currentVertexColor;
};
