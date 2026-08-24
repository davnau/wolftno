#pragma once

#include "shared/idlib/containers/list.h"
#include "shared/idlib/text/cmdargs.h"

struct idAutoComplete {
  int matchLength;
  int currentIndex;
  idCmdArgs args;
  idList<idStr> suggestions;

  void Append(const idStr& suggestion, int completingArgument = -1);
};
