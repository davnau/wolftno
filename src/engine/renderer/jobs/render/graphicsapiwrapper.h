#pragma once

#include <cstdint>

class idDeclRenderProg;
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
void GL_DrawElements(const idODSObject<idDeclRenderProg>& program,
                     const idODSObject<idTriangles>& triangles,
                     std::uint64_t extraState, bool skipDetailTriangles);

static_assert(sizeof(idODSObject<void>) == sizeof(void*),
              "ODS object ABI drift");
