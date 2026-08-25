#include "engine/models/renderworld.h"

#include "engine/models/rendermodeldecal.h"
#include "engine/models/renderworld_load.h"
#include "engine/models/static/staticmodel.h"
#include "engine/models/transparency/rendermodeltransparency.h"
#include "engine/renderer/jobs/render/renderwalkbsp.h"
#include "engine/renderer/renderworldcommitted.h"
#include "shared/idlib/geometry/polytope.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/text/lexer.h"

#include <algorithm>
#include <cstring>
#include <memory>

template bool idList<idStaticModelSurface, 84>::Resize(int);

namespace {

idRenderWorldRuntimeCallbacks loadCallbacks;

void AddBounds(idBounds& destination, const idBounds& source) {
  destination.b[0].x = std::min(destination.b[0].x, source.b[0].x);
  destination.b[0].y = std::min(destination.b[0].y, source.b[0].y);
  destination.b[0].z = std::min(destination.b[0].z, source.b[0].z);
  destination.b[1].x = std::max(destination.b[1].x, source.b[1].x);
  destination.b[1].y = std::max(destination.b[1].y, source.b[1].y);
  destination.b[1].z = std::max(destination.b[1].z, source.b[1].z);
}

void FreeAreaData(idRenderWorldCommitted* world) {
  delete[] world->worldAreas;
  delete[] world->worldAreaSubspaceBounds;
  delete[] world->worldAreaGeometryBounds;
  world->worldAreas = nullptr;
  world->worldAreaSubspaceBounds = nullptr;
  world->worldAreaGeometryBounds = nullptr;
  world->numWorldAreas = 0;
}

void FreeNodeData(idRenderWorldCommitted* world) {
  delete[] static_cast<areaNode_t*>(world->areaNodes);
  delete[] world->areaNodeBounds;
  delete[] world->areaNodeOcclusionIndex;
  world->areaNodes = nullptr;
  world->areaNodeBounds = nullptr;
  world->areaNodeOcclusionIndex = nullptr;
  world->numAreaNodes = 0;
}

} // namespace

void SetRenderWorldLoadRuntimeCallbacks(
    const idRenderWorldRuntimeCallbacks& callbacks) {
  loadCallbacks = callbacks;
}

worldArea_t::worldArea_t() {
  modelRefs.ClearFree();
  drawLightRefs.ClearFree();
  approximatedLightRefs.ClearFree();
  approximatedLightPrivateLightRefs.ClearFree();
  worldSurfaces.ClearFree();
}

worldArea_t::~worldArea_t() = default;

void idVirtualTexture::SetMapName(const char* mapName_) {
  idStrStatic<128> temporary;
  temporary = mapName_;
  mapName = temporary;
}

void idRenderWorldLocal::AssignNodeOcclusionIndices_r(
    int nodeNum, int depth, int& numOcclusionIndices) {
  auto* nodes = static_cast<areaNode_t*>(committedWorld->areaNodes);
  if (depth == 6) {
    if (numOcclusionIndices < 64)
      committedWorld->areaNodeOcclusionIndex[nodeNum] =
          numOcclusionIndices++;
    return;
  }

  const int firstChild = nodes[nodeNum].children[0];
  if (firstChild > 0)
    AssignNodeOcclusionIndices_r(firstChild, depth + 1,
                                 numOcclusionIndices);
  const int secondChild = nodes[nodeNum].children[1];
  if (secondChild > 0)
    AssignNodeOcclusionIndices_r(secondChild, depth + 1,
                                 numOcclusionIndices);
}

