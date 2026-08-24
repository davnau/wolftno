#pragma once

#include "shared/idlib/containers/list.h"

#include <string>

struct idFSMLogEntry {
  int time{0};
  int transitionCode{0};
  std::string state;
};

class idFSMLog {
public:
  explicit idFSMLog(int maxSize);
  ~idFSMLog();
  const idFSMLogEntry* FromLast(int index) const;
  idFSMLogEntry* Alloc();
  int Num() const { return list.Num(); }
  int MaxSize() const { return maxSize; }
private:
  idList<idFSMLogEntry, 5> list;
  int first{0};
  int maxSize{0};
};
