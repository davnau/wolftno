#pragma once

#include <cstddef>

class idImage;

class idRenderDestination {
public:
  void Resize(int width, int height);

  bool isDefault;
  unsigned char alignmentPadding[3];
  int targetWidth;
  int targetHeight;
  unsigned char pointerAlignmentPadding[4];
  idImage* targetImage[4];
  idImage* depthImage;
  idImage* stencilImage;
  unsigned int fboHandle;
  int boundSide;
  int boundLevel;
  unsigned int tailPadding;
};

static_assert(offsetof(idRenderDestination, targetWidth) == 0x04,
              "render-destination width offset drift");
static_assert(offsetof(idRenderDestination, targetImage) == 0x10,
              "render-destination image table offset drift");
static_assert(sizeof(idRenderDestination) == 0x50,
              "render-destination ABI drift");
