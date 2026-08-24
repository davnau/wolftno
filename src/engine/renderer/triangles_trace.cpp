#include "engine/renderer/triangles.h"

#include "shared/idlib/geometry/geometry.h"

float idTriangles::GetSurfaceArea() const {
  if (indexes == nullptr)
    return 0.0f;
  float area = 0.0f;
  for (int i = 0; i < numIndexes; i += 3) {
    area += idGeometry::TriangleArea(verts[indexes[i]].xyz,
                                     verts[indexes[i + 1]].xyz,
                                     verts[indexes[i + 2]].xyz);
  }
  return area;
}
