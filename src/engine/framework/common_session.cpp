#include "engine/framework/common_local.h"

#include "engine/framework/discswapmanager.h"
#include "engine/framework/console.h"
#include "engine/framework/gamesystem.h"
#include "engine/framework/gamespawninfo.h"
#include "engine/framework/keyinput.h"
#include "engine/framework/playerprofile.h"
#include "engine/framework/session.h"
#include "engine/framework/rendermanager.h"
#include "engine/framework/swfhelper.h"
#include "engine/network/snapshot.h"
#include "engine/renderer/renderersystem.h"
#include "engine/sound/soundsystem.h"
#include "engine/sys/sys_signin.h"
#include "engine/sys/sys_lobby_types.h"
#include "engine/sys/sys_public.h"
#include "shared/idlib/csystems/cvarsystemlocal.h"
#include "shared/idlib/csystems/cmdsystemlocal.h"
#include "shared/idlib/exception.h"
#include "shared/idlib/filesystem/compressor.h"
#include "shared/idlib/filesystem/filesystemlocal.h"
#include "shared/idlib/hashing/uuid.h"
#include "shared/idlib/metrics/timer.h"
#include "shared/idlib/sys/sys_time.h"
#include "shared/idlib/text/cmdargs.h"

#include <cstring>
#include <cstdlib>

extern int com_editors;
extern idCVar si_spawnspot;
extern idCVar com_captureFrames;
extern idCVar com_captureSounds;
extern idCVar com_production;
extern idCVar discSwapper_verbose;
extern idCVar si_nextlayeractive;
extern idCVar si_nextlayerdeactive;
extern idCVar si_stopWatch;
extern idCVar si_swapTeams;
extern idCVar si_teamCount;
extern idCVar si_scoreLimit;
extern idCVar si_timeLimit;
extern idCVar si_minRespawnDelay;
extern void Sys_SoundCaptureStop();
extern void Sys_PumpEvents();
extern idSysMutex mutexHitch;

struct idVRAction {
  const char* phrases[3];
  const char* cmdCommand;
  const char* usrCmdAction;
  keyNum_t keyPress;
  std::byte padding[4];
};
static_assert(sizeof(idVRAction)==48,"idVRAction ABI drift");
extern idVRAction vrIngameActions[21];
extern idVRAction vrMenuActions[4];

void WaitOnSession(unsigned int waitingOnState,float pacifierProgress,
                   unsigned int pacifierState) {
  while(static_cast<unsigned int>(session->GetState())!=waitingOnState) {
    Sys_PumpEvents();
    if(pacifierProgress<0.0f) {
      session->UpdateSignInManager();
      session->Pump();
      session->ProcessSnapAckQueue();
    } else {
      common->UpdateLevelLoadPacifier(
          pacifierProgress,static_cast<pacifierState_t>(pacifierState),true,true);
    }
    Sys_Sleep(10);
    const int state=session->GetState();
    if(state!=6 && state!=2 && state!=3 && state!=8 &&
       static_cast<unsigned int>(state)!=waitingOnState) {
      idStr::snPrintf(idException::error,idException::MAX_ERROR_LEN,
          "WaitOnSession: Invalid state (%s) while waiting on state (%s).",
          session->GetStateString(),session->GetStateString(
              static_cast<int>(waitingOnState)));
      idNetworkLoadException exception;
      throw &exception;
    }
  }
}

void idCommonLocal::RestartPassMainMenu_f(const idCmdArgs* args) {
  commonLocal.Game()->RestartPassMainMenu(*args);
}

void idCommonLocal::RestartMultiplayerInvite_f(const idCmdArgs* args) {
  if(args->argc==2) {
    idLib::Printf("Restarting with multiplayer invite\n");
    discSwapMgr->Reset();
    if(discSwapMgr->RebuildStateAfterSwap(
           idDiscSwapManager::DSC_MULTIPLAYER_INVITE,*args) &&
       discSwapMgr->Swap(idDiscSwapManager::DSC_MULTIPLAYER_INVITE,3,true)==
           idDiscSwapManager::ERROR_NONE)
      session->JoinAfterSwap(&discSwapMgr->inviteInfo);
  } else {
    idLib::Warning("syntax: RestartMultiplayerInvite_f <swapDat_invite_stuff>");
  }
}

void idCommonLocal::RestartLoadGame_f(const idCmdArgs* args) {
  idLib::Printf("DISCSWAP      :  RestartLoadGame_f\n");
  if(args->argc==3) {
    idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
                    "[%s]:","idCommonLocal::RestartLoadGame_f");
    for(int index=0;index<args->argc;++index)
      idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
                      "\targs[%d] = %s\n",index,args->Argv(index));
    if(discSwapMgr->RebuildStateAfterSwap(idDiscSwapManager::DSC_LOAD,*args)) {
      commonLocal.Game()->CheckpointBlockOnLoad();
      cmdSystem->AppendCommandText(va("checkmap %d nextMapDiscSwap\n",-1));
      idLib::Printf("Loading checkpoint file %d...\n",-1);
    }
  } else {
    idLib::Warning("syntax: restart_loadGame <swapSerializedUser> <savegame>");
  }
}

