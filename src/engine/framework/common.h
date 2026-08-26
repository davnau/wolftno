#pragma once

// NEEDS_WORK(engine/framework): ABI-only idCommon boundary. These helpers use
// the retail vtable slots required by recovered clients; the owning framework
// compiland must provide the full interface and implementations.
class idRenderVideoOverlay;
class idPlayerProfile;

struct idCommon {
  const void* const* vtable;

  void BeginTerminationThread() {
    using Fn = void (*)(idCommon*);
    reinterpret_cast<Fn>(vtable[5])(this);
  }
  void EndTerminationThread() {
    using Fn = void (*)(idCommon*);
    reinterpret_cast<Fn>(vtable[6])(this);
  }
  bool IsTerminationThreadStopped() const {
    using Fn = bool (*)(const idCommon*);
    return reinterpret_cast<Fn>(vtable[7])(this);
  }
  void SetRefreshOnPrint(bool refresh) {
    using Fn = void (*)(idCommon*, bool);
    reinterpret_cast<Fn>(vtable[17])(this, refresh);
  }
  void UpdateConsoleDisplay(bool force) {
    using Fn = void (*)(idCommon*, bool);
    reinterpret_cast<Fn>(vtable[20])(this, force);
  }
  idPlayerProfile* GetProfileFromMasterLocalUser() {
    using Fn = idPlayerProfile* (*)(idCommon*);
    return reinterpret_cast<Fn>(vtable[25])(this);
  }
  bool IsClient() const {
    using Fn = bool (*)(const idCommon*);
    return reinterpret_cast<Fn>(vtable[42])(this);
  }
  bool IsInGame() const {
    using Fn = bool (*)(const idCommon*);
    return reinterpret_cast<Fn>(vtable[35])(this);
  }
  bool IsToolActive() const {
    using Fn = bool (*)(const idCommon*);
    return reinterpret_cast<Fn>(vtable[45])(this);
  }
  bool IsDialogActive() const {
    using Fn = bool (*)(const idCommon*);
    return reinterpret_cast<Fn>(vtable[54])(this);
  }
  int GetFocusDeviceNum() const {
    using Fn = int (*)(const idCommon*);
    return reinterpret_cast<Fn>(vtable[57])(this);
  }
  idRenderVideoOverlay* VideoOverlay() {
    using Fn = idRenderVideoOverlay* (*)(idCommon*);
    return reinterpret_cast<Fn>(vtable[90])(this);
  }
};

extern idCommon* common;

static_assert(sizeof(idCommon) == 8, "common ABI drift");
