#pragma once

#include "engine/renderer/declrenderprog.h"
#include "engine/renderer/declrenderprogattribs.h"

class idParser;

extern const char* programTargetNameString[PT_NUM_TARGETS];

renderProgData_t TranslateRenderProg(const renderProgParms_t* parms,
                                     idParser* source,
                                     unsigned int checksum,
                                     shaderConstructInfo_t* vertexInfo,
                                     shaderConstructInfo_t* fragmentInfo);
