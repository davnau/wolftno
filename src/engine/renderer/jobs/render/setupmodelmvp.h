#pragma once

#include "engine/renderer/render.h"

void SetupModelMVPMatrix(idRenderMatrix* mvpMatrix,
                         const idRenderModelParms* parameters,
                         const idRenderMatrix* modelMatrix,
                         const idRenderView* renderView, float zNear,
                         float zFar, float guiOriginOffset);

