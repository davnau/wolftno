#pragma once

#include "engine/framework/common.h"
#include "engine/framework/keyinput.h"
#include "engine/framework/usercmd.h"
#include "engine/renderer/render.h"
#include "shared/idlib/containers/staticlist.h"
#include "shared/idlib/math/vector.h"

#include <cstddef>
#include <cstdint>

class idBitMsg;
class idFile_SaveGame;
class idGameSpawnInfo;
class idRenderModelGui;
class idSnapShot;
class idRenderWorld {
public:
  const void* const* vtable;
  void ReadFromSnapshot(idSnapShot& snap,const idSnapShot* oldSnap) {
    using Fn=void(*)(idRenderWorld*,idSnapShot&,const idSnapShot*);
    reinterpret_cast<Fn>(vtable[35])(this,snap,oldSnap);
  }
};
class idSoundWorld {
public:
  const void* const* vtable;
  void ReadFromSnapshot(idSnapShot& snap,const idSnapShot* oldSnap) {
    using Fn=void(*)(idSoundWorld*,idSnapShot&,const idSnapShot*);
    reinterpret_cast<Fn>(vtable[33])(this,snap,oldSnap);
  }
};

struct inputSettings_t {
  bindSet_t bindset;
  float mouseSensScale;
  float joySensScale;
  float joyPitchSpeed;
  float joyYawSpeed;
  float mouseSens;
  bool absoluteAngles;
  bool lookInvert;
  std::byte padding[2];
  idAngles aimAssistAngles;
};

struct gameReturnPlayer_t {
  bool valid;
  std::byte padding[15];
  renderView_t view;
  void* gui;
  void* inventoryModel;
  void* hudModel;
  inputSettings_t inputSettings;
  int vibrationLow;
  int vibrationHigh;
  int vibrationLeft;
  int vibrationRight;
  std::uint32_t visMask;
  idVec3 velocity;
};

struct alignas(16) gameReturn_t {
  int localTime;
  int serverTime;
  bool forceOneView;
  std::byte padding0[3];
  std::uint32_t initTools;
  std::uint32_t updateTools;
  std::byte padding1[12];
  gameReturnPlayer_t players[16];
  idStaticList<idRenderModelGui*,8> debugGuis;
};

// idGame is supplied by the game DLL.  Calls remain explicit vtable dispatches
// so this framework recovery does not invent the rest of its 193-slot ABI.
class idGame {
public:
  const void* const* vtable;

  template <typename Fn> Fn Slot(int index) const {
    return reinterpret_cast<Fn>(vtable[index]);
  }
  void RunFrame(gameFrameParms_t parms,gameReturn_t* result) {
    Slot<void(*)(idGame*,gameFrameParms_t,gameReturn_t*)>(1)(this,parms,result);
  }
  void NetReceiveReliable(int type,idBitMsg& msg,int peer) {
    Slot<void(*)(idGame*,int,idBitMsg&,int)>(2)(this,type,msg,peer);
  }
  idRenderWorld* GetRenderWorld() {
    return Slot<idRenderWorld*(*)(idGame*)>(8)(this);
  }
  idSoundWorld* GetSoundWorld() {
    return Slot<idSoundWorld*(*)(idGame*)>(9)(this);
  }
  const idGameSpawnInfo* GetGameSpawnInfo() {
    return Slot<const idGameSpawnInfo*(*)(idGame*)>(12)(this);
  }
  idFile_SaveGame* CheckpointGetSaveFile() {
    return Slot<idFile_SaveGame*(*)(idGame*)>(37)(this);
  }
  bool CheckpointProcessRead(idFile_SaveGame* file,int slot,const char* mapName) {
    return Slot<bool(*)(idGame*,idFile_SaveGame*,int,const char*)>(38)(
        this,file,slot,mapName);
  }
  int GetGameMs(int timeType) {
    return Slot<int(*)(idGame*,int)>(47)(this,timeType);
  }
  void SetServerGameTimeMs(int value) {
    Slot<void(*)(idGame*,int)>(54)(this,value);
  }
  int GetServerGameTimeMs() {
    return Slot<int(*)(idGame*)>(55)(this);
  }
  void SetSSTimes(int start,int end) {
    Slot<void(*)(idGame*,int,int)>(56)(this,start,end);
  }
  void Shell_ShowNewContentAvailable() {
    Slot<void(*)(idGame*)>(68)(this);
  }
  idRenderModelGui* Shell_Render() {
    return Slot<idRenderModelGui*(*)(idGame*)>(63)(this);
  }
  void ReadFromSnapshot(idSnapShot& snap,const idSnapShot* oldSnap) {
    Slot<void(*)(idGame*,idSnapShot&,const idSnapShot*)>(78)(this,snap,oldSnap);
  }
  void SetInterpolation(float fraction,int serverTime,int previousTime,int nextTime) {
    Slot<void(*)(idGame*,float,int,int,int)>(80)(this,fraction,serverTime,previousTime,nextTime);
  }
  idRenderModelGui* RenderLocalViewForPlayer(int playerIndex,int localTime,
                                             renderView_t* view,
                                             idRenderWorld* world) {
    return Slot<idRenderModelGui*(*)(idGame*,int,int,renderView_t*,idRenderWorld*)>(83)(
        this,playerIndex,localTime,view,world);
  }
  idRenderModelGui* RenderLocalViewForGame(int localTime,idRenderWorld* world) {
    return Slot<idRenderModelGui*(*)(idGame*,int,idRenderWorld*)>(84)(
        this,localTime,world);
  }
  int GetPlayerIndexFromInputDevice(int device) {
    return Slot<int(*)(idGame*,int)>(87)(this,device);
  }
  void GetPlayerPredictionData(int player,usercmd_t** commands,int count,bool& smoothing) {
    Slot<void(*)(idGame*,int,usercmd_t**,int,bool&)>(89)(this,player,commands,count,smoothing);
  }
  void SmoothIncomingUserCmds(usercmd_t& base,usercmd_t* commands,int count) {
    Slot<void(*)(idGame*,usercmd_t&,usercmd_t*,int)>(90)(this,base,commands,count);
  }
};

static_assert(sizeof(renderView_t)==0x1980,"renderView_t ABI drift");
static_assert(offsetof(renderView_t,viewID)==4,"renderView_t::viewID ABI drift");
static_assert(offsetof(renderView_t,fov_x)==0x10,"renderView_t::fov_x ABI drift");
static_assert(offsetof(renderView_t,vieworg)==0x60,"renderView_t::vieworg ABI drift");
static_assert(offsetof(renderView_t,viewaxis)==0x6C,"renderView_t::viewaxis ABI drift");
static_assert(sizeof(inputSettings_t)==40,"inputSettings_t ABI drift");
static_assert(sizeof(gameReturnPlayer_t)==6640,"gameReturnPlayer_t ABI drift");
static_assert(offsetof(gameReturnPlayer_t,inputSettings)==0x19A8,"player input ABI drift");
static_assert(sizeof(gameReturn_t)==106368,"gameReturn_t ABI drift");
static_assert(offsetof(gameReturn_t,players)==0x20,"gameReturn_t players ABI drift");