void idCommonLocal::RestartMapAfterSwap_f(const idCmdArgs* args) {
  idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
                  "[%s]:","idCommonLocal::RestartMapAfterSwap_f");
  for(int index=0;index<args->argc;++index)
    idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
                    "\targs[%d] = %s\n",index,args->Argv(index));
  if(discSwapMgr->RebuildStateAfterSwap(idDiscSwapManager::DSC_MAP,*args)) {
    commonLocal.Game()->CheckpointBlockOnLoad();
    if(discSwapMgr->Swap(idDiscSwapManager::DSC_MAP,0,true)==
       idDiscSwapManager::ERROR_NONE) {
      si_spawnspot.SetString(discSwapMgr->spawnSpot.IsEmpty()
          ? "player_start_1" : discSwapMgr->spawnSpot.c_str(),true);
      si_nextlayerdeactive.SetString("",true);
      si_nextlayeractive.SetString(discSwapMgr->layerData.IsEmpty()
          ? "" : discSwapMgr->layerData.c_str(),true);
      cmdSystem->AppendCommandText(discSwapMgr->additionalArguments.c_str());
      idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
          "DISCSWAP: restartmapafterswap command: %s\n",
          discSwapMgr->additionalArguments.c_str());
      idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
          "          layer: '%s'\n",discSwapMgr->layerData.c_str());
      idLib::PrintfIf(discSwapper_verbose.valueInteger!=0,
          "\t\t  spawnspot: '%s'\n",discSwapMgr->spawnSpot.c_str());
      cmdSystem->ExecuteCommandBuffer();
    }
  }
}

void idCommonLocal::PrepareForMapChange() {
  CloseSnapshotFiles();
  if(idLib::production==3) idLib::production=2;
  console->Close(true);
  cvarSystem->ReportModifiedCVars();
  soundSystem->SetPlayingSoundWorld(nullptr);
  for(int index=0;index<4;++index) {
    sys->Joystick()->ResetColor(index);
    sys->Joystick()->SetRumble(index,0,0,0,0);
  }
  renderManager.Clear();
  gameSystem->FreeGame(Field<idGame*>(0x1D0));
  idList<idMapChangeListener*,5>& listeners=
      Field<idList<idMapChangeListener*,5>>(0x34CA8);
  for(int index=0;index<listeners.Num();++index)
    listeners[index]->MapDisconnected();
  session->SetUserSignedOutDuringLoading(false);
}

void idCommonLocal::LeaveGame_f(const idCmdArgs*) {
  if(session->GetActingGameStateLobbyBase().IsHost()) session->EndMatch(false);
}

void idCommonLocal::LoadGame_f(const idCmdArgs* args) {
  idStr saveName(args->Argv(1));
  if(saveName.IsEmpty()) saveName=session->GetCurrentSaveSlot();
  si_spawnspot.SetString("",true);
  commonLocal.Game()->LoadGame(saveName.c_str());
}

void idCommonLocal::SaveGame_f(const idCmdArgs* args) {
  idStr saveName(args->Argv(1));
  if(saveName.IsEmpty()) saveName=session->GetCurrentSaveSlot();
  commonLocal.Game()->RequestSaveGame(saveName.c_str(),true,false);
}

idFile* idCommonLocal::DetachActiveReadSnapshotFile() {
  delete Field<idFile*>(0x6D8);
  Field<idFile*>(0x6D8)=nullptr;
  delete Field<idFile*>(0x6E0);
  Field<idFile*>(0x6E0)=nullptr;
  idFile* result=Field<idFile*>(0x6E8);
  Field<idFile*>(0x6E8)=nullptr;
  return result;
}

idFile* idCommonLocal::DetachActiveWriteSnapshotFile() {
  delete Field<idFile*>(0x700);
  Field<idFile*>(0x700)=nullptr;
  delete Field<idFile*>(0x708);
  Field<idFile*>(0x708)=nullptr;
  idFile* result=Field<idFile*>(0x710);
  Field<idFile*>(0x710)=nullptr;
  return result;
}

bool idCommonLocal::IsInGame() {
  return !Field<bool>(0x162) && Game()!=nullptr && Game()->IsGameActive(true);
}

snapshotState_t idCommonLocal::SnapshotState() {
  if(Field<idFile*>(0x6E8)!=nullptr) return SNAP_READING;
  if(!session->GetActingGameStateLobbyBase().IsHost() &&
     session->GetActingGameStateLobbyBase().HasActivePeers())
    return SNAP_READING;
  if(Field<idFile*>(0x710)!=nullptr) return SNAP_WRITING;
  if(session->GetActingGameStateLobbyBase().IsHost() &&
     session->GetActingGameStateLobbyBase().HasActivePeers())
    return SNAP_WRITING;
  return SNAP_NONE;
}

bool idCommonLocal::IsToolActive() { return com_editors!=0; }

void idCommonLocal::QueueSnapshotMemFile(idFile_Memory* file,unsigned int tag) {
  Field<idFile_Memory*>(0x34678)=file;
  Field<unsigned int>(0x34680)=tag;
}

void idCommonLocal::ClearQueuedSnapshotFile() {
  Field<idFile_Memory*>(0x34678)=nullptr;
  Field<unsigned int>(0x34680)=0;
}

