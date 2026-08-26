#pragma once

// NEEDS_WORK(engine/framework): this is the PDB-derived renderer boundary.
// The implementation and full layout belong to engine/framework and are
// intentionally deferred until that folder is recovered.

#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/sys/sys_alloc.h"

#include <cstdint>

class idStreamFileCache {
public:
  bool OutOfMemoryCallback();
  void UncachedBackgroundRead(idFile* file, std::int64_t offset, int length,
                              void** destination, memTag_t tag,
                              volatile bool* completionFlag);
  void SignalWork();
};

extern idStreamFileCache* streamFileCache;
