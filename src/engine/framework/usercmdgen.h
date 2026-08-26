#pragma once

class idFile;
class usercmd_t;
struct inputSettings_t;
enum cmdGenButton_t : int;

class idUsercmdGen {
public:
  virtual ~idUsercmdGen() = default;
  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual void Clear() = 0;
  virtual void ClearAngles() = 0;
  virtual void SaveGame(idFile*) = 0;
  virtual bool LoadGame(idFile*) = 0;
  virtual cmdGenButton_t BindStringUsercmdAction(const char*) = 0;
  virtual bool HasCompoundBinding(const char*) = 0;
  virtual usercmd_t GetUsercmd(int,const inputSettings_t&,bool,int) = 0;
  virtual void GetRecordedUserCmd(int,usercmd_t&) = 0;
  virtual bool RecordPlay(int,bool,const char*) = 0;
  virtual bool GetUseAimAssist() = 0;
  virtual bool GetMouseDetected() = 0;
  virtual bool GetJoystickDetected() = 0;
};

extern idUsercmdGen* usercmdGen;

static_assert(sizeof(idUsercmdGen)==8,"idUsercmdGen ABI drift");
