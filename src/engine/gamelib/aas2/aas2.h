#pragma once

#include "engine/aas2file/aas2file.h"
#include "engine/gamelib/aas2/aas2_interface.h"

#include <cstddef>

class AAS2SpacialValidationAccess;

class idAAS2Local : public idAAS2 {
public:
  char GetAreaCells(int areaNum, int subSize, int* subx, int* suby);
  char WorldPosToCellPos(int areaNum, int subSize, const idVec3* origin,
                         int* subx, int* suby);
  char CellPosToWorldPos(int areaNum, int subSize, int subx, int suby,
                         idVec3* worldPos);

  int GetNumCoverPoints() const override;
  const aas2Cover_t* GetCoverPoint(int coverIndex) const override;
  int GetNumAreaCoverPoints(int areaNum) const override;
  int GetAreaCoverPoints(int areaNum, int* coverPoints, int maxCover,
                         int& numCover) const override;
  int FindCoverPoints(int areaNum, const idVec3& origin, float maxDist,
                      int requiredFlags, int excludeFlags, int travelFlags,
                      int* coverPoints, int maxCover,
                      int& numCover) const override;
  int GetNumChokePoints() const override;
  const aas2ChokePoint_t* GetChokePoint(int chokeIndex) const override;
  int GetNumHintNodes() const override;
  const aas2HintNode_t* GetHintNode(int hintIndex) const override;

  idAAS2File* file;

private:
  void FindCoverPoints_Internal(int startAreaNum, const idVec3& origin,
                                const idBounds& bounds, float maxDistSqr,
                                int requiredFlags, int excludeFlags,
                                int travelFlags, int* coverPoints,
                                int maxCover, int& numCover) const;

  friend class AAS2SpacialValidationAccess;
};

static_assert(offsetof(idAAS2Local, file) == 8, "idAAS2Local ABI drift");
static_assert(sizeof(idAAS2Local) == 0x10, "idAAS2Local ABI drift");