void idCommonLocal::PlayQueuedSnapshotFile() {
  PlaySnapshotFile(Field<idFile_Memory*>(0x34678),Field<unsigned int>(0x34680));
  ClearQueuedSnapshotFile();
}

void idCommonLocal::StopSnapshotFileRecord() {
  *reinterpret_cast<std::int64_t*>(&SnapPrevious().localTime)=-1;
  Field<std::uint64_t>(0x748)=0;
  *reinterpret_cast<std::int64_t*>(&SnapCurrent().localTime)=-1;
  delete Field<idFile*>(0x700);
  Field<idFile*>(0x700)=nullptr;
  delete Field<idFile*>(0x708);
  Field<idFile*>(0x708)=nullptr;
  delete Field<idFile*>(0x710);
  Field<idFile*>(0x710)=nullptr;
  idSnapShot* snapshot=Field<idSnapShot*>(0x718);
  if(snapshot!=nullptr) {
    snapshot->~idSnapShot();
    idMem::Singleton()->Free(snapshot,ALIGN_16);
  }
  Field<idSnapShot*>(0x718)=nullptr;
  Field<std::uint64_t>(0x720)=0;
  Field<int>(0x6D0)=Field<int>(0x6D0)==4 && Field<idFile*>(0x6E8)!=nullptr ? 4 : 0;
}

void idCommonLocal::StopSnapshotFilePlayback() {
  *reinterpret_cast<std::int64_t*>(&SnapPrevious().localTime)=-1;
  Field<std::uint64_t>(0x738)=0;
  *reinterpret_cast<std::int64_t*>(&SnapCurrent().localTime)=-1;
  delete Field<idFile*>(0x6D8);
  Field<idFile*>(0x6D8)=nullptr;
  delete Field<idFile*>(0x6E0);
  Field<idFile*>(0x6E0)=nullptr;
  delete Field<idFile*>(0x6E8);
  Field<idFile*>(0x6E8)=nullptr;
  idSnapShot* snapshot=Field<idSnapShot*>(0x6F0);
  if(snapshot!=nullptr) {
    snapshot->~idSnapShot();
    idMem::Singleton()->Free(snapshot,ALIGN_16);
  }
  Field<idSnapShot*>(0x6F0)=nullptr;
  Field<std::uint64_t>(0x6F8)=0;
  Field<int>(0x6D0)=Field<int>(0x6D0)==4 && Field<idFile*>(0x710)!=nullptr ? 4 : 0;
  if(com_captureFrames.valueInteger!=0) com_captureFrames.SetInteger(0,true);
  if(com_captureSounds.valueInteger!=0) {
    com_captureSounds.SetInteger(0,true);
    cvarSystem->SetCVarFloat("s_volume_db",0.0f,0);
    Sys_SoundCaptureStop();
  }
}

void idCommonLocal::CloseSnapshotFiles() {
  if(Field<int>(0x6D0)==0) return;
  StopSnapshotFilePlayback();
  StopSnapshotFileRecord();
  Game()->OnCloseSnapshot();
}

void idCommonLocal::StopDemo_f(const idCmdArgs*) { commonLocal.CloseSnapshotFiles(); }

idPlayerProfile* idCommonLocal::GetProfileFromMasterLocalUser() {
  idPlayerProfile* profile=session->GetProfileFromMasterLocalUser();
  if(profile==nullptr) idLib::Error("MasterLocalUser profile is NULL");
  return profile;
}

idSWFHelper* idCommonLocal::GetSWFHelper() {
  return Field<idSWFHelper*>(0x34CC0);
}

void idCommonLocal::VoiceRecognition_ClearUnprocessedCmd() {
  session->VoiceRecognition_ClearUnprocessedPhrases();
}

void idCommonLocal::VoiceRecognition_GetBareWordlist(idList<idStr,44>& outList) {
  for(const idVRAction& action : vrIngameActions) {
    for(const char* phrase : action.phrases) {
      if(phrase!=nullptr) outList.Append(idStr(phrase));
    }
  }
  for(const idVRAction& action : vrMenuActions) {
    for(const char* phrase : action.phrases) {
      if(phrase==nullptr) continue;
      const idStr value(phrase);
      int index=0;
      while(index<outList.Num() && !(outList[index]==value)) ++index;
      if(index>=outList.Num()) outList.Append(value);
    }
  }
}

bool idCommonLocal::CutScenePress(const sysEvent_t& event,bool pressAlready) {
  if(event.evType!=SE_KEY || (event.evValue2&1)!=0) return false;
  if(!pressAlready) {
    if(idKeyInput::KeyIsBoundToMulti(event.inputDevice,BIND_DEFAULT,
           static_cast<keyNum_t>(event.evValue),0,"_use","_useandreload",
           "_crouch",nullptr) || event.evValue==28 || event.evValue==57)
      renderSystem->SetRenderFlag(RENDERFLAG_PRESSEDDURINGCUTSCENE);
    return false;
  }
  if(idKeyInput::KeyIsBoundToMulti(event.inputDevice,BIND_DEFAULT,
         static_cast<keyNum_t>(event.evValue),0,"_use","_useandreload",
         nullptr,nullptr) || event.evValue==28 || event.evValue==57) {
    renderSystem->RemoveRenderFlag(7);
    return true;
  }
  return false;
}

