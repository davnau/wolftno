#include "engine/models/foliage/jobs/foliagedata.h"

#include <cstdint>

void SetupSubTreePtrsFromData(dm_subTreeDataPtrs_t& subTreePtrs,
                              const dm_subTreeData_t* subTree) {
  const auto base = reinterpret_cast<std::intptr_t>(subTree);
  subTreePtrs.detail = reinterpret_cast<dm_detail_t*>(
      base + static_cast<std::intptr_t>(subTree->detailOffset));
  subTreePtrs.attributes = reinterpret_cast<dm_attribute_t*>(
      base + static_cast<std::intptr_t>(subTree->attributeOffset));
}
