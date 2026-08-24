#pragma once

// NEEDS WORK: engine/renderer owns the complete interface and implementation.
class idRenderSystem {
public:
  virtual ~idRenderSystem() = default;
  virtual int GetWidth() = 0;
  virtual int GetHeight() = 0;
  virtual float GetPixelAspect() = 0;
};
extern idRenderSystem* renderSystem;
