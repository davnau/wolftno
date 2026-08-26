#pragma once

// NEEDS_WORK(engine/framework/session): PDB-exact interface slice required by
// renderer/renderthread.cpp.  The full session implementation remains owned by
// the framework/session recovery pass.

class idSaveGameManager;

struct idSession {
  const void* const* vtable;

  idSaveGameManager* GetSaveGameManager() {
    using Fn = idSaveGameManager* (*)(idSession*);
    return reinterpret_cast<Fn>(vtable[146])(this);
  }
};

extern idSession* session;

static_assert(sizeof(idSession) == 8, "session ABI drift");
