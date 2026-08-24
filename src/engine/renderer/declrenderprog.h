#pragma once

#include "engine/decls/decl.h"
#include "engine/renderer/parmblock.h"

#include <cstddef>
#include <cstdint>

struct renderProgData_t {
  std::uint8_t opaqueBeforeGLState[0x60];
  std::uint64_t glState;
  idParmBlock parmBlock;
  std::uint8_t opaqueBeforeFlags[0x150];
  int flags;
  float stageSort;
  bool hasInteractions;
  bool hasAlphaToCoverage;
  bool has16BitScaleBias;
  bool hasHardwareSkinning;
  bool hasVertexTexture;
  bool hasClip;
  bool hasDepthOutput;
  bool hasBranchStripping;
  std::uint8_t opaqueTail[0x58];
};

template <typename T, std::size_t Count> struct idDeclFixedArray {
  T ptr[Count];
};

class idDeclRenderProg : public idDecl {
public:
  const idDeclRenderProg* renamed;
  renderProgData_t data;
  const idDeclRenderProg* inheritsFromDecl;
  unsigned int vertexProgramChecksum;
  unsigned int fragmentProgramChecksum;
  bool hasDerived;
  bool partiallyLoaded;
  std::uint8_t alignmentBeforeVersions[6];
  idDeclFixedArray<const idDeclRenderProg*, 3> versions;
  void* vertexProgram;
  void* fragmentProgram;
  idDeclFixedArray<void*, 45> vertexUniforms;
  idDeclFixedArray<void*, 42> fragmentUniforms;
};

static_assert(offsetof(renderProgData_t, glState) == 0x60,
              "render-program GL-state offset drift");
static_assert(offsetof(renderProgData_t, parmBlock) == 0x68,
              "render-program parm-block offset drift");
static_assert(offsetof(renderProgData_t, flags) == 0x1E8,
              "render-program flags offset drift");
static_assert(offsetof(renderProgData_t, stageSort) == 0x1EC,
              "render-program stage-sort offset drift");
static_assert(sizeof(renderProgData_t) == 0x250,
              "render-program data ABI drift");
static_assert(offsetof(idDeclRenderProg, renamed) == 0x50,
              "render-program rename offset drift");
static_assert(offsetof(idDeclRenderProg, data) == 0x58,
              "render-program data offset drift");
static_assert(offsetof(idDeclRenderProg, versions) == 0x2C0,
              "render-program versions offset drift");
static_assert(sizeof(idDeclRenderProg) == 0x5A0,
              "render-program ABI drift");