int idCommonLocal::GetMasterLocalUserInputDevice() {
  idSignInManagerBase& signIn=session->GetSignInManager();
  if(signIn.GetMasterLocalUser()==nullptr) return -1;
  return signIn.GetMasterLocalUser()->GetInputDevice();
}

int idCommonLocal::GetMasterLocalUserPlayerId() {
  const int player=Game()->GetPlayerIndexFromInputDevice(
      GetMasterLocalUserInputDevice());
  return player==-1 ? 0 : player;
}

idLocalUser* idCommonLocal::GetMasterLocalUser() {
  return session->GetSignInManager().GetMasterLocalUser();
}

void idUserCmdMgr::ResetPlayer(int playerIndex) {
  for(int frame=0;frame<128;++frame)
    std::memset(&cmdBuffer[frame][playerIndex],0,sizeof(usercmd_t));
  writeFrame[playerIndex]=0;
  readFrame[playerIndex]=-1;
}

const idStrStatic<1024>& idGameSpawnInfo::GetSpawnSpot() const {
  return spawnSpotOverride.IsEmpty() ? spawnSpot : spawnSpotOverride;
}

void idCommonLocal::ResetNetworkingState() {
  Field<std::uint64_t>(0x728)=100000;
  Field<float>(0x34C38)=0.5f;
  Field<float>(0x34C3C)=0.0f;
  Field<std::uint64_t>(0x738)=0;
  Field<std::uint64_t>(0x748)=0;
  Field<std::uint64_t>(0x750)=0;
  Field<std::uint64_t>(0x34660)=0;
  Field<std::uint64_t>(0x34668)=0;
  Field<int>(0x34670)=0;
  Field<std::uint64_t>(0x34730)=0;
  Field<std::uint64_t>(0x34C40)=0;
  for(int i=0;i<16;++i) ReceivedSnaps()[i].Clear();
  std::memset(UserCmdMgr().cmdBuffer,0,sizeof(UserCmdMgr().cmdBuffer));
  std::memset(UserCmdMgr().writeFrame,0,sizeof(UserCmdMgr().writeFrame));
  std::memset(UserCmdMgr().readFrame,-1,sizeof(UserCmdMgr().readFrame));
  *reinterpret_cast<std::int64_t*>(&SnapCurrent().localTime)=-1;
  *reinterpret_cast<std::int64_t*>(&SnapPrevious().localTime)=-1;
  OldSnapshot().Clear();
}

void idCommonLocal::ResetPlayerInput(int playerIndex) {
  UserCmdMgr().ResetPlayer(playerIndex);
}

void idCommonLocal::UpdateBotUserCmds(int botPlayerIndex,const usercmd_t& command) {
  UserCmdMgr().PutUserCmdForPlayer(botPlayerIndex,command);
}

idUUID::idUUID(const idUUID& other)
    : version(other.version),hash(other.hash),generator(other.generator),
      data(other.data) {}

void idMatchParameters::SetMapName(const char* name) {
  const idStrStatic<128> value(name);
  mapName=value;
}

idMatchParameters::idMatchParameters(int gameMode_,int searchMode_,
    int gameMap_,int gameType_,int numSlots_,std::uint8_t matchFlags_,
    const idStr& mapName_,const idStr& layersActive_,
    const idStr& layersDeactive_,const idStr& spawnSpot_,
    const idStr& loadingScreen_,float minRespawnDelay_,float timeLimit_,
    int scoreLimit_,int teamCount_,bool swapTeams_,bool stopWatch_,
    bool comboMap_,bool devmap_,bool transitional_,bool forceHeapReset_,
    int checkpointSlot,bool playVideo_)
    : matchID(),gameMode(gameMode_),searchMode(searchMode_),gameMap(gameMap_),
      gameType(gameType_),numSlots(numSlots_),startCheckSlot(checkpointSlot),
      matchFlags(matchFlags_),mapName(mapName_),layersActive(layersActive_),
      layersDeactive(layersDeactive_),spawnSpot(spawnSpot_),
      loadingScreen(loadingScreen_),timeLimit(static_cast<int>(timeLimit_)),
      scoreLimit(scoreLimit_),numBots(-1),botSkill(1),
      minRespawnDelay(minRespawnDelay_),teamCount(teamCount_),
      swapTeams(swapTeams_),stopWatch(stopWatch_),comboMap(comboMap_),
      devmap(devmap_),isTransitionalMapRequest(transitional_),
      forceHeapReset(forceHeapReset_),playVideo(playVideo_),
      nightmareCompletionStat(-1),completionStats(),collectionStat(-1),
      specialCollectionStat(-1),challengeLeaderboardId(-1),skillLevel(0),
      nextMapDiscSwap(false) {}

