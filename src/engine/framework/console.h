#pragma once

// NEEDS_WORK(engine/framework): PDB-derived console vtable prefix required by
// renderer job timing. The complete console interface is recovered with the
// framework folder; this preserves the retail AddJobListTimeRange slot.

#include <cstdint>

struct idColor;
class idParallelJobList;
struct idConsole;

struct idOverlayHandle {
  int index{-1};
  int time{};
};

enum rangeId_t : int {
  RANGE_MAIN = 0,
  RANGE_RENDER = 1,
  RANGE_GPU = 2,
  RANGE_SOUND = 3,
  RANGE_JBL_COLL = 4,
  RANGE_JBL_GAME = 5,
  RANGE_JBL_RMDL = 6,
  RANGE_JBL_RNDR = 7,
  RANGE_JBL_VDEO = 8,
  RANGE_JBL_ANLZ = 9,
  RANGE_JBL_VIRT = 10,
  RANGE_JBL_SNDO = 11,
  RANGE_JBL_SNAP = 12,
};

struct idConsoleVTablePrefix {
  void* slotsBeforeAddTimeRange[17];
  void (*AddTimeRange)(idConsole* self, rangeId_t range,
                       const idColor* color, std::uint64_t start,
                       std::uint64_t end);
  void* slotBeforeAddJobListTimeRange;
  void (*AddJobListTimeRange)(idConsole* self, rangeId_t range,
                              const idColor* color,
                              const idParallelJobList* jobList,
                              std::uint64_t offset);
  void (*BeginTimeRange)(idConsole* self, rangeId_t range,
                         const idColor* color);
};

struct idConsole {
  const idConsoleVTablePrefix* vtable;

  void AddTimeRange(rangeId_t range, const idColor* color,
                    std::uint64_t start, std::uint64_t end) {
    vtable->AddTimeRange(this, range, color, start, end);
  }

  void AddJobListTimeRange(rangeId_t range, const idColor* color,
                           const idParallelJobList* jobList,
                           std::uint64_t offset) {
    vtable->AddJobListTimeRange(this, range, color, jobList, offset);
  }

  void BeginTimeRange(rangeId_t range, const idColor* color) {
    vtable->BeginTimeRange(this, range, color);
  }

  void Close(bool force) {
    using Fn = void (*)(idConsole*, bool);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[10])(this, force);
  }

  void Draw(bool forceFullScreen, bool noNotifies) {
    using Fn = void (*)(idConsole*, bool, bool);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[13])(this, forceFullScreen, noNotifies);
  }

  template <typename... Args>
  void PrintOverlay(idOverlayHandle* handle, int justify, float x, float y,
                    const char* format, Args... args) {
    using Fn = void (*)(idConsole*, idOverlayHandle*, int, float, float,
                        const char*, ...);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[16])(this, handle, justify, x, y, format,
                                    args...);
  }
};

extern idConsole* console;

static_assert(sizeof(idConsole) == 8, "idConsole ABI drift");
static_assert(sizeof(idOverlayHandle) == 8, "overlay handle ABI drift");
