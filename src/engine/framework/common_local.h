#pragma once

#include "engine/framework/common.h"
#include "engine/framework/game_network.h"
#include "engine/framework/usercmd.h"
#include "engine/framework/usercmdmgr.h"
#include "engine/network/snapshot.h"
#include "shared/idlib/containers/hashindex.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/parallelism/thread.h"
#include "shared/idlib/text/str.h"

#include <cstddef>
#include <cstdint>

class idBitMsg;
class idSerializer;
class idSWF;
class idSWFHelper;
class idRenderModelGui;
class idGameSpawnInfo;
struct sysEvent_t;
class idDialogInfo;

struct idUniqueWarning {
  idUniqueWarning() : message(),count(0) {}
  explicit idUniqueWarning(const char* text) : message(text),count(1) {}
  explicit idUniqueWarning(const idStr& text) : message(text),count(1) {}
  idStr message;
  int count;
};

class idCommonLocal : public idCommon {
  friend class idRenderManager;
public:
  class idLogFileWriter : public idPrintListener {
  public:
    idLogFileWriter() : logFile(nullptr),recursing(false),lock() {}
    ~idLogFileWriter() override { CloseLogFile(); }
    void Print(const char*) override;
    void Init() { logFile=nullptr;recursing=false; }
    void CloseLogFile();
    idFile* GetFile() { return logFile; }
    idFile* logFile;
    bool recursing;
    idSysMutex lock;
  };

  class idWarningList : public idPrintListener {
  public:
    idWarningList() : list(),hash(1024,1024),caption() {}
    ~idWarningList() override=default;
    void Print(const char*) override;
    idList<idUniqueWarning,3> list;
    idHashIndex hash;
    idStrStatic<128> caption;
  };

  class idRefreshOnPrint : public idPrintListener {
  public:
    idRefreshOnPrint() : cl(nullptr),set(false) {}
    ~idRefreshOnPrint() override=default;
    void Print(const char*) override;
    idCommonLocal* cl;
    bool set;
  };

  void SetRefreshOnPrint(bool) override;
  void PrintWarnings() override;
  void DumpWarnings(const char*);
  void ClearWarnings(const char*) override;
  idGame* Game() override;
  bool IsLosingConnectionToHost() override;
  bool AreWeRunningPC() const override;
  bool IsInitialized() const override;
  void UpdateLevelLoadPacifier(float,pacifierState_t,bool) override;
  recordState_t GetRecordState() override;
  bool HasDialogMsg(gameDialogMessages_t,bool*) override;
  void ClearDialog(gameDialogMessages_t,const char*,int) override;
  void AddDialog(gameDialogMessages_t,dialogType_t,idSWFScriptFunction*,
                 idSWFScriptFunction*,bool,const char*,int,bool,bool,bool,int) override;
  void AddDynamicDialog(gameDialogMessages_t,
      const idStaticList<idSWFScriptFunction*,4>&,
      const idStaticList<idStrId,4>&,bool,idStrStatic<256>,bool,bool,bool) override;
  void AddDialogWithTextOverride(gameDialogMessages_t,dialogType_t,
      const idStaticList<idSWFScriptFunction*,4>&,
      const idStaticList<idStrId,4>&,bool,idStrStatic<256>,bool,bool,bool) override;
  void AddDialogIntVal(const char*,int) override;
  void ClearDialogs(bool) override;
  bool IsDialogActive() override;
  idStr GetDialogMsg(gameDialogMessages_t);
  void RemoveWaitDialogs();
  void ReleaseCallBacks(int);
  bool HandleDialogEvent(const sysEvent_t*);
  void ShowDialog(idDialogInfo&,bool);
  void KillDialog();
  void ShowNextDialog(bool);
  void InitDialog();
  void RenderDialog(bool);
  void AddDialogInternal(idDialogInfo&);
  void DrawSnapshotMetrics(bool);
  void LoadingScreenSetup(idSWF*,idRenderModelGui*,const idGameSpawnInfo&);

private:
  void NetReceiveReliable(int,idBitMsg&,int) override;
  void NetReceiveSnapshot(idSnapShot&) override;
  void NetReceiveUsercmds(idBitMsg&) override;
  void WriteUserCmdDelta(idBitMsg&,const usercmd_t&,const usercmd_t*);
  void ReadUserCmdDelta(const idBitMsg&,usercmd_t&,const usercmd_t*);
  void NetSendUsercmds();
  void SerializePlayer(idSerializer&,gameReturnPlayer_t&);
  void InterpolateSnapshot(gameReturn_t&,gameReturn_t&,gameReturn_t&,float,bool);
  void ProcessSnapshot(idSnapShot&);
  void ProcessNextSnapshot();