idMatchParameters::idMatchParameters(const idMatchParameters& other)
    : matchID(other.matchID),gameMode(other.gameMode),
      searchMode(other.searchMode),gameMap(other.gameMap),
      gameType(other.gameType),numSlots(other.numSlots),
      startCheckSlot(other.startCheckSlot),matchFlags(other.matchFlags),
      mapName(other.mapName),layersActive(other.layersActive),
      layersDeactive(other.layersDeactive),spawnSpot(other.spawnSpot),
      loadingScreen(other.loadingScreen),timeLimit(other.timeLimit),
      scoreLimit(other.scoreLimit),numBots(other.numBots),
      botSkill(other.botSkill),minRespawnDelay(other.minRespawnDelay),
      teamCount(other.teamCount),swapTeams(other.swapTeams),
      stopWatch(other.stopWatch),comboMap(other.comboMap),devmap(other.devmap),
      isTransitionalMapRequest(other.isTransitionalMapRequest),
      forceHeapReset(other.forceHeapReset),playVideo(other.playVideo),
      nightmareCompletionStat(other.nightmareCompletionStat),
      completionStats(other.completionStats),collectionStat(other.collectionStat),
      specialCollectionStat(other.specialCollectionStat),
      challengeLeaderboardId(other.challengeLeaderboardId),
      skillLevel(other.skillLevel),nextMapDiscSwap(other.nextMapDiscSwap) {}

idMatchParameters::~idMatchParameters()=default;

idGameSpawnInfo::idGameSpawnInfo(const idGameSpawnInfo& other)
    : gameFlags(other.gameFlags),mapName(other.mapName),
      spawnSpot(other.spawnSpot),spawnSpotOverride(other.spawnSpotOverride),
      checkpointName(other.checkpointName),loadingScreen(other.loadingScreen),
      activeLayers(other.activeLayers),deactiveLayers(other.deactiveLayers),
      comboMap(other.comboMap),devmap(other.devmap),
      forceHeapReset(other.forceHeapReset),gameSpawnFlags(other.gameSpawnFlags),
      numPlayers(other.numPlayers),
      currentCheckpointSlot(other.currentCheckpointSlot) {}

idGameSpawnInfo::idGameSpawnInfo(const char* mapName_,int gameFlags_)
    : gameFlags(gameFlags_),mapName(mapName_),spawnSpot(),spawnSpotOverride(),
      checkpointName(),loadingScreen(),activeLayers(),deactiveLayers(),
      comboMap(false),devmap(false),forceHeapReset(false),gameSpawnFlags(1),
      numPlayers(1),currentCheckpointSlot(-1) {}

void idGameSpawnInfo::SetSpawnSpot(const char* value) {
  const idStrStatic<1024> text(value);
  spawnSpot=text;
}

void idGameSpawnInfo::SetSpawnSpotOverride(const char* value) {
  const idStrStatic<1024> text(value);
  spawnSpotOverride=text;
}

void idGameSpawnInfo::WriteLayersToCommaSeparatedStrings(
    idStr& activeString,idStr& deactiveString) {
  activeString="";
  for(int index=0;index<activeLayers.Num()-1;++index) {
    activeString+=activeLayers[index];
    activeString+=",";
  }
  if(activeLayers.Num()>0) activeString+=activeLayers[activeLayers.Num()-1];

  deactiveString="";
  for(int index=0;index<deactiveLayers.Num()-1;++index) {
    deactiveString+=deactiveLayers[index];
    deactiveString+=",";
  }
  if(deactiveLayers.Num()>0)
    deactiveString+=deactiveLayers[deactiveLayers.Num()-1];
}

void idCommonLocal::Hitch_f(const idCmdArgs* args) {
  idSoundWorld* playingWorld=soundSystem->GetPlayingSoundWorld();
  if(playingWorld!=nullptr) {
    soundSystem->SetMute(true);
    Sys_MutexLock(&mutexHitch.handle,true);
  }
  const unsigned long milliseconds=args->argc==2
      ? static_cast<unsigned long>(std::atol(args->Argv(1))) : 100UL;
  Sys_Sleep(milliseconds);
  if(playingWorld!=nullptr) {
    Sys_MutexUnlock(&mutexHitch.handle);
    soundSystem->SetMute(false);
  }
}

void idCommonLocal::UpdateConsoleDisplay(bool) {
  if(idLib::production!=1 && com_production.valueInteger<=0 &&
     console->IsInitialized() && !Field<bool>(0x163)) {
    Field<bool>(0x163)=true;
    sys->Mouse()->GrabCursor(false);
    sys->GenerateEvents();
    renderManager.UpdateConsole();
    soundSystem->Render(false);
    Field<bool>(0x163)=false;
  }
}

void idCommonLocal::CompressFile_f(const idCmdArgs* args) {
  idStr name;
  idTimer timer;
  timer.Start();
  idFile* input=fileSystem->OpenFileReadRetail(args->Argv(1),true,false);
  if(input==nullptr) return;
  name=args->Argv(1);
  name.SetFileExtension("zid");
  idFile* output=fileSystem->OpenFileWriteRetail(name.c_str(),FSPATH_SAVE);
  if(output==nullptr) return;

  idCompressor* arithmetic=idCompressor::AllocArithmetic();
  arithmetic->Init(output,true,8);
  idCompressor* lzss=idCompressor::AllocLZSS_ByteAligned();
  lzss->Init(arithmetic,true,8);
  const int length=static_cast<int>(input->Length());
  lzss->Write(&length,sizeof(length));
  unsigned char buffer[65536];
  int offset=0;
  while(offset<length-65536) {
    input->Read(buffer,sizeof(buffer));
    lzss->Write(buffer,sizeof(buffer));
    offset+=static_cast<int>(sizeof(buffer));
  }
  input->Read(buffer,length-offset);
  lzss->Write(buffer,length-offset);
  delete lzss;
  delete arithmetic;
  delete output;
  delete input;
  timer.Stop();
  idLib::Printf("%1.0f milliseconds to compress the file\n",
                static_cast<float>(timer.Milliseconds()));
}

