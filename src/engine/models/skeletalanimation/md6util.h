#pragma once

#include "engine/decls/animwebs/declanimweb.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>

struct idJointMat {
  float mat[12];
  void Invert();
};

class idMD6Skel {
public:
  int GetJointIndex(const char* jointName) const;
  const void* data = nullptr;
  idList<idAtomicString, 5> jointNames;
  idAtomicString name;
};

using idMD6UtilAnimWebLookup = const idDeclAnimWeb* (*)(const char*);
using idMD6UtilDeclLookup = const idDeclMD6* (*)(const char*);
using idMD6UtilDecodeFrame = void (*)(const idMD6Model*, const idMD6Anim*,
                                      float, float, idJointMat*, std::size_t);
void SetMD6UtilAnimWebLookup(idMD6UtilAnimWebLookup lookup);
void SetMD6UtilDeclLookup(idMD6UtilDeclLookup lookup);
void SetMD6UtilDecodeFrame(idMD6UtilDecodeFrame decode);

class idMD6Util {
public:
  struct animDelta_t { idVec3 deltaTranslation; idMat3 deltaAxis; };
  class idJointCache {
  public:
    idJointCache(idAnimWebPath* webPath, int frameNum);
    bool GetDelta(animDelta_t* delta, const char* jointName,
                  const char* refJointName);
    void UpdateCache();
    void Set(const idDeclAnimWeb* webRef, idAnimWebPath* webPath, int frameNum);

    idJointMat jointMatrices[256];
    const idDeclAnimWeb* webRef;
    const idDeclMD6* md6Decl;
    const idMD6Anim* md6anim;
    int frameNum;
    bool validJointCache;
  };

  static const idMD6Anim* FindMD6Anim(
      const idDeclAnimWeb* webRef, idAnimWebPath* webPath,
      const idDeclMD6* declMD6, animWebModelIndex_t modelIndex,
      bool loadAnim);
};

static_assert(sizeof(idJointMat) == 48, "idJointMat ABI drift");
static_assert(sizeof(idMD6Util::animDelta_t) == 48, "animDelta_t ABI drift");
static_assert(sizeof(idMD6Util::idJointCache) == 12320, "idJointCache ABI drift");
static_assert(offsetof(idMD6Util::idJointCache, webRef) == 12288, "webRef ABI drift");
static_assert(offsetof(idMD6Util::idJointCache, md6Decl) == 12296, "md6Decl ABI drift");
static_assert(offsetof(idMD6Util::idJointCache, md6anim) == 12304, "md6anim ABI drift");
static_assert(offsetof(idMD6Util::idJointCache, frameNum) == 12312, "frameNum ABI drift");
static_assert(offsetof(idMD6Util::idJointCache, validJointCache) == 12316,
              "validJointCache ABI drift");
