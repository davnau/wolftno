#pragma once

// NEEDS_WORK(engine/framework): ABI-only interface recovered for declaration
// dependencies. Implement code-resource registration when this folder is
// recovered; do not treat this header as a completed framework module.

class idResource;
class idResourceList;

class idBaseCodeResource {
public:
  const char* name = nullptr;
  idBaseCodeResource* next = nullptr;
  idResource* r = nullptr;
  idResourceList* rl = nullptr;

  inline static idBaseCodeResource* codeResourceList = nullptr;
};

template <typename T> class idCodeResource : public idBaseCodeResource {
public:
  explicit idCodeResource(const char* resourceName = nullptr) {
    name = resourceName;
    next = codeResourceList;
    codeResourceList = this;
  }
  const T* Get() const { return static_cast<const T*>(r); }
  const T* operator->() const { return Get(); }
  operator const T*() const { return Get(); }
};

static_assert(sizeof(idBaseCodeResource) == 32, "idBaseCodeResource ABI drift");
