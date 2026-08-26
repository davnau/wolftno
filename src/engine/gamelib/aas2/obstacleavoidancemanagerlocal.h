#pragma once

#include "shared/idlib/containers/list.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idMD6Anim;
class idParallelJobList;
struct CellSpursJob128;
struct obstacleBox_t;
struct obstacleCorner_t;
struct obstacleEdge_t;
struct obstacleParms_t;
struct obstacleRoute_t;
struct obstacleTrace_t;
struct obstacleVertex_t;

struct obstaclePath_t {
  obstaclePath_t();
  std::uint8_t recovered[0x270];
};

struct idObstacleAvoidance;

struct idObstacleBuffers {
  idObstacleAvoidance* user;
  CellSpursJob128* job;
  obstacleParms_t* parms;
  obstacleRoute_t* route;
  obstacleBox_t* obstacles;
  obstacleVertex_t* vertices;
  obstacleEdge_t* edges;
  obstacleCorner_t* corners;
  obstacleTrace_t* traces;
  float* traceFractions;
  obstaclePath_t* path;
  int numObstacles;
  int numVertices;
  int numEdges;
  int numCorners;
  int numTraces;
  int frameCount;
};

struct idObstacleAvoidance {
  idObstacleBuffers* buffers;
  idVec3 lastDir;
  idVec4 lastCorner;
  int lastUpdateTime;
  int firstUpdateTime;
};

class idObstacleAvoidanceManager {
public:
  virtual ~idObstacleAvoidanceManager();
  static __forceinline void operator delete(void* memory) noexcept {
    idMem::Singleton()->Free(memory, ALIGN_16);
  }
  virtual void Init(idParallelJobList* jobList) = 0;
  virtual void Shutdown() = 0;
  virtual void AllocObstacleBuffers(
      idObstacleAvoidance* obstacleAvoidance) = 0;
  virtual void FreeObstacleBuffers(
      idObstacleAvoidance* obstacleAvoidance) = 0;
  virtual void FrameFreeObstacleBuffers() = 0;
  virtual idParallelJobList* GetJobList() = 0;

protected:
  __forceinline idObstacleAvoidanceManager() noexcept {}
};

class idObstacleAvoidanceManagerLocal : public idObstacleAvoidanceManager {
public:
  idObstacleAvoidanceManagerLocal();
  ~idObstacleAvoidanceManagerLocal() override;

  void Init(idParallelJobList* jobList) override;
  void Shutdown() override;
  void AllocObstacleBuffers(idObstacleAvoidance* obstacleAvoidance) override;
  void FreeObstacleBuffers(idObstacleAvoidance* obstacleAvoidance) override;
  void FrameFreeObstacleBuffers() override;
  idParallelJobList* GetJobList() override { return jobList; }

  idParallelJobList* jobList;
  idList<idObstacleBuffers*, 5> usedBuffers;
  idList<idObstacleBuffers*, 5> freeBuffers;
  idList<idObstacleBuffers*, 5> toBeFreedBuffers;
};

extern idObstacleAvoidanceManagerLocal obstacleAvoidanceManagerLocal;

static_assert(sizeof(obstaclePath_t) == 0x270,
              "retail obstacle path ABI drift");
static_assert(sizeof(idObstacleBuffers) == 0x70,
              "retail obstacle-buffer ABI drift");
static_assert(offsetof(idObstacleBuffers, path) == 0x50,
              "retail obstacle path-pointer offset drift");
static_assert(offsetof(idObstacleBuffers, numObstacles) == 0x58,
              "retail obstacle count offset drift");
static_assert(sizeof(idObstacleAvoidance) == 0x30,
              "retail obstacle-avoidance ABI drift");
static_assert(sizeof(idObstacleAvoidanceManager) == 0x08,
              "retail obstacle-manager ABI drift");
static_assert(offsetof(idObstacleAvoidanceManagerLocal, jobList) == 0x08,
              "retail obstacle job-list offset drift");
static_assert(offsetof(idObstacleAvoidanceManagerLocal, usedBuffers) == 0x10,
              "retail used-buffer list offset drift");
static_assert(offsetof(idObstacleAvoidanceManagerLocal, freeBuffers) == 0x28,
              "retail free-buffer list offset drift");
static_assert(offsetof(idObstacleAvoidanceManagerLocal, toBeFreedBuffers) ==
                  0x40,
              "retail pending-buffer list offset drift");
static_assert(sizeof(idObstacleAvoidanceManagerLocal) == 0x58,
              "retail local obstacle-manager ABI drift");