void idCommonLocal::DecompressFile_f(const idCmdArgs* args) {
  idStr name(args->Argv(1));
  idTimer timer;
  timer.Start();
  name.SetFileExtension("zid");
  idFile* input=fileSystem->OpenFileReadRetail(name.c_str(),true,false);
  if(input==nullptr) return;
  idFile* output=fileSystem->OpenFileWriteRetail(args->Argv(1),FSPATH_SAVE);
  if(output==nullptr) return;

  idCompressor* arithmetic=idCompressor::AllocArithmetic();
  arithmetic->Init(input,false,8);
  idCompressor* lzss=idCompressor::AllocLZSS_ByteAligned();
  lzss->Init(arithmetic,false,8);
  int length;
  lzss->Read(&length,sizeof(length));
  unsigned char buffer[65536];
  int offset=0;
  while(offset<length-65536) {
    input->Read(buffer,sizeof(buffer));
    lzss->Write(buffer,sizeof(buffer));
    offset+=static_cast<int>(sizeof(buffer));
  }
  input->Read(buffer,length-offset);
  lzss->Write(buffer,length-offset);
  delete lzss;
  delete arithmetic;
  delete output;
  delete input;
  timer.Stop();
  idLib::Printf("%1.0f milliseconds to decompress the file\n",
                static_cast<float>(timer.Milliseconds()));
}

template<>
std::size_t idFile::ReadBig<std::uint64_t>(std::uint64_t& value) {
  const std::size_t result=Read(&value,sizeof(value));
  value=((value&0x00000000000000FFULL)<<56) |
        ((value&0x000000000000FF00ULL)<<40) |
        ((value&0x0000000000FF0000ULL)<<24) |
        ((value&0x00000000FF000000ULL)<<8) |
        ((value&0x000000FF00000000ULL)>>8) |
        ((value&0x0000FF0000000000ULL)>>24) |
        ((value&0x00FF000000000000ULL)>>40) |
        ((value&0xFF00000000000000ULL)>>56);
  return result;
}

namespace {
std::uint32_t Big32(std::uint32_t value) {
  return ((value&0x000000FFU)<<24) | ((value&0x0000FF00U)<<8) |
         ((value&0x00FF0000U)>>8) | ((value&0xFF000000U)>>24);
}
std::uint64_t Big64(std::uint64_t value) {
  return ((value&0x00000000000000FFULL)<<56) |
         ((value&0x000000000000FF00ULL)<<40) |
         ((value&0x0000000000FF0000ULL)<<24) |
         ((value&0x00000000FF000000ULL)<<8) |
         ((value&0x000000FF00000000ULL)>>8) |
         ((value&0x0000FF0000000000ULL)>>24) |
         ((value&0x00FF000000000000ULL)>>40) |
         ((value&0xFF00000000000000ULL)>>56);
}
}

void idCommonLocal::RecordSnapshotFile(idFile* file,unsigned int tag) {
  if(file==nullptr) idLib::Error("Snapshot record failed because of NULL file.");
  idLib::Printf("Recording %s\n",file->GetName());
  Game()->GetRenderWorld()->GenerateAllInteractions();
  if(tag==0x44454D4FU) {
    Field<int>(0x6D0)=session->GetActingGameStateLobbyBase().IsHost() ||
        !session->GetActingGameStateLobbyBase().HasActivePeers() ? 1 : 2;
  } else {
    Field<int>(0x6D0)=4;
  }

  void* memory=idMem::Singleton()->AllocWithLocation(
      __FILE__,sizeof(idSnapShot),static_cast<memTag_t>(57),false,ALIGN_16,
      HEAP_DEFAULTHEAP);
  Field<idSnapShot*>(0x718)=memory!=nullptr ? new(memory) idSnapShot() : nullptr;
  Field<std::uint32_t>(0x720)=0;
  Field<idFile*>(0x710)=file;
  Field<idCompressor*>(0x708)=idCompressor::AllocLZW();
  Field<idCompressor*>(0x708)->Init(Field<idFile*>(0x710),true,8);
  Field<idCompressor*>(0x700)=idCompressor::AllocRunLength_ZeroBased();
  Field<idCompressor*>(0x700)->Init(Field<idCompressor*>(0x708),true,8);
  idCompressor* compressor=Field<idCompressor*>(0x700);

  std::uint32_t word=Big32(tag);
  compressor->Write(&word,sizeof(word));
  word=Big32(3);
  compressor->Write(&word,sizeof(word));

  const idGameSpawnInfo* spawnInfo=Game()->GetGameSpawnInfo();
  compressor->WriteString(spawnInfo->mapName.c_str());
  std::uint8_t flag=spawnInfo->comboMap;
  compressor->Write(&flag,sizeof(flag));
  flag=spawnInfo->devmap;
  compressor->Write(&flag,sizeof(flag));
  word=Big32(static_cast<std::uint32_t>(spawnInfo->numPlayers));
  compressor->Write(&word,sizeof(word));

  word=Big32(static_cast<std::uint32_t>(spawnInfo->activeLayers.Num()));
  compressor->Write(&word,sizeof(word));
  for(int index=0;index<spawnInfo->activeLayers.Num();++index) {
    const idStr& layer=spawnInfo->activeLayers[index];
    const int length=layer.Length();
    compressor->Write(&length,sizeof(length));
    compressor->Write(layer.c_str(),length);
  }
  word=Big32(static_cast<std::uint32_t>(spawnInfo->deactiveLayers.Num()));
  compressor->Write(&word,sizeof(word));
  for(int index=0;index<spawnInfo->deactiveLayers.Num();++index) {
    const idStr& layer=spawnInfo->deactiveLayers[index];
    const int length=layer.Length();
    compressor->Write(&length,sizeof(length));
    compressor->Write(layer.c_str(),length);
  }
  word=Big32(static_cast<std::uint32_t>(spawnInfo->gameSpawnFlags));
  compressor->Write(&word,sizeof(word));
  word=Big32(static_cast<std::uint32_t>(spawnInfo->currentCheckpointSlot));
  compressor->Write(&word,sizeof(word));

  std::uint64_t wide=Big64(Field<std::uint64_t>(0x728));
  compressor->Write(&wide,sizeof(wide));
  const idMatchParameters& match=
      session->GetActingGameStateLobbyBase().GetMatchParms();
  word=Big32(static_cast<std::uint32_t>(match.gameMap));
  compressor->Write(&word,sizeof(word));
  word=Big32(static_cast<std::uint32_t>(match.gameMode));
  compressor->Write(&word,sizeof(word));
  Field<std::uint64_t>(0x750)=0;
  Field<float>(0x34660)=0.0f;
  Field<std::uint64_t>(0x738)=0;
}

