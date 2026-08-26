#pragma once

#include "engine/framework/usercmd.h"

#include <cstddef>
#include <cstring>

class idUserCmdMgr {
public:
  usercmd_t cmdBuffer[128][16];
  int writeFrame[16];
  int readFrame[16];

  usercmd_t& Command(int frame,int player) { return cmdBuffer[frame%128][player]; }
  const usercmd_t& Command(int frame,int player) const { return cmdBuffer[frame%128][player]; }
  void PutUserCmdForPlayer(int player,const usercmd_t& command) {
    Command(writeFrame[player],player)=command;
    if(writeFrame[player]-readFrame[player]+1>128) {
      readFrame[player]=writeFrame[player]-64;
    }
    ++writeFrame[player];
  }
};

static_assert(sizeof(idUserCmdMgr)==155776,"idUserCmdMgr ABI drift");
static_assert(offsetof(idUserCmdMgr,writeFrame)==155648,"writeFrame ABI drift");
static_assert(offsetof(idUserCmdMgr,readFrame)==155712,"readFrame ABI drift");