idBounds idRenderWorldLocal::BuildNodeBounds_r(int nodeNum, idPlane* planes,
                                               int numPlanes) {
  if (nodeNum < 0) {
    const int areaIndex = -1 - nodeNum;
    worldArea_t& area = committedWorld->worldAreas[areaIndex];
    idBounds subspaceBounds =
        idPolytope::ConvexPolytopeBounds(planes, numPlanes);
    idBounds geometryBounds;
    geometryBounds.Clear();
    for (int index = 0; index < area.worldSurfaces.Num(); ++index) {
      const idRenderModelSurface& surface = area.worldSurfaces[index];
      if (surface.geometry != nullptr)
        AddBounds(geometryBounds, surface.geometry->bounds);
    }
    if (geometryBounds.b[0].x > geometryBounds.b[1].x) {
      const idVec3 center = (subspaceBounds.b[0] + subspaceBounds.b[1]) * 0.5f;
      geometryBounds.b[0] = center;
      geometryBounds.b[1] = center;
    }
    committedWorld->worldAreaSubspaceBounds[areaIndex] = subspaceBounds;
    AddBounds(committedWorld->worldAreaSubspaceBounds[areaIndex],
              geometryBounds);
    committedWorld->worldAreaGeometryBounds[areaIndex] = geometryBounds;
    return committedWorld->worldAreaSubspaceBounds[areaIndex];
  }

  auto* nodes = static_cast<areaNode_t*>(committedWorld->areaNodes);
  const areaNode_t& node = nodes[nodeNum];
  idBounds bounds;
  if (node.children[0] != 0 && node.children[1] != 0) {
    planes[numPlanes] = {-node.plane.a, -node.plane.b, -node.plane.c,
                         -node.plane.d};
    bounds = BuildNodeBounds_r(node.children[0], planes, numPlanes + 1);
    planes[numPlanes] = node.plane;
    AddBounds(bounds,
              BuildNodeBounds_r(node.children[1], planes, numPlanes + 1));
  } else if (node.children[0] != 0) {
    planes[numPlanes] = {-node.plane.a, -node.plane.b, -node.plane.c,
                         -node.plane.d};
    bounds = BuildNodeBounds_r(node.children[0], planes, numPlanes + 1);
  } else {
    planes[numPlanes] = node.plane;
    bounds = BuildNodeBounds_r(node.children[1], planes, numPlanes + 1);
  }
  committedWorld->areaNodeBounds[nodeNum] = bounds;
  return bounds;
}

void idRenderWorldLocal::ParseNodes(idLexer* source) {
  source->ExpectTokenString("{");
  int nodeCount = source->ParseInt();
  if (nodeCount < 0) {
    source->Error("R_ParseNodes: bad numAreaNodes");
    return;
  }
  if (committedWorld->isComboMap && nodeCount > 8192) {
    source->Error("R_ParseNodes: numAreaNodes exceeds %d", 8192);
    return;
  }
  if (nodeCount == 0)
    nodeCount = 1;
  FreeNodeData(committedWorld);
  committedWorld->numAreaNodes = nodeCount;
  committedWorld->areaNodes = new areaNode_t[nodeCount]{};
  committedWorld->areaNodeBounds = new idBounds[nodeCount]{};
  committedWorld->areaNodeOcclusionIndex = new int[nodeCount];

  auto* nodes = static_cast<areaNode_t*>(committedWorld->areaNodes);
  if (source->PeekTokenString("}")) {
    nodes[0].plane = {0.0f, 0.0f, 1.0f, 131072.0f};
    nodes[0].children[0] = -1;
    nodes[0].children[1] = 0;
    committedWorld->areaNodeOcclusionIndex[0] = -1;
  } else {
    for (int index = 0; index < nodeCount; ++index) {
      source->Parse1DMatrix(4, &nodes[index].plane.a);
      nodes[index].children[0] = source->ParseInt();
      nodes[index].children[1] = source->ParseInt();
      committedWorld->areaNodeOcclusionIndex[index] = -1;
    }
  }
  source->ExpectTokenString("}");
}