void idCommonLocal::RecordDemo_f(const idCmdArgs* args) {
  idStr demoName(args->Argv(1));
  if(demoName.IsEmpty()) {
    int lastNumber=0;
    fileSystem->GetNumberedFilenameRetail(
        lastNumber,"demos/demo","demo",demoName,true);
  } else {
    demoName="demos/"+demoName;
  }
  demoName.SetFileExtension("demo");
  if(commonLocal.Field<int>(0x6D0)!=0) {
    commonLocal.StopSnapshotFilePlayback();
    commonLocal.StopSnapshotFileRecord();
    commonLocal.Game()->OnCloseSnapshot();
  }
  idFile* file=fileSystem->OpenFileWriteRetail(demoName.c_str(),FSPATH_SAVE);
  commonLocal.RecordSnapshotFile(file,0x44454D4FU);
  if(session->GetActingGameStateLobbyBase().IsHost() ||
     !session->GetActingGameStateLobbyBase().HasActivePeers())
    commonLocal.Field<int>(0x6D0)=1;
  else
    commonLocal.Field<int>(0x6D0)=2;
}

void idCommonLocal::DeleteGenerated_f(const idCmdArgs*) {
  idFileList* files=fileSystem->ListFilesTreeRetail("generated","*.*",false);
  idLib::Printf("Removing generated files...\n");
  for(int index=0;index<files->list.Num();++index) {
    idLib::Printf("Deleting %s\n",files->list[index].c_str());
    fileSystem->RemoveFileRetail(files->list[index].c_str(),FSPATH_BASE);
  }
  idLib::Printf("done.\n");
  fileSystem->FreeFileListRetail(files);
}

