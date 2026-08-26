#include "engine/gamelib/aas2/aas2callback_findflaggedarea.h"

#include "shared/idlib/sys/sys_alloc.h"

idAAS2Callback_FindFlaggedArea::idAAS2Callback_FindFlaggedArea(int areaFlag_,
                                                               bool set)
    : idAAS2Callback_AvoidLocation(), areaFlag(areaFlag_), test(set) {}

idAAS2Callback_FindFlaggedArea::~idAAS2Callback_FindFlaggedArea() = default;

bool idAAS2Callback_FindFlaggedArea::AreaIsGoal(
    idAAS2* aas, int areaNum, const idVec3* potentialGoal) {
  (void)potentialGoal;
  return (areaFlag & aas->GetAreaFlags(areaNum)) == test;
}

void idAAS2Callback_FindFlaggedArea::operator delete(void* memory) noexcept {
  if (memory != nullptr) {
    idMem::Singleton()->Free(memory, ALIGN_16);
  }
}
