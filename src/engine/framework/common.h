#pragma once

#include "engine/framework/dialog.h"
#include "shared/idlib/text/str.h"

#include <cstddef>

class idFile;
class idBitMsg;
class idGame;
class idGameSpawnInfo;
class idPlayerProfile;
class idRenderVideoOverlay;
class idRenderWorld;
class idSoundWorld;
class idRegression;
class idMapFile;
class idMapChangeListener {
public:
  virtual void MapLoaded(idMapFile*)=0;
  virtual void MapDisconnected()=0;
};
class idLocalUser;
class idUserCmdMgr;
class idSnapShot;
class idSWFScriptFunction;
class idSWFHelper;
#ifndef ID_MILLISECOND_UNIQUE_T_DEFINED
#define ID_MILLISECOND_UNIQUE_T_DEFINED
struct millisecondUnique_t { int value{0}; };
#endif
enum recordState_t : int { RECORD_NONE=0, RECORD_MAKE=1, RECORD_PLAY=2 };
enum snapshotState_t : int { SNAP_NONE=0, SNAP_READING=1, SNAP_WRITING=2 };
enum pacifierState_t : int { PACIFIER_STATE_0=0, PACIFIER_STATE_1=1 };
enum toolFlag_t : int { EDITOR_PARTICLE=0x400 };

struct gameFrameParms_t {
  gameFrameParms_t();
  gameFrameParms_t(const gameFrameParms_t&);
  ~gameFrameParms_t();
  gameFrameParms_t& operator=(const gameFrameParms_t&)=default;
  int renderWidth;
  int renderHeight;
  float pixelAspect;
  bool isServer;
  bool isClient;
  bool isHeadlessServer;
  bool isDemoPlaying;
  bool isDemoRecording;
  bool isToolActive;
  bool advanceTime;
  bool editorRequestsStopTime;
  recordState_t recordState;
  idLocalUser* masterLocalUser;
  idRegression* regression;
  idUserCmdMgr* userCmdMgr;
  idStr currentEditorReferenceMapID;
};

