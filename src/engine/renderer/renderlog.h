#pragma once

class idRenderLog {
public:
  void OpenBlock(const char* format, ...);
};

extern idRenderLog renderLog;

static_assert(sizeof(idRenderLog) == 1, "render-log ABI drift");