void idCommonLocal::Map_f(const idCmdArgs* args) {
  bool comboMap=false;
  bool devmap=false;
  bool forceHeapReset=false;
  bool checkpointLoad=false;
  bool nextMapDiscSwap=false;
  int checkpointSlot=-1;
  idGame* game=commonLocal.Game();
  bool playVideo=false;

  idSignInManagerBase& signIn=session->GetSignInManager();
  if(signIn.GetNumLocalUsers()<=0 || signIn.GetLocalUserByIndex(0)==nullptr) {
    signIn.SetDesiredLocalUsers(1,1);
    session->GetSignInManager().Pump();
  }
  for(int index=2;index<args->argc;++index) {
    const char* option=args->Argv(index);
    if(idStr::Icmp(option,"combo")==0) comboMap=true;
    else if(idStr::Icmp(option,"dev")==0) devmap=true;
    else if(idStr::Icmp(option,"reset")==0) forceHeapReset=true;
    else if(idStr::Icmp(option,"check")==0) {
      checkpointLoad=true;
      comboMap=true;
    } else if(idStr::Icmp(option,"nextMapDiscSwap")==0) {
      nextMapDiscSwap=true;
    }
  }

  idStr mapExt("map");
  idStr map(args->Argv(1));
  map.ToLower();
  if(map.Right(8)=="entities") mapExt="entities";
  else map.StripFileExtension();

  if(game!=nullptr && !checkpointLoad) {
    game->CheckpointCapFileData(-1);
  } else if(checkpointLoad) {
    checkpointSlot=std::atoi(args->Argv(1));
    if(checkpointSlot==-1)
      checkpointSlot=game->CheckpointGetActiveSlots()-1;
    playVideo=std::atoi(args->Argv(2))!=0;
    idGameSpawnInfo checkpointInfo("",0);
    if(game->CheckpointGetSpawninfoFromSlot(checkpointSlot,checkpointInfo)) {
      si_spawnspot.SetString(checkpointInfo.GetSpawnSpot().c_str(),true);
      idStr active;
      idStr deactive;
      checkpointInfo.WriteLayersToCommaSeparatedStrings(active,deactive);
      si_nextlayeractive.SetString(active.c_str(),true);
      si_nextlayerdeactive.SetString(deactive.c_str(),true);
      map=checkpointInfo.mapName;
      map.ToLower();
      map.StripFileExtension();
    } else {
      idLib::Printf("idCommonLocal::Map_f map doesnt have valid data");
    }
  }

  discSwapMgr->Reset();
  discSwapMgr->map=map;
  discSwapMgr->additionalArguments=args->Args(0,-1,false,false);
  discSwapMgr->layerData=si_nextlayeractive.valueString;
  discSwapMgr->spawnSpot=si_spawnspot.valueString;
  if(discSwapMgr->Swap(idDiscSwapManager::DSC_MAP,0,true)!=
     idDiscSwapManager::ERROR_NONE)
    return;

  if(!session->CheckMapInstallAndShowGui(map.c_str(),true,true)) {
    session->MoveToPressStart();
    return;
  }

  if(idLib::production==0) {
    idStr path(va("maps/%s",map.c_str()));
    path.SetFileExtension(mapExt.c_str());
    if(!fileSystem->FileExistsRetail(path.c_str(),false)) {
      idLib::Printf("Can't find map '%s'\n",path.c_str());
      if(mapExt=="entities")
        idLib::Printf("Use the writeEntitiesFile command to convert the map to an entities file.\n");
      return;
    }
    if(comboMap) {
      idStr regionMap(map);
      regionMap.StripFileExtension();
      regionMap.StripPath();
      path.Format("maps/%s/_combo/%s.proc",map.c_str(),regionMap.c_str());
      if(!fileSystem->FileExistsRetail(path.c_str(),false))
        idLib::Error("You need to build the combo map for: %s",map.c_str());
    } else {
      idStr regionMap(va("maps/%s",map.c_str()));
      regionMap.StripFileExtension();
      regionMap+="_region.map";
      if(fileSystem->FileExistsRetail(regionMap.c_str(),false)) {
        idLib::Printf("Loading region map: %s\n",map.c_str());
        map=va("%s_region",map.c_str());
      }
    }
  }

  const idStr loadingScreen("");
  const idStr spawnSpot(si_spawnspot.valueString);
  const idStr deactive(si_nextlayerdeactive.valueString);
  const idStr active(si_nextlayeractive.valueString);
  idMatchParameters matchParms(
      -1,-1,-1,0,16,0,map,active,deactive,spawnSpot,loadingScreen,
      si_minRespawnDelay.valueFloat,si_timeLimit.valueFloat,
      si_scoreLimit.valueInteger,si_teamCount.valueInteger,
      si_swapTeams.valueInteger!=0,si_stopWatch.valueInteger!=0,
      comboMap,devmap,false,forceHeapReset,checkpointSlot,playVideo);
  matchParms.nextMapDiscSwap=nextMapDiscSwap;
  session->ClearSessionOption(-1);
  session->CreateMatch(matchParms);
  WaitOnSession(5,-1.0f,0);
  session->StartMatch();
  WaitOnSession(6,-1.0f,0);
  si_nextlayeractive.SetString("",true);
  si_nextlayerdeactive.SetString("",true);
  if(si_spawnspot.valueString.IsEmpty())
    si_spawnspot.SetString("player_start",true);
  fileSystem->ClearZipCacheRetail();
}

void idCommonLocal::DevMap_f(const idCmdArgs* args) {
  si_spawnspot.SetString("player_start_1",true);
  idCmdArgs copy(*args);
  copy.AppendArg("dev");
  Map_f(&copy);
}

void idCommonLocal::ComboMap_f(const idCmdArgs* args) {
  idCmdArgs copy(*args);
  copy.AppendArg("combo");
  Map_f(&copy);
}

void idCommonLocal::DevComboMap_f(const idCmdArgs* args) {
  idCmdArgs copy(*args);
  copy.AppendArg("dev");
  copy.AppendArg("combo");
  Map_f(&copy);
}

void idCommonLocal::CheckLoad_f(const idCmdArgs* args) {
  idCmdArgs copy(*args);
  copy.AppendArg("check");
  Map_f(&copy);
}

void idCommonLocal::NextMap_f(const idCmdArgs* args) {
  if(commonLocal.Game()->IsGameActive(true)) {
    if(args->argc==7) {
      idGameSpawnInfo gameSpawnInfo("",0);
      if(commonLocal.Game()->NextMap(*args,gameSpawnInfo))
        commonLocal.ExecuteMapChange(gameSpawnInfo,nullptr);
    } else {
      idLib::Printf("syntax: nextMap <shouldWeSave - 0, 1> <mapnameTarget> <spawnspotTarget> <loadingScreen> <layersActive> <layersDeactive>\n");
    }
  } else {
    idLib::Printf("nextMap only works if a map is running!");
  }
}
