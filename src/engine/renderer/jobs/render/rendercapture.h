#pragma once

enum renderCapture_t : int {
  CAPTURE_VIEW_COLOR,
  CAPTURE_VIEW_DEPTH,
  CAPTURE_VIEW_NORMAL,
  CAPTURE_FEEDBACK,
  CAPTURE_GLARE_MAP,
  CAPTURE_GUI_IMAGE,
};

class idDeclRenderParm;
class idDeclRenderProg;
class idImage;
class idPixelPackBuffer;
struct idRenderDestination;
class idTextureSampler;
class idTriangles;
struct renderSettings_t;
template <typename T> struct idHeapArray;

struct captureParms_t {
  renderCapture_t capture;
  const renderSettings_t* settings;
  int renderWidth;
  int renderHeight;
  int windowWidth;
  int windowHeight;
  bool forceFullVirtualTextureLoad;
  int viewIndex;
  const idRenderDestination* renderDestDefault;
  const idImage* imgViewDepth;
  const idRenderDestination* renderDestViewColor;
  const idRenderDestination* renderDestViewDepth;
  const idRenderDestination* renderDestViewNormal;
  idRenderDestination* renderDestFeedback;
  idHeapArray<unsigned char>* feedbackBuffer;
  idPixelPackBuffer* feedbackBufferObject;
  const idDeclRenderParm* rpFeedbackColor;
  const idRenderDestination* renderDestGui;
  bool createMipMaps;
  const idRenderDestination* renderDestMip;
  const idRenderDestination* renderDestMip3;
  const idDeclRenderParm* rpGlareMap;
  const idDeclRenderProg* progGlareScale;
  const idDeclRenderProg* progOverlappedDownSample;
  const idDeclRenderProg* progFeedbackDownSample;
  const idDeclRenderProg* progCaptureDepth;
  const idDeclRenderParm* rpViewColorMap;
  const idDeclRenderParm* rpMipMapSampler;
  const idTextureSampler* mipMapSamplers[16];
  const idDeclRenderParm* rpTargetDepth;
  const idTriangles* unitSquareTris;
  const idImage* imgBlack;
};

void CaptureViewColor(const captureParms_t* parameters);
void CaptureViewDepth(const captureParms_t* parameters);
void CaptureFeedback(const captureParms_t* parameters);
void CaptureGlareMap(const captureParms_t* parameters);
void CaptureGuiImage(const captureParms_t* parameters);
void RenderCapture(captureParms_t* parameters);

static_assert(sizeof(captureParms_t) == 352, "capture parameters ABI drift");
