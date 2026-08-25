#pragma once

#include "engine/decls/decl.h"
#include "engine/renderer/parmblock.h"
#include "shared/idlib/containers/staticlist.h"

#include <cstddef>
#include <cstdint>

class idImage;
class idTextureSampler;

struct renderProgData_t {
  idStr inheritsFrom;
  unsigned int compoundCheckSum;
  unsigned int parentCheckSum;
  idList<idStr> dependencies;
  idList<unsigned int> dependencyTimestamps;
  std::uint64_t glState;
  idParmBlock parmBlock;
  idStaticList<short, 42> vertexParms;
  idStaticList<short, 42> fragmentParms;
  idStaticList<short, 16> textureParms;
  idStaticList<short, 16> samplerParms;
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
  unsigned int vertexMask;
  std::uint16_t fragmentOutputs;
  std::uint8_t registerCountPS3;
  std::uint8_t registerCount360;
  idStr vertexCode;
  idStr fragmentCode;
};

template <typename T, std::size_t Count> struct idDeclFixedArray {
  T ptr[Count];
};

class idDeclRenderProg : public idDecl {
public:
  static void BindTexture(int textureUnit, const idImage* image,
                          const idTextureSampler* sampler);

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
static_assert(offsetof(renderProgData_t, vertexParms) == 0x98,
              "render-program vertex-parm offset drift");
static_assert(offsetof(renderProgData_t, samplerParms) == 0x1B0,
              "render-program sampler-parm offset drift");
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
