#pragma once

#include "engine/renderer/render.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idRenderWorld;

enum sphereNodeFlag_t : int {
  SPHERE_NODE_NONE = 0,
  SPHERE_NODE_SUPER = 1,
  SPHERE_NODE_ROOT_TREE = 2,
  SPHERE_NODE_ROOT_NODE = 4,
  SPHERE_NODE_LINKED = 8,
  SPHERE_NODE_RECOMPUTE = 16,
};

struct sphereNode_t {
  idVec3 origin{};
  float radius{};
  sphereNode_t* parent{};
  sphereNode_t* prev{};
  sphereNode_t* next{};
  sphereNode_t* child{};
  int link{};
  std::uint32_t flags{};
  float slop{};
  int id{};
};

class idSphereTree {
public:
  idSphereTree();
  ~idSphereTree();

  void Alloc(int maxSpheres, float rootSize, float leafSize, float balance);
  sphereNode_t& AddSphere(const idVec3& origin, float radius,
                          sphereNodeFlag_t flags, int id);
  void Update();
  int CollectSpheresFromView(const idRenderView* view, int* ids,
                             int maxIds);
  bool FirstCollision(const idVec3& origin, float radius, int* id);
  bool NextCollision(const idVec3& origin, float radius, int* id);

  int maxSpheres{};
  float rootSize{};
  float leafSize{};
  float balance{};
  idList<sphereNode_t, 5> spheres;
  idList<sphereNode_t*, 5> integrate;
  idList<sphereNode_t*, 5> recompute;
  idList<int, 5> heapId;
  idList<const sphereNode_t*, 5> work;
  sphereNode_t* root{};
  sphereNode_t* leaf{};
};

static_assert(sizeof(sphereNode_t) == 0x40, "sphere-node ABI drift");
static_assert(offsetof(sphereNode_t, radius) == 0x0c,
              "sphere-node radius offset drift");
static_assert(offsetof(sphereNode_t, parent) == 0x10,
              "sphere-node parent offset drift");
static_assert(offsetof(sphereNode_t, link) == 0x30,
              "sphere-node link offset drift");
static_assert(offsetof(sphereNode_t, flags) == 0x34,
              "sphere-node flags offset drift");
static_assert(sizeof(idSphereTree) == 0x98, "sphere-tree ABI drift");
static_assert(offsetof(idSphereTree, spheres) == 0x10,
              "sphere-tree sphere-list offset drift");
static_assert(offsetof(idSphereTree, integrate) == 0x28,
              "sphere-tree integrate-list offset drift");
static_assert(offsetof(idSphereTree, recompute) == 0x40,
              "sphere-tree recompute-list offset drift");
static_assert(offsetof(idSphereTree, heapId) == 0x58,
              "sphere-tree heap-list offset drift");
static_assert(offsetof(idSphereTree, work) == 0x70,
              "sphere-tree work-list offset drift");
static_assert(offsetof(idSphereTree, root) == 0x88,
              "sphere-tree root offset drift");
static_assert(offsetof(idSphereTree, leaf) == 0x90,
              "sphere-tree leaf offset drift");
