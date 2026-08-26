#pragma once

#include "engine/cm/collisionmodel.h"
#include "shared/idlib/containers/list.h"

#include <cstdint>

class idClipQuery {
public:
  idClipQuery() = default;
  explicit idClipQuery(std::uint64_t queryIndex) : index(queryIndex) {}

  std::uint64_t index;
};

class idResidencyQuery {
public:
  static constexpr std::uint64_t INVALID_INDEX = 0;

  idResidencyQuery() : index(INVALID_INDEX) {}
  explicit idResidencyQuery(std::uint64_t queryIndex) : index(queryIndex) {}
  void Cancel() { index = INVALID_INDEX; }

  std::uint64_t index;
};

class idClip;
class idTraceModelCache;
class idMaterial;
class idTypeInfoTools;
struct contactsResult_t;

class idCollisionQuery {
public:
  idCollisionQuery() : offset(0) {}
  idCollisionQuery(const idCollisionQuery& other) : offset(other.offset) {}

  std::uint64_t offset;

  static const std::uint64_t finalResultsFirstSubmittedOffset;
  static const std::uint64_t finalResultsLastSubmittedOffset;
};

class idClipModel {
public:
  idClipModel(idClip* clip_, const idTraceModel* traceModels_,
              int numTraceModels, const idMaterial* material);
  int GetNumCollisionModels() const;
  idCollisionModel* GetCollisionModel(int index) const;
  void Delete();
  void Link(int entityNumber, int physicsId, int bodyId,
            const idVec3& origin, const idMat3& axis);
  void Unlink();
  void SetContents(int newContents);
  void SetLinkedPosition(const idVec3& linkedOrigin,
                         const idMat3& linkedAxis) {
    origin = linkedOrigin;
    axis = linkedAxis;
  }

  const void* const* vtable;
  std::uint32_t flags;
  int physicsId;
  int bodyId;
  int bodyMask;
  int touchCount;
  int contents;
  idVec3 origin;
  idMat3 axis;
  idBounds absBounds;
  idBounds bounds;
  idCollisionModel* collisionModel;
  idList<int, 13> traceModels;
  idList<idClipModel*, 13> children;
  void* sphereFeatures;
  idClip* clip;
  void* clipLinks;
  idCollisionQuery collisionQuery;
  idClipModel* nextDeleted;
  std::uint8_t aasLinks[0x58];

private:
  idCollisionModel* NoTraceModelWarning() const;

  friend class idBuoyancy;
  friend class idEffectPhysicsBroadPhase;
};

class idClip {
public:
  struct idClipCollisionQuery {
    int type;
    int parmIndex;
    const idClipModel* clipModel;
    idCollisionQuery query;
  };

  struct idClipResidencyQuery {
    std::uint8_t recoveredPrefix[0x18];
    bool resident;
    std::uint8_t recoveredTail[7];
  };

  idClipQuery Translation(trace_t* result, const idVec3& start,
                          const idVec3& end, const idClipModel* clipModel,
                          const idMat3& startAxis, int clipMask,
                          int passEntityNumber, bool moveClipModel,
                          const char* userName);
  idClipQuery Rotation(trace_t* result, const idVec3& start,
                       const idRotation& rotation,
                       const idClipModel* clipModel,
                       const idMat3& startAxis, int clipMask,
                       int passEntityNumber, bool moveClipModel,
                       const char* userName);
  void TranslationModel(trace_t& result, const idVec3& start,
                        const idVec3& end, const idClipModel* clipModel,
                        const idMat3& startAxis, int clipMask,
                        const idClipModel* model, const idVec3& modelOrigin,
                        const idMat3& modelAxis,
                        const char* userName) const;
  void RotationModel(trace_t& result, const idVec3& start,
                     const idRotation& rotation,
                     const idClipModel* clipModel,
                     const idMat3& startAxis, int clipMask,
                     const idClipModel* model, const idVec3& modelOrigin,
                     const idMat3& modelAxis,
                     const char* userName) const;
  idClipQuery Contents(trace_t* result, const idVec3& start,
                       const idClipModel* clipModel,
                       const idMat3& startAxis, int clipMask,
                       int passEntityNumber, const char* userName);
  void ContentsModel(trace_t& result, const idVec3& start,
                     const idClipModel* clipModel,
                     const idMat3& startAxis, int clipMask,
                     const idClipModel* model, const idVec3& modelOrigin,
                     const idMat3& modelAxis,
                     const char* userName) const;
  idClipQuery StepMoveContacts(
      trace_t* result, contactsResult_t* contacts, const idVec3& start,
      const idVec3& end, const idVec3& downNormal, float stepUp,
      float stepDown, const idClipModel* clipModel,
      const idMat3& startAxis, int clipMask, int passEntityNumber,
      bool moveClipModel, const char* userName);
  void ShowCollisionPoint(const contactInfo_t& contact,
                          const idTypeInfoTools* tools);

