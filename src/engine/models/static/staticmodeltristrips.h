#pragma once

#include "shared/idlib/containers/list.h"

#include <cstdint>

void StripIndexesToTriangleIndexes(const std::uint16_t* stripIndexes,
                                   int numStripIndexes,
                                   idList<std::uint16_t, 5>* triIndexes);
void InOrderTriangleStripper(const std::uint16_t* triIndexes,
                             int numTriIndexes,
                             idList<std::uint16_t, 5>* stripIndexes);
void TriangleStripper(const std::uint16_t* indexes, unsigned int numIndexes,
                      idList<std::uint16_t, 5>* stripIndexes);

