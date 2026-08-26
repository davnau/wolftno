#pragma once

#include "engine/framework/dialog.h"

#include <cstddef>

class idFile;
class idBitMsg;
class idGame;
class idPlayerProfile;
class idRenderVideoOverlay;
class idSnapShot;
class idSWFScriptFunction;
struct gameFrameParms_t { std::byte data[88]; };
enum recordState_t : int { RECORD_NONE=0 };
enum pacifierState_t : int { PACIFIER_STATE_0=0, PACIFIER_STATE_1=1 };

class idCommon {
public:
  virtual ~idCommon()=default;                                      // 0
  virtual void CommonSlot01(){} virtual void CommonSlot02(){}
  virtual void CommonSlot03(){}
  virtual bool IsInitialized() const {return false;}                // 4
  virtual void BeginTerminationThread(){}                          // 5
  virtual void EndTerminationThread(){}                            // 6
  virtual bool IsTerminationThreadStopped(){return false;}         // 7
  virtual void CommonSlot08(){} virtual void CommonSlot09(){}
  virtual void CommonSlot10(){} virtual void CommonSlot11(){}
  virtual void CommonSlot12(){} virtual void CommonSlot13(){}
  virtual void CommonSlot14(){} virtual void CommonSlot15(){}
  virtual void CommonSlot16(){}
  virtual void SetRefreshOnPrint(bool){}                            // 17
  virtual void PrintWarnings(){}                                   // 18
  virtual void ClearWarnings(const char*){}                        // 19
  virtual void UpdateConsoleDisplay(bool){}                        // 20
  virtual void UpdateLevelLoadPacifier(float,pacifierState_t,bool){} // 21
  virtual void CommonSlot22(){}
  virtual void CommonSlot23(){} virtual void CommonSlot24(){}
  virtual idPlayerProfile* GetProfileFromMasterLocalUser(){return nullptr;} // 25
  virtual void CommonSlot26(){} virtual void CommonSlot27(){}
  virtual void CommonSlot28(){} virtual void CommonSlot29(){}
  virtual bool AreWeRunningPC() const {return false;}               // 30
  virtual void CommonSlot31(){}
  virtual void CommonSlot32(){} virtual void CommonSlot33(){}
  virtual void CommonSlot34(){}
  virtual bool IsInGame(){return false;}                            // 35
  virtual void CommonSlot36(){} virtual void CommonSlot37(){}
  virtual void CommonSlot38(){} virtual void CommonSlot39(){}
  virtual void CommonSlot40(){}
  virtual bool IsServer(){return false;}                            // 41
  virtual bool IsClient(){return false;}                            // 42
  virtual void CommonSlot43(){} virtual void CommonSlot44(){}
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
  virtual void CommonSlot55(){} virtual void CommonSlot56(){}
  virtual int GetFocusDeviceNum(){return -1;}                       // 57
  virtual void CommonSlot58(){} virtual void CommonSlot59(){}
  virtual void CommonSlot60(){} virtual void CommonSlot61(){}
  virtual void CommonSlot62(){}
  virtual idGame* Game(){return nullptr;}                           // 63
  virtual gameFrameParms_t GameFrameParms(const int){return {};}   // 64
  virtual void CommonSlot65(){}
  virtual void CommonSlot66(){} virtual void CommonSlot67(){}
  virtual void CommonSlot68(){}
  virtual recordState_t GetRecordState(){return RECORD_NONE;}       // 69
  virtual void CommonSlot70(){} virtual void CommonSlot71(){}
  virtual void CommonSlot72(){} virtual void CommonSlot73(){}
  virtual void CommonSlot74(){} virtual void CommonSlot75(){}
  virtual void CommonSlot76(){} virtual void CommonSlot77(){}
  virtual void CommonSlot78(){} virtual void CommonSlot79(){}
  virtual void CommonSlot80(){} virtual void CommonSlot81(){}
  virtual void CommonSlot82(){} virtual void CommonSlot83(){}
  virtual void CommonSlot84(){}
  virtual void NetReceiveReliable(int,idBitMsg&,int){}              // 85
  virtual void NetReceiveSnapshot(idSnapShot&){}                    // 86
  virtual void NetReceiveUsercmds(idBitMsg&){}                      // 87
  virtual int GetSnapRate(){return 0;}                              // 88
  virtual bool IsLosingConnectionToHost(){return false;}            // 89
  virtual idRenderVideoOverlay* VideoOverlay(){return nullptr;}     // 90
  virtual void CommonSlot91(){} virtual void CommonSlot92(){}
  virtual idFile* GetLogfile(){return nullptr;}                     // 93
  virtual void CommonSlot94(){} virtual void CommonSlot95(){}
  virtual void CommonSlot96(){} virtual void CommonSlot97(){}
  virtual void CommonSlot98(){} virtual void CommonSlot99(){}
  virtual void CommonSlot100(){} virtual void CommonSlot101(){}
  virtual void CommonSlot102(){} virtual void CommonSlot103(){}
};

extern idCommon* common;
static_assert(sizeof(idCommon)==8,"idCommon ABI drift");
static_assert(sizeof(gameFrameParms_t)==88,"gameFrameParms_t ABI drift");