void idRenderWorldLocal::ParseInterAreaPortals(idLexer* source) {
  source->ExpectTokenString("{");
  const int areaCount = source->ParseInt();
  if (areaCount < 0 || areaCount > 8192) {
    source->Error("R_ParseInterAreaPortals: bad numPortalAreas");
    return;
  }
  FreeAreaData(committedWorld);
  committedWorld->numWorldAreas = areaCount;
  committedWorld->worldAreas = new worldArea_t[areaCount];
  committedWorld->worldAreaSubspaceBounds = new idBounds[areaCount];
  committedWorld->worldAreaGeometryBounds = new idBounds[areaCount];
  if (source->ParseInt() < 0) {
    source->Error("R_ParseInterAreaPortals: bad numInterAreaPortals");
    return;
  }
  source->ExpectTokenString("}");
}

void idRenderWorldLocal::ParseAreas(idLexer* source) {
  committedWorld->hasWorldSurfaceOffsets = true;
  source->ExpectTokenString("{");
  const int areaCount = source->ParseInt();
  if (areaCount < 0 || areaCount > 8192) {
    source->Error("R_ParseAreas: bad numAreas");
    return;
  }
  FreeAreaData(committedWorld);
  committedWorld->numWorldAreas = areaCount;
  committedWorld->worldAreas = new worldArea_t[areaCount];
  committedWorld->worldAreaSubspaceBounds = new idBounds[areaCount];
  committedWorld->worldAreaGeometryBounds = new idBounds[areaCount];
  for (int index = 0; index < areaCount; ++index) {
    worldArea_t& area = committedWorld->worldAreas[index];
    area.firstSurfaceInWorldModel = source->ParseInt();
    area.worldSurfaces.SetNum(source->ParseInt());
  }
  source->ExpectTokenString("}");
}

void idRenderWorldLocal::ClearWorld() {
  FreeAreaData(committedWorld);
  FreeNodeData(committedWorld);
  committedWorld->numWorldAreas = 1;
  committedWorld->worldAreas = new worldArea_t[1];
  committedWorld->worldAreas[0].areaNum = 0;
  committedWorld->worldAreas[0].firstSurfaceInWorldModel = 0;
  committedWorld->worldAreas[0].binaryModelId = -1;
  committedWorld->worldAreaSubspaceBounds = new idBounds[1];
  committedWorld->worldAreaGeometryBounds = new idBounds[1];
  constexpr float worldLimit = 131072.0f;
  const idBounds maximumWorldBounds{{{-worldLimit, -worldLimit, -worldLimit},
                                      {worldLimit, worldLimit, worldLimit}}};
  committedWorld->worldAreaSubspaceBounds[0] = maximumWorldBounds;
  committedWorld->worldAreaGeometryBounds[0] = maximumWorldBounds;

  committedWorld->numAreaNodes = 1;
  committedWorld->areaNodes = new areaNode_t[1]{};
  committedWorld->areaNodeBounds = new idBounds[1]{};
  committedWorld->areaNodeOcclusionIndex = new int[1];
  auto* node = static_cast<areaNode_t*>(committedWorld->areaNodes);
  node->plane = {0.0f, 0.0f, 1.0f, worldLimit};
  node->children[0] = -1;
  node->children[1] = 0;
  committedWorld->areaNodeOcclusionIndex[0] = -1;
  committedWorld->binaryModelNames.Clear();
  committedWorld->binaryModelState.Clear();
  committedWorld->binaryModelHash.Clear();
  skyModel = nullptr;
}

void idRenderWorldLocal::ParseEnvAreas(idLexer* source) {
  source->ExpectTokenString("{");
  committedWorld->dynamicEnvironments.ClearFree();
  const int count = source->ParseInt();
  idToken token;
  for (int index = 0; index < count; ++index) {
    source->ReadToken(token);
    const idDeclEnv* environment =
        loadCallbacks.findEnvironmentDecl != nullptr
            ? loadCallbacks.findEnvironmentDecl(token.data)
            : nullptr;
    idBounds bounds;
    source->Parse1DMatrix(6, &bounds.b[0].x);
    const float volume = source->ParseFloat();
    const int planeCount = source->ParseInt();
    idPlane planes[16]{};
    for (int plane = 0; plane < planeCount && plane < 16; ++plane)
      source->Parse1DMatrix(4, &planes[plane].a);
    AddEnvironment(bounds, environment, planes, planeCount, volume);
  }
  source->ExpectTokenString("}");
}

