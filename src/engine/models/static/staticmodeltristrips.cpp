#include "engine/models/static/staticmodeltristrips.h"

#include "engine/models/static/tristripforsyth.h"
#include "shared/idlib/sys/sys_alloc.h"

#include <algorithm>
#include <vector>

namespace {
constexpr std::uint16_t kStripRestart = 0xffffu;
bool IsDegenerateTail(const idList<std::uint16_t, 5>& triangles) {
  const int count = triangles.num;
  return triangles.list[count - 3] == triangles.list[count - 2] ||
         triangles.list[count - 3] == triangles.list[count - 1] ||
         triangles.list[count - 2] == triangles.list[count - 1];
}
struct ForsythResultOwner {
  int* data = nullptr;
  ~ForsythResultOwner() {
    if (data) idMem::Singleton()->Free(data, ALIGN_16);
  }
};
} // namespace

void StripIndexesToTriangleIndexes(const std::uint16_t* stripIndexes,
                                   int numStripIndexes,
                                   idList<std::uint16_t, 5>* triIndexes) {
  triIndexes->ClearFree();
  int scan = 0;
  while (scan < numStripIndexes) {
    while (scan < numStripIndexes && stripIndexes[scan] == kStripRestart)
      ++scan;
    if (scan == numStripIndexes) break;
    const int stripStart = scan;
    triIndexes->Append(stripIndexes[scan++]);
    triIndexes->Append(stripIndexes[scan++]);
    triIndexes->Append(stripIndexes[scan++]);
    std::uint16_t previousPrevious = stripIndexes[stripStart + 1];
    std::uint16_t previous = stripIndexes[stripStart + 2];
    int parity = 0;
    while (scan < numStripIndexes) {
      if (IsDegenerateTail(*triIndexes)) triIndexes->num -= 3;
      const std::uint16_t current = stripIndexes[scan++];
      if (current == kStripRestart) break;
      triIndexes->Append(current);
      if (parity != 0) {
        triIndexes->Append(previousPrevious);
        triIndexes->Append(previous);
      } else {
        triIndexes->Append(previous);
        triIndexes->Append(previousPrevious);
      }
      parity ^= 1;
      previousPrevious = previous;
      previous = current;
    }
  }
}

void InOrderTriangleStripper(const std::uint16_t* triIndexes,
                             int numTriIndexes,
                             idList<std::uint16_t, 5>* stripIndexes) {
  stripIndexes->ClearFree();
  int triangleOffset = 0;
  if (numTriIndexes <= 0) return;
  const int finalTriangleOffset = numTriIndexes - 3;
  while (triangleOffset != finalTriangleOffset) {
    int sharedPrefix = 0;
    for (int vertex = 0; vertex < 2; ++vertex) {
      bool found = false;
      for (int nextVertex = 0; nextVertex < 3; ++nextVertex) {
        if (triIndexes[triangleOffset + vertex] ==
            triIndexes[triangleOffset + 3 + nextVertex]) {
          found = true;
          break;
        }
      }
      if (!found) break;
      ++sharedPrefix;
    }
    for (int vertex = 0; vertex < 3; ++vertex)
      stripIndexes->Append(
          triIndexes[triangleOffset + (vertex + sharedPrefix) % 3]);
    triangleOffset += 3;
    if (triangleOffset >= numTriIndexes) return;

    for (;;) {
      const int count = stripIndexes->num;
      int remainingVertex = 0;
      int orientation = 2;
      while (triIndexes[triangleOffset + (orientation - 1) % 3] !=
                 stripIndexes->list[count - 1] ||
             triIndexes[triangleOffset + orientation % 3] !=
                 stripIndexes->list[count - 2]) {
        ++orientation;
        ++remainingVertex;
        if (orientation >= 5) break;
      }
      if (remainingVertex == 3) break;
      stripIndexes->Append(triIndexes[triangleOffset + remainingVertex]);
      triangleOffset += 3;
      if (triangleOffset >= numTriIndexes) return;
    }
    if (triangleOffset >= numTriIndexes) return;
    stripIndexes->Append(kStripRestart);
  }
  stripIndexes->Append(triIndexes[triangleOffset]);
  stripIndexes->Append(triIndexes[triangleOffset + 1]);
  stripIndexes->Append(triIndexes[triangleOffset + 2]);
}

void TriangleStripper(const std::uint16_t* indexes, unsigned int numIndexes,
                      idList<std::uint16_t, 5>* stripIndexes) {
  const int indexCount = static_cast<int>(numIndexes);
  std::vector<int> oldOrder(static_cast<std::size_t>(indexCount));
  int maximumIndex = -1;
  for (int index = 0; index < indexCount; ++index) {
    oldOrder[index] = indexes[index];
    maximumIndex = std::max(maximumIndex, oldOrder[index]);
  }
  ForsythResultOwner reordered;
  reordered.data = reorderForsyth(oldOrder.data(), indexCount / 3,
                                  maximumIndex + 1);
  if (!reordered.data) {
    reordered.data = static_cast<int*>(idMem::Singleton()->AllocWithLocation(
        "StaticModelTriStrips fallback indices",
        sizeof(int) * static_cast<std::size_t>(indexCount), TAG_NEW, false,
        ALIGN_16, HEAP_DEFAULTHEAP));
    for (int index = 0; index < indexCount; ++index)
      reordered.data[index] = indexes[index];
  }
  idList<std::uint16_t, 5> triangleIndexes;
  triangleIndexes.ResizeForPort(indexCount);
  for (int index = 0; index < indexCount; ++index)
    triangleIndexes.Append(static_cast<std::uint16_t>(reordered.data[index]));
  InOrderTriangleStripper(triangleIndexes.list, triangleIndexes.num,
                          stripIndexes);
}
