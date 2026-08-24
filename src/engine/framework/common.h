#pragma once

// NEEDS_WORK(engine/framework): SWF rendering only needs this PDB-derived
// portion of idCommon. The complete interface and its exact vtable ordering
// belong to the framework recovery pass.
class idCommon {
public:
  virtual ~idCommon() = default;
  virtual bool IsClient() const = 0;
  virtual int GetFocusDeviceNum() const = 0;
  virtual bool IsTerminationThreadStopped() const = 0;
  virtual void BeginTerminationThread() = 0;
  virtual void EndTerminationThread() = 0;
  virtual bool IsDialogActive() const = 0;
};

extern idCommon* common;