void idRenderWorldLocal::ParseEnvArea(const char* worldName, idLexer* source,
                                      int version) {
  source->ExpectTokenString("{");
  idToken token;
  idStr envDeclPath;
  idStr userEnvMap;
  source->ExpectTokenString("envDecl");
  source->ReadToken(token);
  envDeclPath = token;
  source->ExpectTokenString("userEnvMap");
  source->ParseRestOfLine(token);
  userEnvMap = token;
  source->ExpectTokenString("volume");
  const float volume = source->ParseFloat();
  source->ExpectTokenString("bounds");
  idBounds bounds;
  source->Parse1DMatrix(6, &bounds.b[0].x);
  source->ExpectTokenString("numPlanes");
  const int planeCount = source->ParseInt();
  source->ExpectTokenString("planes");
  source->ExpectTokenString("{");
  idStaticList<idPlane, 16> planes;
  planes.SetNum(planeCount);
  for (int index = 0; index < planeCount; ++index)
    source->Parse1DMatrix(4, &planes[index].a);
  source->ExpectTokenString("}");

  idVec3 specularOrigin{};
  idColor specularTint{0.0f, 0.0f, 0.0f, 1.0f};
  idVec2 specularPowers{2.0f, 64.0f};
  idVec2 specularScales{0.5f, 2.0f};
  if (version > 1) {
    source->ExpectTokenString("specularOrigin");
    source->Parse1DMatrix(3, &specularOrigin.x);
  }
  if (version > 2) {
    source->ExpectTokenString("specularTint");
    source->Parse1DMatrix(4, &specularTint.r);
    source->ExpectTokenString("specularPowers");
    source->Parse1DMatrix(2, &specularPowers.x);
    source->ExpectTokenString("specularScales");
    source->Parse1DMatrix(2, &specularScales.x);
  }
  source->ExpectTokenString("}");

  const idDeclEnv* environment =
      loadCallbacks.findEnvironmentDecl != nullptr
          ? loadCallbacks.findEnvironmentDecl(envDeclPath.data)
          : nullptr;
  const bool customEnvironment = !userEnvMap.IsEmpty() ||
      specularOrigin.x != 0.0f || specularOrigin.y != 0.0f ||
      specularOrigin.z != 0.0f;
  if (customEnvironment && loadCallbacks.buildEnvironmentDecl != nullptr) {
    idStr areaName;
    areaName.Format("area%d", committedWorld->dynamicEnvironments.Num());
    environment = loadCallbacks.buildEnvironmentDecl(
        areaName.data, envDeclPath.data, worldName, userEnvMap.data,
        specularOrigin, specularTint, specularPowers, specularScales);
  }
  AddEnvironment(bounds, environment, planes.list, planes.Num(), volume);
}

