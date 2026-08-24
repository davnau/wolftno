#pragma once

#include "engine/models/cuttable/rendermodelcuttableresource.h"
#include "engine/models/rendermodel.h"
#include "shared/idlib/filesystem/file.h"

class idRenderModelCuttable final : public idRenderModel {
public:
  explicit idRenderModelCuttable(const char* modelName);
  ~idRenderModelCuttable() override;

  const idPlane* GetReferencePlane();
  const idList<idVec3>* GetContourBounds();
  bool LoadModel(const char* modelName);
  void Save(idFile* file) override;
  bool Load(idFile* file) override;
  int UpdateSurfaces(int bufferIndex_, const idVertexBuffer* vertexBuffer,
                     const idVertexBuffer* stBuffer, const idIndexBuffer* indexBuffer,
                     const idCuttableGenGeometry* genGeometry);

  idRenderModelCuttableResource* modelResource = nullptr;
  int bufferIndex = 0;
};

