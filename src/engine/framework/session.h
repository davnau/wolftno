#pragma once

// NEEDS_WORK(engine/framework/session): PDB-exact interface slice required by
// renderer/renderthread.cpp.  The full session implementation remains owned by
// the framework/session recovery pass.

class idSaveGameManager;
class idBitMsg;
class idRenderModelGui;
class idFile_SaveGame;
class idSaveGameDetails;
template <typename T,int Capacity> class idStaticList;

class idLobbyBase {
public:
  const void* const* vtable;
  bool IsPeer() {
    using Fn=bool(*)(idLobbyBase*);
    return reinterpret_cast<Fn>(vtable[2])(this);
  }
  bool HasActivePeers() {
    using Fn=bool(*)(idLobbyBase*);
    return reinterpret_cast<Fn>(vtable[3])(this);
  }
  void DrawDebugNetworkHUD_ServerSnapshotMetrics(idRenderModelGui* gui,bool draw) {
    using Fn=void(*)(idLobbyBase*,idRenderModelGui*,bool);
    reinterpret_cast<Fn>(vtable[51])(this,gui,draw);
  }
  void DrawDebugNetworkHUD(idRenderModelGui* gui) {
    using Fn=void(*)(idLobbyBase*,idRenderModelGui*);
    reinterpret_cast<Fn>(vtable[49])(this,gui);
  }
};

class idSession {
public:
  enum sessionGoToStoreState_t : int {
    GO_TO_STORE_INVALID = 0,
    GO_TO_STORE_WAITING_FOR_ANSWER = 1,
    GO_TO_STORE_YES = 2,
    GO_TO_STORE_NO = 3,
  };

  const void* const* vtable;

  bool IsLosingConnectionToHost() {
    using Fn=bool(*)(idSession*);
    return reinterpret_cast<Fn>(vtable[28])(this);
  }

  idLobbyBase& GetActingGameStateLobbyBase() {
    using Fn=idLobbyBase&(*)(idSession*);
    return reinterpret_cast<Fn>(vtable[45])(this);
  }

  int GetState() {
    using Fn=int(*)(idSession*);
    return reinterpret_cast<Fn>(vtable[60])(this);
  }

  int GetNumContentPackages() {
    using Fn = int (*)(idSession*);
    return reinterpret_cast<Fn>(vtable[75])(this);
  }

  const char* GetContentPackagePath(int index) {
    using Fn = const char* (*)(idSession*, int);
    return reinterpret_cast<Fn>(vtable[77])(this, index);
  }

  void SetGoToStoreState(sessionGoToStoreState_t state) {
    using Fn=void(*)(idSession*,sessionGoToStoreState_t);
    reinterpret_cast<Fn>(vtable[86])(this,state);
  }

  int SaveGameSync(const char* folder,
                   const idStaticList<idFile_SaveGame*,10>& files,
                   const idSaveGameDetails& details,bool skipErrorDialog,
                   bool waitForAlreadyInProgress) {
    using Fn=int(*)(idSession*,const char*,
                    const idStaticList<idFile_SaveGame*,10>&,
                    const idSaveGameDetails&,bool,bool);
    return reinterpret_cast<Fn>(vtable[107])(
        this,folder,files,details,skipErrorDialog,waitForAlreadyInProgress);
  }

  idSaveGameManager* GetSaveGameManager() {
    using Fn = idSaveGameManager* (*)(idSession*);
    return reinterpret_cast<Fn>(vtable[146])(this);
  }

  int GetTitleStorageInt(const char* name,int fallback) {
    using Fn=int(*)(idSession*,const char*,int);
    return reinterpret_cast<Fn>(vtable[90])(this,name,fallback);
  }

  void SendUsercmds(idBitMsg& msg) {
    using Fn=void(*)(idSession*,idBitMsg&);
    reinterpret_cast<Fn>(vtable[141])(this,msg);
  }
};

extern idSession* session;

static_assert(sizeof(idSession) == 8, "session ABI drift");
static_assert(sizeof(idLobbyBase) == 8, "lobby ABI drift");