bool idRenderWorldLocal::LoadEnvAreas(const char* worldName,
                                      const char* fileName) {
  auto source = std::make_unique<idLexer>(592);
  if (!source->LoadFile(fileName, false)) {
    idLib::Printf("idRenderWorldLocal::InitFromMap: %s not found\n", fileName);
    return false;
  }
  idToken token;
  if (!source->ReadToken(token))
    idLib::Error("idRenderWorldLocal::LoadEnvAreas: couldn't read envareas file id");
  int version = 0;
  if (token.Icmp("mapAreasFile001") == 0)
    version = 1;
  else if (token.Icmp("mapAreasFile002") == 0)
    version = 2;
  else if (token.Icmp("mapAreasFile003") == 0)
    version = 3;
  else
    idLib::Error(
        "idRenderWorldLocal::LoadEnvAreas: bad id '%s' instead of ( '%s', '%s' or '%s' )",
        token.data, "mapAreasFile003", "mapAreasFile002", "mapAreasFile001");

  unsigned expectedCount = 0;
  while (source->ReadToken(token)) {
    if (token == "numEnvAreas")
      expectedCount = static_cast<unsigned>(source->ParseInt());
    else if (token == "envArea")
      ParseEnvArea(worldName, source.get(), version);
  }
  if (expectedCount !=
      static_cast<unsigned>(committedWorld->dynamicEnvironments.Num())) {
    idLib::Error(
        "idRenderWorldLocal::LoadEnvAreas: failed to parse correct number of envAreas %d vs %d",
        committedWorld->dynamicEnvironments.Num(), expectedCount);
  }
  return true;
}
void idRenderWorldLocal::SpecificationComplete(bool preloadPageFileCache) {
  idLib::Printf("---------- idRenderWorldLocal::SpecificationComplete ----------\n");
  constexpr float worldLimit = 131072.0f;
  const idBounds maximumWorldBounds{{{-worldLimit, -worldLimit, -worldLimit},
                                      {worldLimit, worldLimit, worldLimit}}};
  if (committedWorld->numAreaNodes <= 0) {
    if (committedWorld->areaNodeBounds != nullptr)
      committedWorld->areaNodeBounds[0] = maximumWorldBounds;
    if (committedWorld->numWorldAreas > 0) {
      committedWorld->worldAreaSubspaceBounds[0] = maximumWorldBounds;
      committedWorld->worldAreaGeometryBounds[0] = maximumWorldBounds;
    }
  } else {
    idBounds worldBounds;
    worldBounds.Clear();
    for (int area = 0; area < committedWorld->numWorldAreas; ++area) {
      for (int surface = 0;
           surface < committedWorld->worldAreas[area].worldSurfaces.Num();
           ++surface) {
        const idTriangles* geometry =
            committedWorld->worldAreas[area].worldSurfaces[surface].geometry;
        if (geometry != nullptr)
          AddBounds(worldBounds, geometry->bounds);
      }
    }
    if (skyModel != nullptr)
      AddBounds(worldBounds, skyModel->referenceBounds);
    if (worldBounds.b[0].x > worldBounds.b[1].x)
      worldBounds = maximumWorldBounds;
    idPlane planes[6]{};
    idPolytope::ConvexPolytopeFromBounds(worldBounds, planes);
    BuildNodeBounds_r(0, planes, 6);
    int numOcclusionIndices = 0;
    auto* nodes = static_cast<areaNode_t*>(committedWorld->areaNodes);
    if (nodes[0].children[0] > 0)
      AssignNodeOcclusionIndices_r(nodes[0].children[0], 1,
                                   numOcclusionIndices);
    if (nodes[0].children[1] > 0)
      AssignNodeOcclusionIndices_r(nodes[0].children[1], 1,
                                   numOcclusionIndices);
  }

  if (committedWorld->dynamicEnvironments.Num() == 0) {
    const idDeclEnv* defaultEnvironment =
        loadCallbacks.findEnvironmentDecl != nullptr
            ? loadCallbacks.findEnvironmentDecl("default")
            : nullptr;
    if (defaultEnvironment == nullptr)
      idLib::Error("Invalid or missing environment decl 'default' !");
    AddEnvironment(maximumWorldBounds, defaultEnvironment, nullptr, 0, 0.0f);
  }

  int modelCount = renderModels.Num();
  while (modelCount > 0 && renderModels[modelCount - 1] == nullptr)
    --modelCount;
  renderModels.SetNum(modelCount);
  int lightCount = renderLights.Num();
  while (lightCount > 0 && renderLights[lightCount - 1] == nullptr)
    --lightCount;
  renderLights.SetNum(lightCount);

  if (loadCallbacks.specificationComplete != nullptr)
    loadCallbacks.specificationComplete(this, preloadPageFileCache);
  specificationCompleteHasBeenCalled = true;
}

