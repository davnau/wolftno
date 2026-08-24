#pragma once

#include <cstdint>

class idDeclRenderProg;
class idImage;
struct idRenderModelSurface;
class idTextureSampler;
struct idRenderDestination;
class idTriangles;

template <typename T> struct idODSObject {
  const T* objectPtr;
};

void GL_SetDefaultState();
void GL_SetRenderDestination(
    const idODSObject<idRenderDestination>& destination, int level, int side);
void GL_Viewport(int x, int y, int width, int height);
void GL_Scissor(int x, int y, int width, int height);
void GL_Clear(bool color, bool depth, bool stencil, unsigned char stencilValue,
              float red, float green, float blue, float alpha);
int GL_GetDeferredQueryResult(std::int64_t queryNumber);
void GL_Flush();
void GL_BeginQueryBatch();
void GL_DepthBoundsTest(float minimum, float maximum);
void GL_State(std::uint64_t stateBits, bool forceState);
void GL_PolygonOffset(float scale, float bias, bool fill);
void GL_BeginQuery(std::int64_t* queryNumber);
void GL_EndQuery();
void GL_BindTexture(int textureUnit, const idImage* image,
                    const idTextureSampler* sampler);
void GL_ResetTextureState();
void GL_DrawElements(const idODSObject<idDeclRenderProg>& program,
                     const idODSObject<idTriangles>& triangles,
                     std::uint64_t extraState, bool skipDetailTriangles);
void GL_DrawElements(const idODSObject<idDeclRenderProg>& program,
                     const idODSObject<idRenderModelSurface>& surface,
                     std::uint64_t extraState, bool skipDetailTriangles);

static_assert(sizeof(idODSObject<void>) == sizeof(void*),
              "ODS object ABI drift");
