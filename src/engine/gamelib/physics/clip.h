#pragma once

#include "engine/cm/collisionmodel.h"

#include <cstdint>

class idClipQuery {
public:
  std::uint64_t index;
};

class idClipModel;

class idClip {
public:
  idClipQuery Translation(trace_t* result, const idVec3& start,
                          const idVec3& end, const idClipModel* clipModel,
                          const idMat3& startAxis, int clipMask,
                          int passEntityNumber, bool moveClipModel,
                          const char* userName);

  bool TracePoint(trace_t& result, const idVec3& start, const idVec3& end,
                  int clipMask, int passEntityNumber, const char* userName);
};

static_assert(sizeof(idClipQuery) == 8, "idClipQuery ABI drift");
