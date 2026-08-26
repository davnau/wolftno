#pragma once

class idGameSystem {
public:
  const void* const* vtable;

  void SetMemoryFiles() {
    using Fn = void (*)(idGameSystem*);
    reinterpret_cast<Fn>(vtable[9])(this);
  }
};

extern idGameSystem* gameSystem;

static_assert(sizeof(idGameSystem) == 8, "game-system ABI drift");
