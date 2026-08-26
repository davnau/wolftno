#pragma once

#include "engine/cm/collisionmodel.h"

#include <cstdint>

class idClipQuery {
public:
  idClipQuery() = default;
  explicit idClipQuery(std::uint64_t queryIndex) : index(queryIndex) {}

  std::uint64_t index;
};

class idClipModel;

class idCollisionQuery {
public:
  std::uint64_t offset;

  static const std::uint64_t finalResultsFirstSubmittedOffset;
  static const std::uint64_t finalResultsLastSubmittedOffset;
};

class idClip {
public:
  struct idClipCollisionQuery {
    int type;
    int parmIndex;
    const idClipModel* clipModel;
    idCollisionQuery query;
  };

  idClipQuery Translation(trace_t* result, const idVec3& start,
                          const idVec3& end, const idClipModel* clipModel,
                          const idMat3& startAxis, int clipMask,
                          int passEntityNumber, bool moveClipModel,
                          const char* userName);

  bool TracePoint(trace_t& result, const idVec3& start, const idVec3& end,
                  int clipMask, int passEntityNumber, const char* userName);

  std::uint8_t recoveredPrefix[0x5C288];
  idClipCollisionQuery collisionQueries[4096];
  std::uint64_t collisionQueryFirstSubmittedIndex;
  std::uint64_t collisionQueryLastSubmittedIndex;
  std::uint8_t recoveredTail[0x15078];
};

static_assert(sizeof(idClipQuery) == 8, "idClipQuery ABI drift");
static_assert(sizeof(idCollisionQuery) == 8,
              "idCollisionQuery ABI drift");
static_assert(sizeof(idClip::idClipCollisionQuery) == 0x18,
              "idClip collision-query ABI drift");
static_assert(offsetof(idClip, collisionQueries) == 0x5C288,
              "idClip collision-query array offset drift");
static_assert(offsetof(idClip, collisionQueryFirstSubmittedIndex) == 0x74288,
              "idClip first-submitted offset drift");
static_assert(offsetof(idClip, collisionQueryLastSubmittedIndex) == 0x74290,
              "idClip last-submitted offset drift");
static_assert(sizeof(idClip) == 0x89310, "idClip ABI drift");
