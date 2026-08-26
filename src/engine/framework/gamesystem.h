#pragma once

class idGameSystem {
public:
  const void* const* vtable;

  void Init() {
    using Fn=void(*)(idGameSystem*);
    reinterpret_cast<Fn>(vtable[1])(this);
  }

  void LoadStaticResources() {
    using Fn=void(*)(idGameSystem*);
    reinterpret_cast<Fn>(vtable[3])(this);
  }

  void SetMemoryFiles() {
    using Fn = void (*)(idGameSystem*);
    reinterpret_cast<Fn>(vtable[9])(this);
  }

  void Shutdown() {
    using Fn=void(*)(idGameSystem*);
    reinterpret_cast<Fn>(vtable[2])(this);
  }

  void FreeGame(class idGame*& game) {
    using Fn=void(*)(idGameSystem*,idGame*&);
    reinterpret_cast<Fn>(vtable[7])(this,game);
  }
};

extern idGameSystem* gameSystem;

static_assert(sizeof(idGameSystem) == 8, "game-system ABI drift");