  template <typename T> T& Field(std::size_t offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<std::byte*>(this)+offset);
  }
  template <typename T> const T& Field(std::size_t offset) const {
    return *reinterpret_cast<const T*>(reinterpret_cast<const std::byte*>(this)+offset);
  }
  gameReturn_t& SnapCurrent() { return Field<gameReturn_t>(0x760); }
  gameReturn_t& SnapPrevious() { return Field<gameReturn_t>(0x1A6E0); }
  idSnapShot* ReceivedSnaps() { return &Field<idSnapShot>(0x34738); }
  idSnapShot& OldSnapshot() { return Field<idSnapShot>(0x34C58); }
  idUserCmdMgr& UserCmdMgr() { return Field<idUserCmdMgr>(0x34D68); }
  idSWF*& Dialog() { return Field<idSWF*>(0x5AE10); }
  bool& DialogWingstickActive() { return Field<bool>(0x5AE0C); }
  bool& DialogPause() { return Field<bool>(0x5AE0D); }
  idRenderModelGui*& DialogGuiModel() { return Field<idRenderModelGui*>(0x5AE20); }
  idStaticList<idDialogInfo,4>& MessageList() {
    return Field<idStaticList<idDialogInfo,4>>(0x5AE28);
  }
  idSWFHelper*& SwfHelper() { return Field<idSWFHelper*>(0x34CC0); }
  bool& PreventOverlayPurge() { return Field<bool>(0x34721); }

public:
#if defined(WOLF_COMMON_NETWORK_VALIDATION)
  void ValidationWriteUserCmdDelta(idBitMsg& msg,const usercmd_t& command,
                                   const usercmd_t* base) {
    WriteUserCmdDelta(msg,command,base);
  }
  void ValidationReadUserCmdDelta(const idBitMsg& msg,usercmd_t& command,
                                  const usercmd_t* base) {
    ReadUserCmdDelta(msg,command,base);
  }
  void ValidationSerializePlayer(idSerializer& serializer,gameReturnPlayer_t& player) {
    SerializePlayer(serializer,player);
  }
#endif
  idLogFileWriter logFileWriter;
  idWarningList warningList;
  idRefreshOnPrint refreshOnPrint;
  std::byte opaqueTail[375504-352]{};
};

extern idCommonLocal commonLocal;

static_assert(sizeof(idPrintListener)==24,"idPrintListener ABI drift");
static_assert(sizeof(idUniqueWarning)==48,"idUniqueWarning ABI drift");
static_assert(sizeof(idCommonLocal::idLogFileWriter)==48,"idLogFileWriter ABI drift");
static_assert(sizeof(idCommonLocal::idWarningList)==256,"idWarningList ABI drift");
static_assert(sizeof(idCommonLocal::idRefreshOnPrint)==40,"idRefreshOnPrint ABI drift");
static_assert(offsetof(idCommonLocal,logFileWriter)==8,"idCommonLocal::logFileWriter ABI drift");
static_assert(offsetof(idCommonLocal,warningList)==56,"idCommonLocal::warningList ABI drift");
static_assert(offsetof(idCommonLocal,refreshOnPrint)==312,"idCommonLocal::refreshOnPrint ABI drift");
static_assert(sizeof(idCommonLocal)==375504,"idCommonLocal ABI drift");
