#pragma once

#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/tracemodel.h"
#include "shared/idlib/math/plane.h"
#include "shared/idlib/text/str.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

enum cmType_t { CM_TYPE_POLYGON, CM_TYPE_SPHERE, CM_TYPE_GRID };

enum cmContents_t : std::uint32_t {
  CONTENTS_NONE = 0,
  CONTENTS_SOLID = 1u << 0,
  CONTENTS_BODY = 1u << 1,
  CONTENTS_TRIGGER = 1u << 2,
  CONTENTS_WATER = 1u << 3,
  CONTENTS_ALL = 0xFFFFFFFFu
};

struct cmEdge_t {
  std::uint32_t vertex[2]{};
  idVec3 normal{};
};

struct cmPolygon_t {
  idPlane plane{};
  idBounds bounds{};
  std::vector<std::uint32_t> vertices;
  std::uint32_t contents{CONTENTS_SOLID};
  int material{};
};

struct cmPolytope_t {
  std::vector<idPlane> planes;
  idBounds bounds{};
  std::uint32_t contents{CONTENTS_SOLID};
};

struct cmModelStats_t {
  std::size_t vertices{};
  std::size_t edges{};
  std::size_t polygons{};
  std::size_t polytopes{};
  std::size_t bytes{};
};

struct contactInfo_t {
  idVec3 point{};
  idVec3 normal{};
  float distance{};
  std::uint32_t contents{};
  int material{};
  int modelFeature{-1};
  int traceFeature{-1};
  int entityNum{-1};
};

struct trace_t {
  float fraction{1.0f};
  idVec3 endpos{};
  idMat3 endAxis{mat3_identity};
  contactInfo_t contact{};
  bool startSolid{};
  bool allSolid{};
};

class idCollisionModel {
public:
  virtual ~idCollisionModel();
  virtual const char* GetName() const = 0;
  virtual cmType_t GetType() const = 0;
  virtual const idBounds& GetBounds() const = 0;
  virtual std::uint32_t GetContents() const = 0;
  virtual cmModelStats_t GetPolytopeStats() const = 0;
};

class idCollisionModelLocal final : public idCollisionModel {
public:
  idCollisionModelLocal();

  const char* GetName() const override { return name.data; }
  cmType_t GetType() const override { return type; }
  const idBounds& GetBounds() const override;
  std::uint32_t GetContents() const override;
  cmModelStats_t GetPolytopeStats() const override;

  const idVec3* GetVertex(std::size_t index) const;
  const cmEdge_t* GetEdge(std::size_t index) const;
  const cmPolygon_t* GetPolygon(std::size_t index) const;
  const cmPolytope_t* GetPolytope(std::size_t index) const;
  std::size_t GetPolytopes(const idBounds& query, const cmPolytope_t** output,
                           std::size_t capacity) const;
  std::size_t GetTotalMemory() const;
  std::size_t GetLoadedMemory() const;
  std::size_t GetMaxResidentMemory() const;
  idBounds GetBox() const;
  int GetJoint(std::size_t index) const;
  static const std::vector<idCollisionModelLocal*>& GetResourceList();
  static bool ValidateStreamFileHeader(const void* data, std::size_t size);

  void MakeDefault();
  void FreeData();
  bool Write_Binary(const char* path) const;
  bool Load_Binary(const char* path);
  bool ReloadIfStale();
  bool LoadResource(const char* path);

  idStr name{"default"};
  cmType_t type{CM_TYPE_POLYGON};
  idBounds bounds{};
  std::vector<idVec3> vertices;
  std::vector<cmEdge_t> edges;
  std::vector<cmPolygon_t> polygons;
  std::vector<cmPolytope_t> polytopes;
  float sphereRadius{};
};

bool CM_BoundsIntersect(const idBounds& first, const idBounds& second);
idBounds CM_TranslatedBounds(const idBounds& bounds, const idVec3& origin);
idPlane CM_GetPolygonPlane(const cmPolygon_t& polygon);

struct cmStreamArea_t {
  const std::uint8_t* data{};
  std::size_t size{};
};

bool SetupStreamAreaPtrs(const void* data, std::size_t size, std::vector<cmStreamArea_t>& areas);
std::shared_ptr<std::vector<std::uint8_t>> AcquireSubModelData(const void* data, std::size_t size);
void ReleaseSubModelData(std::shared_ptr<std::vector<std::uint8_t>>& data);
void SwapModelTreeNodes(std::vector<std::uint32_t>& nodes);
void SwapSubModelHeader(cmModelStats_t& header);
void SwapSubModelData(idCollisionModelLocal& model);
void SwapStreamAreas(std::vector<cmStreamArea_t>& areas);
void SwapBigSphereModelStruct(idCollisionModelLocal& model);
void SwapBigSphereModelData(std::vector<float>& sphereData);
