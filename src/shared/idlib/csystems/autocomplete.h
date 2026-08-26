#pragma once

#include "shared/idlib/containers/list.h"
#include "shared/idlib/text/cmdargs.h"

struct idAutoComplete {
  int matchLength;
  int currentIndex;
  idCmdArgs args;
  idList<idStr> suggestions;

  void Clear();
  void Append(const idStr& suggestion, int completingArgument = -1);
  int NumSuggestions() const { return suggestions.Num(); }
  const idStr& GetSuggestion(int index) const { return suggestions[index]; }
  int NumArgs() const { return args.argc; }
  const char* GetArg(int index) const { return args.Argv(index); }
  idCmdArgs& GetArgs() { return args; }
};