class idCommon {
public:
  virtual ~idCommon()=default;                                      // 0
  virtual void Init(int,const char**,const char*){}                 // 1
  virtual void Quit(int){}                                         // 2
  virtual void Shutdown(int){}                                     // 3
  virtual bool IsInitialized() const {return false;}                // 4
  virtual void BeginTerminationThread(){}                          // 5
  virtual void EndTerminationThread(){}                            // 6
  virtual bool IsTerminationThreadStopped(){return false;}         // 7
  virtual void SyncAllBackgroundOperations(bool){}                 // 8
  virtual void CommonSlot09(){}
  virtual void InitTool(toolFlag_t){} virtual void UpdateTool(toolFlag_t){}
  virtual void GetToolReferenceMap(toolFlag_t,idStr&){}            // 12
  virtual void ActivateTool(bool){}                               // 13
  virtual void RegisterMapChangeListener(idMapChangeListener*){}   // 14
  virtual void UnRegisterMapChangeListener(idMapChangeListener*){} // 15
  virtual void WriteConfigToFile(const char*){}                    // 16
  virtual void SetRefreshOnPrint(bool){}                            // 17
  virtual void PrintWarnings(){}                                   // 18
  virtual void ClearWarnings(const char*){}                        // 19
  virtual void UpdateConsoleDisplay(bool){}                        // 20
  virtual void UpdateLevelLoadPacifier(float,pacifierState_t,bool,bool=false){} // 21
  virtual millisecondUnique_t PlayLoadSound()=0;                   // 22
  virtual void SetLoadScreenInfo(const idStr&,const idStr&){}      // 23
  virtual const char* GetLoadMaterialForMission(const char*,bool){return "";} // 24
  virtual idPlayerProfile* GetProfileFromMasterLocalUser(){return nullptr;} // 25
  virtual int GetMasterLocalUserInputDevice(){return -1;}           // 26
  virtual int GetMasterLocalUserPlayerId(){return 0;}               // 27
  virtual idLocalUser* GetMasterLocalUser(){return nullptr;}       // 28
  virtual idSWFHelper* GetSWFHelper(){return nullptr;}              // 29
  virtual bool AreWeRunningPC() const {return false;}               // 30
  virtual void CommonSlot31(){}
  virtual void CommonSlot32(){} virtual bool IsMainMenuActive(){return false;} // 33
  virtual bool IsGamePaused(){return false;}                       // 34
  virtual bool IsInGame(){return false;}                            // 35
  virtual void CommonSlot36(){} virtual void CommonSlot37(){}
  virtual void CommonSlot38(){} virtual snapshotState_t SnapshotState(){return SNAP_NONE;} // 39
  virtual void CommonSlot40(){}
  virtual bool IsServer(){return SnapshotState()!=SNAP_READING;}   // 41
  virtual bool IsClient(){return SnapshotState()==SNAP_READING;}   // 42
  virtual bool SnapshotsAreStalled(){return false;}                // 43
  virtual bool IsMultiplayer() const {return false;}                // 44
  virtual bool IsToolActive(){return false;}                        // 45
  virtual bool AllowCheats(){return false;}                         // 46
  virtual void AddDialog(gameDialogMessages_t,dialogType_t,
                         idSWFScriptFunction*,idSWFScriptFunction*,bool,
                         const char*,int,bool,bool,bool,int){}      // 47
  virtual void AddDynamicDialog(gameDialogMessages_t,
      const idStaticList<idSWFScriptFunction*,4>&,
      const idStaticList<idStrId,4>&,bool,idStrStatic<256>,
      bool,bool,bool){}                                            // 48
  virtual void AddDialogWithTextOverride(gameDialogMessages_t,dialogType_t,
      const idStaticList<idSWFScriptFunction*,4>&,
      const idStaticList<idStrId,4>&,bool,idStrStatic<256>,
      bool,bool,bool){}                                            // 49
  virtual void AddDialogIntVal(const char*,int){}                  // 50
  virtual void ClearDialogs(bool){}                               // 51
  virtual bool HasDialogMsg(gameDialogMessages_t,bool*){return false;} // 52
  virtual void ClearDialog(gameDialogMessages_t,const char*,int){}     // 53
  virtual bool IsDialogActive(){return false;}                      // 54
  virtual bool IsDialogPausing(){return false;}                    // 55
  virtual int GetRecentInputDevice(){return -1;}                   // 56
  virtual int GetFocusDeviceNum(){return -1;}                       // 57
  virtual void SetFocusDeviceNum(int){}                            // 58
  virtual int GetPauseMenuDeviceNum(){return -1;}                  // 59
  virtual void SetPauseMenuDeviceNum(int){}                        // 60
  virtual bool GetAutosaveEnabled(){return false;}                 // 61
  virtual void EnableAutosaves(bool){}                             // 62
  virtual idGame* Game(){return nullptr;}                           // 63
  virtual gameFrameParms_t GameFrameParms(const int){return {};}   // 64
  virtual idRenderWorld* RW(){return nullptr;}                     // 65
  virtual idSoundWorld* SW(){return nullptr;}                      // 66
  virtual void ExecuteMapChange(const idGameSpawnInfo&,idFile*){}  // 67
  virtual void CommonSlot68(){}
  virtual recordState_t GetRecordState(){return RECORD_NONE;}       // 69
  virtual idRegression* GetRegression(){return nullptr;}           // 70
  virtual void CommonSlot71(){}
  virtual void CommonSlot72(){} virtual void CommonSlot73(){}
  virtual void CommonSlot74(){} virtual void CommonSlot75(){}
  virtual void CommonSlot76(){} virtual void CommonSlot77(){}
  virtual void CommonSlot78(){} virtual bool IsDemoPlaying(){return false;} // 79
  virtual bool IsDemoRecording(){return false;}                    // 80
  virtual void CommonSlot81(){}
  virtual void CommonSlot82(){} virtual void CommonSlot83(){}
  virtual void CommonSlot84(){}
  virtual void NetReceiveReliable(int,idBitMsg&,int){}              // 85
  virtual void NetReceiveSnapshot(idSnapShot&){}                    // 86
  virtual void NetReceiveUsercmds(idBitMsg&){}                      // 87
  virtual int GetSnapRate(){return 0;}                              // 88
  virtual bool IsLosingConnectionToHost(){return false;}            // 89
  virtual idRenderVideoOverlay* VideoOverlay(){return nullptr;}     // 90
  virtual void CommonSlot91(){} virtual bool IsPurgeOverlaysPending(){return false;} // 92
  virtual idFile* GetLogfile(){return nullptr;}                     // 93
  virtual void CommonSlot94(){} virtual void CommonSlot95(){}
  virtual void CommonSlot96(){} virtual void CommonSlot97(){}
  virtual void CommonSlot98(){} virtual bool ShouldDelayShutdown(){return false;} // 99
  virtual void SetShouldDelayShutdown(bool){}                      // 100
  virtual bool GetAbnormalFileRead(){return false;}                // 101
  virtual void SetAbnormalFileRead(bool){}                         // 102
  virtual void CommonSlot103(){}
};

extern idCommon* common;
static_assert(sizeof(idCommon)==8,"idCommon ABI drift");
static_assert(sizeof(gameFrameParms_t)==88,"gameFrameParms_t ABI drift");
