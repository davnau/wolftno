#pragma once

// NEEDS_WORK(engine/framework): declaration-only boundary. Video-name remap
// policy belongs to the version-selector implementation, not renderer.
class idVersionSelector {
public:
  static const char* RemapVideoName(char* buffer, int bufferSize,
                                    const char* name);
};