bool idRenderWorldLocal::ParseProc(const char* fileName) {
  committedWorld->hasWorldSurfaceOffsets = false;
  auto source = std::make_unique<idLexer>(592);
  if (!source->LoadFile(fileName, false)) {
    idLib::Printf("idRenderWorldLocal::InitFromMap: %s not found\n", fileName);
    ClearWorld();
    return false;
  }
  idToken token;
  if (!source->ReadToken(token)) {
    idLib::Warning("idRenderWorldLocal::InitFromMap: couldn't read proc file id");
    return true;
  }
  if (token.Icmp("mapProcFile005") != 0 &&
      token.Icmp("mapProcFile006") != 0 &&
      token.Icmp("mapProcFile007") != 0) {
    idLib::Warning("idRenderWorldLocal::InitFromMap: bad id '%s' instead of '%s'",
                   token.data, "mapProcFile007");
    return true;
  }

  while (source->ReadToken(token)) {
    if (token == "interAreaPortals") {
      ParseInterAreaPortals(source.get());
    } else if (token == "areas") {
      ParseAreas(source.get());
    } else if (token == "nodes") {
      ParseNodes(source.get());
    } else if (token == "binaryModels") {
      source->ExpectTokenString("{");
      const int count = source->ParseInt();
      for (int index = 0; index < count; ++index) {
        source->ReadToken(token);
        const bool enabled = source->ParseBool();
        if (loadCallbacks.setBinaryModelState != nullptr)
          loadCallbacks.setBinaryModelState(committedWorld, token.data,
                                            enabled, true);
      }
      source->ExpectTokenString("}");
    } else if (token == "envAreas") {
      ParseEnvAreas(source.get());
    } else {
      source->Error("idRenderWorldLocal::InitFromMap: bad token \"%s\"",
                    token.data);
    }
  }
  return true;
}
bool idRenderWorldLocal::InitFromMap(const char* name, bool comboMap,
                                     bool toolMap) {
  if (everInitialized)
    idLib::FatalError("idRenderWorldLocal::InitFromMap: everInitialized was set");
  everInitialized = true;

  transparencyRenderModel = new idRenderModelTransparency;
  AddRenderModel(transparencyRenderModel);
  transparencyRenderModel->CommitThisFrame();
  decalRenderModel = new idRenderModelDecal(true);
  AddRenderModel(decalRenderModel);
  decalRenderModel->CommitThisFrame();
  virtualTextures.ClearFree();
  megaTextures.ClearFree();

  if (name == nullptr || *name == '\0') {
    mapName = "<FREED>";
    committedWorld->mapName = mapName.data;
    ClearWorld();
    return true;
  }

  idStr baseName(name);
  baseName.StripPath();
  idStr fileName;
  fileName.Format(comboMap ? "%s/_combo/%s" : "%s/%s", name,
                  baseName.data);
  fileName.SetFileExtension("proc");
  if (!ParseProc(fileName.data))
    return false;

  fileName.Format(comboMap ? "%s/_combo/_world.envareas"
                           : "%s/_world.envareas",
                  name);
  LoadEnvAreas(name, fileName.data);
  mapName = name;
  mapName.StripFileExtension();
  committedWorld->mapName = mapName.data;
  committedWorld->isComboMap = comboMap;
  committedWorld->isToolsWorld = toolMap;
  committedWorld->hasWorldGeometry = true;
  committedWorld->transparencyRenderModel = transparencyRenderModel->committed;
  if (committedWorld->numWorldAreas == 0)
    ClearWorld();

  return loadCallbacks.finishInitFromMap == nullptr ||
         loadCallbacks.finishInitFromMap(this);
}
