#include "engine/models/glass/spheretree.h"

#include <algorithm>

namespace {

bool IsActive(const sphereNode_t& node) {
  return (node.flags & 0x80000000u) != 0;
}

bool Intersects(const sphereNode_t& node, const idVec3& origin, float radius) {
  const idVec3 delta = node.origin - origin;
  const float combinedRadius = node.radius + radius;
  return IsActive(node) && delta.LengthSqr() <= combinedRadius * combinedRadius;
}

} // namespace

idSphereTree::idSphereTree() {
  spheres.granularity = 5;
  integrate.granularity = 5;
  recompute.granularity = 5;
  heapId.granularity = 5;
  work.granularity = 5;
}

void idSphereTree::Alloc(int maximum, float rootSizeValue,
                         float leafSizeValue, float balanceValue) {
  maxSpheres = std::max(0, maximum);
  rootSize = rootSizeValue;
  leafSize = leafSizeValue;
  balance = balanceValue;
  spheres.SetNum(maxSpheres);
  for (int index = 0; index < spheres.num; ++index)
    spheres[index] = {};
  integrate.Clear();
  recompute.Clear();
  heapId.Clear();
  work.Clear();
  root = nullptr;
  leaf = nullptr;
}

sphereNode_t& idSphereTree::AddSphere(const idVec3& origin, float radius,
                                      sphereNodeFlag_t flags, int id) {
  for (int index = 0; index < spheres.num; ++index) {
    sphereNode_t& node = spheres[index];
    if (IsActive(node))
      continue;
    node = {};
    node.origin = origin;
    node.radius = radius;
    node.link = index;
    node.flags = static_cast<std::uint32_t>(flags) | 0x80000000u;
    node.id = id;
    return node;
  }
  static sphereNode_t exhausted{};
  exhausted = {};
  exhausted.origin = origin;
  exhausted.radius = radius;
  exhausted.id = id;
  return exhausted;
}

void idSphereTree::Update() {}

int idSphereTree::CollectSpheresFromView(const idRenderView*, int* ids,
                                         int maxIds) {
  int count = 0;
  if (ids == nullptr || maxIds <= 0)
    return 0;
  for (int index = 0; index < spheres.num && count < maxIds; ++index) {
    if (IsActive(spheres[index]))
      ids[count++] = spheres[index].id;
  }
  return count;
}

bool idSphereTree::FirstCollision(const idVec3& origin, float radius, int* id) {
  work.Clear();
  for (int index = spheres.num - 1; index >= 0; --index) {
    if (Intersects(spheres[index], origin, radius))
      work.Append(&spheres[index]);
  }
  return NextCollision(origin, radius, id);
}

bool idSphereTree::NextCollision(const idVec3&, float, int* id) {
  if (work.num <= 0)
    return false;
  const sphereNode_t* node = work[work.num - 1];
  --work.num;
  if (id != nullptr)
    *id = node->id;
  return true;
}
