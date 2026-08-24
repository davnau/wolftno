#pragma once

#include "engine/sound/soundshader.h"
#include "shared/idlib/math/vector.h"

struct millisecondUnique_t {
  int value{0};
};

class idSoundEmitter {
public:
  virtual int Index() = 0;
  virtual void Free(bool instant) = 0;
  virtual void Reset() = 0;
  virtual void UpdateEmitter(const idVec3& origin, const idMat3& axis,
                             const idVec3& velocity, int listenerId,
                             bool virtualize) = 0;
  virtual millisecondUnique_t StartSound(soundChannel_t channel,
                                         const idSoundShader* shader,
                                         const soundShaderParms_t* parms) = 0;
  virtual void ModifySound(soundChannel_t, const idSoundShader*,
                           const soundShaderParms_t*) = 0;
  virtual void StopSound(void*) = 0;
  virtual void StopSound(soundChannel_t channel) = 0;
  virtual void SetVolume(soundChannel_t, float) = 0;
  virtual void SetPitch(soundChannel_t, float) = 0;
  virtual void FadeSound(soundChannel_t, float, float, float, bool) = 0;
  virtual void FadePitch(soundChannel_t, float, float, float) = 0;
  virtual bool IsCurrentlyPlaying(soundChannel_t channel) = 0;
  virtual ~idSoundEmitter();
};

class idSoundWorld {
public:
  virtual ~idSoundWorld();
  virtual void StopAllSounds() = 0;
  virtual idSoundEmitter* AllocSoundEmitter() = 0;
};

class idSoundSystem {
public:
  virtual ~idSoundSystem();
  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual idSoundWorld* AllocSoundWorld(const char*, void*) = 0;
  virtual void FreeSoundWorld(idSoundWorld*) = 0;
  virtual void SetPlayingSoundWorld(idSoundWorld*) = 0;
  virtual void SetMixingDecl(const idDeclSoundMixing*) = 0;
  virtual const idDeclSoundMixing* GetMixingDecl() = 0;
  virtual idSoundWorld* GetPlayingSoundWorld() = 0;
  virtual void SetEditorSoundWorld(idSoundWorld*) = 0;
  virtual idSoundWorld* GetEditorSoundWorld() = 0;
  virtual idSoundWorld* GetGlobalSoundWorld() = 0;
};

extern idSoundSystem* soundSystem;
