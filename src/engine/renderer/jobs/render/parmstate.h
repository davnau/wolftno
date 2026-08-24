#pragma once

class idDeclRenderParm;
class idTextureSampler;
class idParmBlock;
class idVec3;
struct idRenderMatrix;

class idParmState {
public:
  void SetMVPMatrix(const idRenderMatrix& matrix);
  void SetParmFloat(const idDeclRenderParm* parameter, float value);
  void SetParmSampler(const idDeclRenderParm* parameter,
                      const idTextureSampler* sampler);
  void SetParmVec3(const idDeclRenderParm* parameter, const idVec3& value);
  void SetFragmentPositionToViewTexture(int renderWidth, int renderHeight,
                                        int windowWidth, int windowHeight);
  void SetSrcTexClamp(int validWidth, int validHeight, int textureWidth,
                      int textureHeight);
  void VirtualEvaluateParmBlock(const idParmBlock& localParmBlock,
                                const idParmBlock& parmBlock);
  void VirtualRollbackParmBlock(const idParmBlock& localParmBlock,
                                const idParmBlock& parmBlock);
};

extern idParmState* renderThreadParmState;