  bool TracePoint(trace_t& result, const idVec3& start, const idVec3& end,
                  int clipMask, int passEntityNumber, const char* userName);
  int GetClipModelsTouchingBounds(const idBounds& bounds, int clipMask,
                                  idClipModel** clipModelList,
                                  int maxCount) const;
  idResidencyQuery TestResidency(const idBounds& bounds);

private:
  int GetPositionedModelsInternal(
      const idVec3& boundsMin, const idVec3& boundsMax, bool isLine,
      int clipMask, int passEntityNumber, int passOwnerNumber, int physicsId,
      int bodyMask, class idPositionedCollisionModel* models,
      int maxModels) const;

  friend class idEffectPhysicsBroadPhase;

public:
  idClipModel* world;
  idClipModel* temporaryClipModel;
  idClipModel* defaultClipModel;
  idClipModel* playerMeleeClipModel;
  idClipModel* clip8x8;
  idClipModel* clip8x8x8;
  idClipModel* clip16x16;
  idClipModel* clip16x16x16;
  idClipModel* clip24x24;
  idClipModel* clip32x32;
  idClipModel* clip8x16;
  idClipModel* clip48x48;
  idClipModel* clip96x96;
  idClipModel* clip24x24x96;
  idClipModel* clip32x32x96;
  idClipModel* clip48x48x96;
  std::uint8_t recoveredPrefix[0x40078];
  idTraceModelCache* traceModelCache;
  std::uint8_t recoveredQueryPrefix[0x1C188];
  idClipCollisionQuery collisionQueries[4096];
  std::uint64_t collisionQueryFirstSubmittedIndex;
  std::uint64_t collisionQueryLastSubmittedIndex;
  std::uint64_t collisionQueryFirstAllocedIndex;
  std::uint64_t collisionQueryLastAllocedIndex;
  std::uint64_t collisionQueryLastResolvedIndex;
  std::uint8_t recoveredResidencyPrefix[0xD040];
  idClipResidencyQuery residencyQueries[1024];
  std::uint64_t residencyQueryFirstSubmittedIndex;
  std::uint64_t residencyQueryLastSubmittedIndex;
  std::uint64_t residencyQueryFirstAllocedIndex;
  std::uint64_t residencyQueryLastAllocedIndex;
};

static_assert(sizeof(idClipQuery) == 8, "idClipQuery ABI drift");
static_assert(sizeof(idResidencyQuery) == 8,
              "idResidencyQuery ABI drift");
static_assert(sizeof(idCollisionQuery) == 8,
              "idCollisionQuery ABI drift");
static_assert(sizeof(idClip::idClipCollisionQuery) == 0x18,
              "idClip collision-query ABI drift");
static_assert(sizeof(idClip::idClipResidencyQuery) == 0x20,
              "idClip residency-query ABI drift");
static_assert(sizeof(idClipModel) == 0x138, "idClipModel ABI drift");
static_assert(offsetof(idClipModel, physicsId) == 0x0C,
              "idClipModel physics offset drift");
static_assert(offsetof(idClipModel, origin) == 0x20,
              "idClipModel origin offset drift");
static_assert(offsetof(idClipModel, collisionModel) == 0x80,
              "idClipModel collision-model offset drift");
static_assert(offsetof(idClipModel, traceModels) == 0x88,
              "idClipModel trace-model offset drift");
static_assert(offsetof(idClipModel, clip) == 0xC0,
              "idClipModel owner offset drift");
static_assert(offsetof(idClip, defaultClipModel) == 0x10,
              "idClip default-model offset drift");
static_assert(offsetof(idClip, traceModelCache) == 0x400F8,
              "idClip trace-model cache offset drift");
static_assert(offsetof(idClip, collisionQueries) == 0x5C288,
              "idClip collision-query array offset drift");
static_assert(offsetof(idClip, collisionQueryFirstSubmittedIndex) == 0x74288,
              "idClip first-submitted offset drift");
static_assert(offsetof(idClip, collisionQueryLastSubmittedIndex) == 0x74290,
              "idClip last-submitted offset drift");
static_assert(offsetof(idClip, collisionQueryFirstAllocedIndex) == 0x74298,
              "idClip first-allocated offset drift");
static_assert(offsetof(idClip, collisionQueryLastAllocedIndex) == 0x742A0,
              "idClip last-allocated offset drift");
static_assert(offsetof(idClip, collisionQueryLastResolvedIndex) == 0x742A8,
              "idClip last-resolved offset drift");
static_assert(offsetof(idClip, residencyQueries) == 0x812F0,
              "idClip residency-query array offset drift");
static_assert(offsetof(idClip::idClipResidencyQuery, resident) == 0x18,
              "idClip residency result offset drift");
static_assert(offsetof(idClip, residencyQueryFirstSubmittedIndex) == 0x892F0,
              "idClip residency first-submitted offset drift");
static_assert(offsetof(idClip, residencyQueryLastSubmittedIndex) == 0x892F8,
              "idClip residency last-submitted offset drift");
static_assert(sizeof(idClip) == 0x89310, "idClip ABI drift");
