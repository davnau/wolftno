#pragma once

#include "engine/models/detail/jobs/detailgather.h"
#include "engine/models/foliage/jobs/foliagedata.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/geometry/drawvert.h"

#include <cstddef>
#include <cstdint>

struct dm_detail_t {
  std::int16_t xyz[3];
  std::uint8_t attribIdx;
  std::uint8_t uniformSizeScale;
  std::uint8_t color[3];
  std::uint8_t pad[9];
  idVec3 rotation;
};

struct dm_attribute_t {
  std::uint16_t declIdx;
  std::uint16_t paletteIdx;
  std::uint8_t pad[28];
};

struct idDetailPaletteData {
  const void* material;
  int numSurfaces;
  int pad;
  const void* surfaces;
  idBounds bounds;
};

struct detailPaletteEntry_t {
  std::uint16_t vertOffset;
  std::uint16_t indexOffset;
  std::uint16_t numVerts;
  std::uint16_t numIndices;
};

struct detailPaletteVert_t {
  std::int16_t xyz[3];
  std::uint16_t st[2];
};

struct detailPalette_t {
  const detailPaletteEntry_t* entry;
  const detailPaletteVert_t* verts;
  const std::uint16_t* indices;
};

struct detailGenTotals_t {
  int totalVerts;
  int totalIndices;
  int pad[2];
};

struct deferredDetailGenParms_t {
  const detailRenderView_t* viewData;
  const dm_subTreeBounds_t* subTreeBounds;
  const dm_subTreeData_t* subTreeData;
  int subTreeDataSize;
  int pad0;
  const std::uint16_t* detailIndices;
  int maxDetailIndices;
  int numJobsForIndices;
  int jobIndexForIndices;
  int pad1;
  const detailSubTreeStats_t* detailCountsList;
  int detailCountsIndex;
  int pad2;
  const detailPaletteEntry_t* paletteEntries;
  int paletteEntriesSize;
  int pad3;
  const detailPaletteVert_t* paletteVerts;
  int paletteVertsSize;
  int pad4;
  const std::uint16_t* paletteIndices;
  int paletteIndicesSize;
  int numVertsAllocated;
  idDrawVert* verts;
  std::uint16_t* indices;
  detailGenTotals_t* totalsUsed;
};

void CalculateDetailBounds(const dm_detail_t* detail,
                           const idDetailPaletteData* data,
                           idBounds* bounds);
void GetDetailTotalCount(const detailSubTreeStats_t* detailCountsList,
                         detailGenTotals_t* totalsCount);
void GetDetailOffsetsForSubTree(
    const detailSubTreeStats_t* detailCountsList, int subTreeIndex,
    int* vertCount, int* indexCount);
void BuildDetailModel(const detailPalette_t* palette, idDrawVert* verts,
                      std::uint16_t* indices, int vertOffset,
                      const dm_detail_t* detail,
                      const dm_attribute_t* attribute,
                      detailGenTotals_t* totals);
std::uint32_t GenDetailForSubTree(
    const deferredDetailGenParms_t* parms,
    const std::uint16_t* detailIndices, std::uint32_t numDetail,
    int vertexBufferOffset, int indexBufferOffset, int indexOffset,
    detailGenTotals_t* totals);

static_assert(sizeof(dm_detail_t) == 32, "dm_detail_t ABI drift");
static_assert(offsetof(dm_detail_t, rotation) == 20,
              "dm_detail_t::rotation ABI drift");
static_assert(sizeof(dm_attribute_t) == 32, "dm_attribute_t ABI drift");
static_assert(sizeof(idDetailPaletteData) == 48,
              "idDetailPaletteData ABI drift");
static_assert(offsetof(idDetailPaletteData, bounds) == 24,
              "idDetailPaletteData::bounds ABI drift");
static_assert(sizeof(detailPaletteEntry_t) == 8,
              "detailPaletteEntry_t ABI drift");
static_assert(sizeof(detailPaletteVert_t) == 10,
              "detailPaletteVert_t ABI drift");
static_assert(sizeof(detailPalette_t) == 24,
              "detailPalette_t ABI drift");
static_assert(sizeof(detailGenTotals_t) == 16,
              "detailGenTotals_t ABI drift");
static_assert(sizeof(deferredDetailGenParms_t) == 144,
              "deferredDetailGenParms_t ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, detailIndices) == 32,
              "deferredDetailGenParms_t::detailIndices ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, detailCountsList) == 56,
              "deferredDetailGenParms_t::detailCountsList ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, paletteEntries) == 72,
              "deferredDetailGenParms_t::paletteEntries ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, paletteVerts) == 88,
              "deferredDetailGenParms_t::paletteVerts ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, paletteIndices) == 104,
              "deferredDetailGenParms_t::paletteIndices ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, verts) == 120,
              "deferredDetailGenParms_t::verts ABI drift");
static_assert(offsetof(deferredDetailGenParms_t, totalsUsed) == 136,
              "deferredDetailGenParms_t::totalsUsed ABI drift");

