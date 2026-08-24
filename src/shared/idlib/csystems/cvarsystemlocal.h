#pragma once

#include "shared/idlib/containers/hashindex.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/commandlink.h"
#include "shared/idlib/csystems/autocomplete.h"
#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/text/str.h"

enum cvarFlags_t {
  CVAR_ALL = 0,
  CVAR_BOOL = 1 << 0,
  CVAR_INTEGER = 1 << 1,
  CVAR_FLOAT = 1 << 2,
  CVAR_CHEAT = 1 << 3,
  CVAR_NOCHEAT = 1 << 4,
  CVAR_SYSTEM = 1 << 5,
  CVAR_RENDERER = 1 << 6,
  CVAR_SOUND = 1 << 7,
  CVAR_GUI = 1 << 8,
  CVAR_GAME = 1 << 9,
  CVAR_TOOL = 1 << 10,
  CVAR_USERINFO = 1 << 11,
  CVAR_SERVERINFO = 1 << 12,
  CVAR_NETWORKSYNC = 1 << 13,
  CVAR_INIT = 1 << 14,
  CVAR_ROM = 1 << 15,
  CVAR_ARCHIVE = 1 << 16,
  CVAR_MODIFIED = 1 << 17,
  CVAR_MODIFIED2 = 1 << 18
};

class idCVar {
public:
  struct cvarCallback_t {
    void (*callback)(idCVar&);
    cvarCallback_t* next;
  };
  idCVar();
  idCVar(const char* name, const char* value, int flags, const char* description,
         float minimum = 0.0f, float maximum = 0.0f, const char** values = nullptr,
         argCompletion_t completion = nullptr);
  void Init(const char* name, const char* value, int flags, const char* description,
            float minimum = 0.0f, float maximum = 0.0f, const char** values = nullptr,
            argCompletion_t completion = nullptr);
  void UpdateValue();
  void Reset();
  void Set(const char* value, bool force = false);
  void SetString(const char*);
  void SetBool(bool);
  void SetInteger(int);
  void SetFloat(float);
  const char* GetString() const { return valueString.data; }
  bool GetBool() const { return valueInteger != 0; }
  int GetInteger() const { return valueInteger; }
  float GetFloat() const { return valueFloat; }
  static void RegisterStaticVars();
  static idCVar* StaticVars();
  idStr valueString;
  int valueInteger{};
  float valueFloat{};
  const char* name{};
  const char* resetString{};
  const char* description{};
  int flags{};
  float valueMin{}, valueMax{};
  const char** valueStrings{};
  argCompletion_t valueCompletion{};
  cvarCallback_t* onChange{};
  idCVar* next{};
};

class idCVarSystem {
public:
  virtual ~idCVarSystem() = default;
  virtual int NumCVars() const = 0;
  virtual const idCVar* FindByIndex(int) const = 0;
  virtual void ResetCVarByIndex(int) = 0;
  virtual idCVar* Find(const char*) const = 0;
  virtual bool CvarExists(const char*, bool = false) const = 0;
  virtual void FindCvarsByPrefix(const char*, idList<idCVar*>&) const = 0;
  virtual void SetCVarString(const char*, const char*, int = 0) = 0;
  virtual void SetCVarBool(const char*, bool, int = 0) = 0;
  virtual void SetCVarInteger(const char*, int, int = 0) = 0;
  virtual void SetCVarFloat(const char*, float, int = 0) = 0;
  virtual const char* GetCVarString(const char* name, const char* fallback = "") const = 0;
  virtual bool GetCVarBool(const char*, bool = false) const = 0;
  virtual int GetCVarInteger(const char*, int = 0) const = 0;
  virtual float GetCVarFloat(const char*, float = 0) const = 0;
  virtual bool Command(const idCmdArgs&) = 0;
  virtual void CommandCompletion(idAutoComplete*) const = 0;
  virtual void SetModifiedFlags(int) = 0;
  virtual int GetModifiedFlags() const = 0;
  virtual void ClearModifiedFlags(int) = 0;
  virtual void ResetFlaggedVariables(int) = 0;
  virtual void WriteFlaggedVariables(int, idFile*) const = 0;
  virtual void ReportModifiedCVars() const = 0;
  virtual void ClearModifiedCVars() = 0;
  virtual void RegisterStaticVars() = 0;
  virtual void ListCVars(const idCmdArgs&) const = 0;
};

class idCVarSystemLocal final : public idCVarSystem {
public:
  idCVarSystemLocal();
  ~idCVarSystemLocal() override = default;
  int NumCVars() const override { return cvars.Num(); }
  const idCVar* FindByIndex(int) const override;
  void ResetCVarByIndex(int) override;
  idCVar* Find(const char*) const override;
  bool CvarExists(const char*, bool = false) const override;
  void FindCvarsByPrefix(const char*, idList<idCVar*>&) const override;
  void SetCVarString(const char*, const char*, int = 0) override;
  void SetCVarBool(const char*, bool, int = 0) override;
  void SetCVarInteger(const char*, int, int = 0) override;
  void SetCVarFloat(const char*, float, int = 0) override;
  const char* GetCVarString(const char* name, const char* fallback = "") const override;
  bool GetCVarBool(const char*, bool = false) const override;
  int GetCVarInteger(const char*, int = 0) const override;
  float GetCVarFloat(const char*, float = 0) const override;
  bool Command(const idCmdArgs&) override;
  void CommandCompletion(idAutoComplete*) const override;
  void SetModifiedFlags(int flags) override { modifiedFlags |= flags; }
  int GetModifiedFlags() const override { return modifiedFlags; }
  void ClearModifiedFlags(int flags) override { modifiedFlags &= ~flags; }
  void ResetFlaggedVariables(int) override;
  void WriteFlaggedVariables(int, idFile*) const override;
  void ReportModifiedCVars() const override;
  void ClearModifiedCVars() override;
  void RegisterStaticVars() override;
  void ListCVars(const idCmdArgs&) const override;
  void FindCvarsByPrefix(const char*, idList<idCVar*>&);
  idList<idCVar*> cvars;
  idHashIndex cvarHash;
  int modifiedFlags{};

private:
  void Add(idCVar*);
};

extern idCVarSystemLocal cvarSystemLocal;
extern idCVarSystem* cvarSystem;
