#pragma once

// NEEDS_WORK(engine/framework): PDB-derived console vtable prefix required by
// renderer job timing. The complete console interface is recovered with the
// framework folder; this preserves the retail AddJobListTimeRange slot.

#include <cstdint>

struct idColor;
class idParallelJobList;
class idDebugGraph;
class idRenderModelGui;
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
  RANGE_STREAM_CT = 13,
  RANGE_STREAM_HD = 14,
  RANGE_STREAM_OD = 15,
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

  void Init() {
    using Fn=void(*)(idConsole*);
    reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[1])(this);
  }

  void LoadGraphics() {
    using Fn=void(*)(idConsole*);
    reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[4])(this);
  }

  void ClearNotifyLines() {
    using Fn=void(*)(idConsole*);
    reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[8])(this);
  }

  bool IsActive() {
    using Fn=bool(*)(idConsole*);
    return reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[7])(this);
  }
  bool IsInitialized() {
    using Fn=bool(*)(idConsole*);
    const auto slots=reinterpret_cast<const void* const*>(vtable);
    return reinterpret_cast<Fn>(slots[6])(this);
  }
  bool ProcessEvent(const struct sysEvent_t* event,bool forceAccept) {
    using Fn=bool(*)(idConsole*,const sysEvent_t*,bool);
    return reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[5])(
        this,event,forceAccept);
  }

  void InitGuiModel() {
    using Fn=void(*)(idConsole*);
    reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[3])(this);
  }

  void AddTimeRange(rangeId_t range, const idColor* color,
                    std::uint64_t start, std::uint64_t end) {
    vtable->AddTimeRange(this, range, color, start, end);
  }

  void SetTimeRangeLabel(rangeId_t range,const char* label) {
    using Fn=void(*)(idConsole*,rangeId_t,const char*);
    reinterpret_cast<Fn>(reinterpret_cast<const void* const*>(vtable)[18])(
        this,range,label);
  }

  void AddJobListTimeRange(rangeId_t range, const idColor* color,
                           const idParallelJobList* jobList,
                           std::uint64_t offset) {
    vtable->AddJobListTimeRange(this, range, color, jobList, offset);
  }

  void BeginTimeRange(rangeId_t range, const idColor* color) {
    vtable->BeginTimeRange(this, range, color);
  }

  idRenderModelGui* Gui() {
    using Fn=idRenderModelGui*(*)(idConsole*);
    const auto slots=reinterpret_cast<const void* const*>(vtable);
    return reinterpret_cast<Fn>(slots[15])(this);
  }

  idDebugGraph* CreateGraph(int count) {
    using Fn=idDebugGraph*(*)(idConsole*,int);
    const auto slots=reinterpret_cast<const void* const*>(vtable);
    return reinterpret_cast<Fn>(slots[22])(this,count);
  }

  void DestroyGraph(idDebugGraph* graph) {
    using Fn = void (*)(idConsole*, idDebugGraph*);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[23])(this, graph);
  }

  void AddSnapObject(const char* name,int size,int changed,float fraction) {
    using Fn=void(*)(idConsole*,const char*,int,int,float);
    const auto slots=reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[26])(this,name,size,changed,fraction);
  }

  void ClearSnapshots() {
    using Fn=void(*)(idConsole*);
    const auto slots=reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[27])(this);
  }

  void Close(bool force) {
    using Fn = void (*)(idConsole*, bool);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[10])(this, force);
  }

  void ClearEditLine() {
    using Fn = void (*)(idConsole*);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[9])(this);
  }

  void Shutdown() {
    using Fn = void (*)(idConsole*);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[2])(this);
  }

  void Dump(const char* fileName, idStr* text = nullptr) {
    using Fn = void (*)(idConsole*, const char*, idStr*);
    const auto slots = reinterpret_cast<const void* const*>(vtable);
    reinterpret_cast<Fn>(slots[12])(this, fileName, text);
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
