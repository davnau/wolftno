#pragma once

// NEEDS_WORK(engine/renderer): this is the PDB-derived public boundary needed
// by GUI recovery. Renderer owns the complete image layout and implementation.

#include "engine/decls/decl.h"

#include <cstdint>

enum textureFilter_t : int { TF_LINEAR, TF_NEAREST, TF_NEAREST_MIPMAP_NEAREST,
  TF_LINEAR_MIPMAP_NEAREST, TF_TRILINEAR, TF_DEFAULT };
enum textureRepeat_t : int { TR_REPEAT, TR_CLAMP, TR_CLAMP_S, TR_CLAMP_T,
  TR_CLAMP_TO_BORDER, TR_MIRROR };
enum textureType_t : int { TT_2D, TT_3D, TT_CUBIC };
enum textureFormat_t : int { FMT_NONE, FMT_RGBA32F, FMT_RGBA16F, FMT_RGBA8,
  FMT_ARGB8, FMT_ALPHA, FMT_L8A8, FMT_RG8, FMT_LUM8, FMT_INT8, FMT_DXT1,
  FMT_DXT5, FMT_DEPTH, FMT_DEPTH_STENCIL, FMT_X32F, FMT_Y16F_X16F,
  FMT_X16, FMT_Y16_X16, FMT_RGB565 };
enum textureColor_t : int { CFM_DEFAULT, CFM_RGBA_HQ_DXT5, CFM_NORMAL_HQ_DXT5,
  CFM_YCOCG_DXT5, CFM_YCOCGA_DXT5, CFM_YCOCGA_HQ_DXT5, CFM_RGB_HQ_DXT1 };
enum textureBindType_t : int { TB_SHADER, TB_RENDER_TARGET, TB_DEPTH_STENCIL_TARGET };
enum textureUsage_t : int { TU_DEFAULT, TU_STATIC, TU_DYNAMIC, TU_STAGING };
enum textureOptions_t : int { TO_NONE, TO_GENERATE_MIPS };

struct samplerOptions_t {
  textureFilter_t filter{};
  textureRepeat_t repeat{};
  idVec4 border{};
  float lodBias{};
  float lodMinClamp{};
  float lodMaxClamp{};
  float aniso{};
  bool forceBias{};
  bool depthCompareMode{};
  std::uint16_t structPad{};
};

class idImageOpts {
public:
  textureType_t textureType{};
  textureFormat_t format{};
  textureColor_t colorFormat{};
  int width{};
  int height{};
  int depth{};
  int numLevels{};
  int numSamples{};
  bool packedTail{};
  bool readback{};
  bool linear{};
  bool partiallyResident{};
  bool cubeFilter{};
  bool overlayMemory{};
  bool startPurged{};
  std::uint8_t structPad{};
  textureBindType_t textureBindType{};
  textureUsage_t textureUsage{};
  textureOptions_t textureOptions{};
  samplerOptions_t sampler{};
  std::uint32_t structPad2{};
};

enum imageState_t : int {
  IMAGE_STATE_UNLOADED = 0,
  IMAGE_STATE_LOADING = 1,
  IMAGE_STATE_LOADED = 2,
};

class idImage : public idResource {
public:
  idImage();
  ~idImage() override;

  static void ParseImageOpts(class idParser& parser, idImageOpts& options);
  static void MakeGeneratedName(char* binaryFileName, int maxLength,
                                const char* imageName);
  static void WriteBinaryImage(const char* imageName, idImageOpts options,
                               const std::uint8_t* imageData,
                               std::uint32_t sourceFileTime);
  void LoadFromBinaryImage(const class idBinaryImage& binaryImage);
  struct idImageData* GetImageData(int mipLevel, int z) const;
  imageState_t ImageState() const;
  void PurgeImage();
  void BackgroundRead(class idFile* file, std::uint64_t offset);
  void SubImageUpload(int mipLevel, int x, int y, int z, int width,
                      int height, const void* pixels, int pixelPitch) const;

  unsigned sourceFileTime{};
  unsigned binaryFileTime{};
  void* backgroundReadData{};
  volatile bool backgroundReadComplete{};
  int frameUsed{};
  int bindCount{};
  idImageOpts opts{};
  bool optsHasBeenSet{};
  bool defaulted{};
  bool allocationFailed{};
  int physicalMemoryBytes{};
  void* imageBuffer{};
  const class idTextureSampler* textureSampler{};
  unsigned oglTexture{~0u};
  unsigned internalFormat{};
  unsigned dataFormat{};
  unsigned dataType{};
  unsigned target{};
  unsigned bindTarget{};
  unsigned uploadTarget{};

  static idTypedResourceList<idImage> resourceList;
};

void ComputeNumLevels(idImageOpts* options, const char* imageName);

static_assert(sizeof(samplerOptions_t) == 44, "samplerOptions_t ABI drift");
static_assert(sizeof(idImageOpts) == 100, "idImageOpts ABI drift");
static_assert(offsetof(idImage, opts) == 76, "idImage options offset drift");
static_assert(offsetof(idImage, oglTexture) == 200, "idImage texture offset drift");
static_assert(offsetof(idImage, dataFormat) == 208, "idImage data format offset drift");
static_assert(offsetof(idImage, dataType) == 212, "idImage data type offset drift");
