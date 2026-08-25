#pragma once

#include "shared/idlib/metrics/gaugemetric.h"

#include <cstddef>

class idStr;

// NEEDS_WORK(engine/renderer/resolutionscale.cpp): declaration and ABI are
// complete; behavior belongs to the later resolutionscale.cpp pass.
class idResolutionScale {
public:
  idResolutionScale();
  void InitForMap(const char* mapName);
  void GetCurrentResolutionScale(float& x, float& y) const;
  void SetCurrentGPUFrameTime(int microseconds);
  void GetConsoleText(idStr& text);
  void LogMetrics();

  idGaugeMetric resolutionScaleMetricX;
  idGaugeMetric resolutionScaleMetricY;
  float dropMilliseconds;
  float raiseMilliseconds;
  float currentResolution;
  int framesAboveRaise;
};

extern idResolutionScale resolutionScale;

static_assert(offsetof(idResolutionScale, resolutionScaleMetricY) == 128,
              "resolution scale second metric offset drift");
static_assert(offsetof(idResolutionScale, currentResolution) == 264,
              "resolution scale current value offset drift");
static_assert(sizeof(idResolutionScale) == 272,
              "resolution scale ABI drift");
