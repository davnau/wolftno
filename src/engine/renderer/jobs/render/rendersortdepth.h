#pragma once

#include <cstdint>

struct idRenderModelCommitted;
class idRenderView;
struct renderSettings_t;

struct renderSortDepthParms_t {
  const renderSettings_t* settings;
  const idRenderView* renderView;
  const idRenderModelCommitted** viewModels;
  std::uint64_t* viewModelSortKeys;
  int numViewModels;
  const idRenderModelCommitted** viewModelsPreZ;
  int (*sortedViewModels)[2];
  int* numViewSceneModels;
};

void DepthModelQuickSort(const idRenderModelCommitted** viewModels,
                         char* keys, std::int64_t count);
void RenderSortDepth(const renderSortDepthParms_t* parameters);

static_assert(sizeof(renderSortDepthParms_t) == 64,
              "render-sort-depth parameters ABI drift");
