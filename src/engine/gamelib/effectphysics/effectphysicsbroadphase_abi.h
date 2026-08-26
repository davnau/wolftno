#pragma once

#include "engine/gamelib/effectphysics/effectphysicsbroadphase.h"

class idCollisionModelManager {
public:
  bool GetStepMoveResult(trace_t* result, idCollisionQuery& query, bool peek);
  idCollisionQuery Motion(
      trace_t* result, const idVec3& start, const idVec3& end,
      const idRotation& rotation, const idBounds& globalBounds,
      const idTraceModel** const traceModels, int numTraceModels,
      const idMat3& traceModelAxis, int contentsMask,
      const idPositionedCollisionModel* models, int numModels,
      const char* userName);
};

extern idCollisionModelManager* collisionModelManager;
