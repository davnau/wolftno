#pragma once

#include <cstdint>

// Renderer/framework boundary required by font loading. NEEDS WORK: the full
// resource manager interface is owned by engine/framework.
class idResourceManager {
public:
  virtual ~idResourceManager() = default;
  virtual bool ExcludeFont(const char* filename) = 0;
  virtual unsigned GetBuildPlatforms() const { return 0; }
  virtual void SetFileHook() {}
  virtual void ReleaseFileHook() {}
  virtual bool IsResourceFile() const { return false; }
  virtual unsigned GetResourceFilePlatform() const { return 0; }
  virtual void SetResourceFilePlatform(unsigned) {}
  virtual bool GetCacheFileInfo(const char*, std::int64_t*, std::uint64_t*,
                                std::uint64_t*, std::uint32_t*, std::uint8_t*,
                                bool) { return false; }
  virtual class idFile* GetCacheFile(std::uint8_t) { return nullptr; }
};

extern idResourceManager* resourceManager;
