#pragma once

#include "engine/aas2file/aas2file.h"

#include <cstddef>

class idAAS2Local {
public:
  char GetAreaCells(int areaNum, int subSize, int* subx, int* suby);
  char WorldPosToCellPos(int areaNum, int subSize, const idVec3* origin,
                         int* subx, int* suby);
  char CellPosToWorldPos(int areaNum, int subSize, int subx, int suby,
                         idVec3* worldPos);

  void* vtable;
  idAAS2File* file;
};

static_assert(offsetof(idAAS2Local, file) == 8, "idAAS2Local ABI drift");

