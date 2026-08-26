#pragma once

#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idClip;
class idDeclBreakable;
class idRenderModelDiscreteAnimation;
class idRenderModelEffects;
class idSoundWorld;
class idTraceModelCache;
class idDeclImpactSound;

class idEffectPhysicsPieceEmitter {
public:
  struct pieceEmitterSound_t {
    idSoundWorld* soundWorld;
    const idDeclImpactSound* impactTable;
    int nextSoundTime;
    float minBounceVelSqr;
  };

  void Init(idClip* clip, idTraceModelCache* traceModelCache,
            idRenderModelDiscreteAnimation* model,
            idRenderModelEffects* modelEffects,
            pieceEmitterSound_t* soundInfo, int entityNumber);
  void EmitPiece(int emitTime, const idVec3& position,
                 const idMat3& axes, const idVec3& impulsePoint,
                 const idVec3& impulse);
  void UpdateSimulation(const idVec3& origin, const idMat3& axis,
                        int currentTime, int gameMsPerFrame);
  void UpdateModel(const idVec3& activeDrawScale3D);
  void SubmitCollisionQueries(bool worldCollisionModelOnly);

private:
  std::uint8_t prefix[0xE0];

public:
  int pieceLifeTime;
  float pieceFriction;
  int pieceIndex;
  int numPiecesInUse;
  bool hidden;
  bool hiddenOverride;

private:
  std::uint8_t tailPadding[6];
};

static_assert(sizeof(idEffectPhysicsPieceEmitter::pieceEmitterSound_t) ==
                  0x18,
              "pieceEmitterSound_t ABI drift");
static_assert(offsetof(idEffectPhysicsPieceEmitter, pieceLifeTime) == 0xE0,
              "piece emitter lifetime ABI drift");
static_assert(offsetof(idEffectPhysicsPieceEmitter, numPiecesInUse) == 0xEC,
              "piece emitter in-use count ABI drift");
static_assert(sizeof(idEffectPhysicsPieceEmitter) == 0xF8,
              "idEffectPhysicsPieceEmitter ABI drift");

