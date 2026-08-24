#include "engine/models/skeletalanimation/md6util.h"

#include <string>

namespace {
idMD6UtilAnimWebLookup animWebLookup = nullptr;
idMD6UtilDeclLookup declLookup = nullptr;
idMD6UtilDecodeFrame decodeFrame = nullptr;

struct WebPathNames { std::string web, subWeb, state; };
WebPathNames SplitWebPath(const idAnimWebPath* path) {
  WebPathNames result;
  const std::string value = path && path->data ? path->data : "";
  const std::size_t first = value.find('/');
  if (first == std::string::npos) { result.web = value; return result; }
  result.web = value.substr(0, first);
  const std::size_t second = value.find('/', first + 1);
  if (second == std::string::npos) { result.state = value.substr(first + 1); return result; }
  result.subWeb = value.substr(first + 1, second - first - 1);
  result.state = value.substr(second + 1);
  return result;
}
idMat3 JointAxis(const idJointMat& joint) {
  idMat3 result;
  result.mat[0] = {joint.mat[0], joint.mat[4], joint.mat[8]};
  result.mat[1] = {joint.mat[1], joint.mat[5], joint.mat[9]};
  result.mat[2] = {joint.mat[2], joint.mat[6], joint.mat[10]};
  return result;
}
idVec3 JointTranslation(const idJointMat& joint) {
  return {joint.mat[3], joint.mat[7], joint.mat[11]};
}
} // namespace

int idMD6Skel::GetJointIndex(const char* jointName) const {
  for (int i = 0; i < jointNames.num; ++i)
    if (idStr::Icmp(jointNames.list[i].str, jointName) == 0) return i;
  return -1;
}
void SetMD6UtilAnimWebLookup(idMD6UtilAnimWebLookup value) { animWebLookup = value; }
void SetMD6UtilDeclLookup(idMD6UtilDeclLookup value) { declLookup = value; }
void SetMD6UtilDecodeFrame(idMD6UtilDecodeFrame value) { decodeFrame = value; }

const idMD6Anim* idMD6Util::FindMD6Anim(
    const idDeclAnimWeb* webRef, idAnimWebPath* webPath,
    const idDeclMD6* declMD6, animWebModelIndex_t modelIndex, bool loadAnim) {
  const WebPathNames names = SplitWebPath(webPath);
  if (!webRef && animWebLookup) webRef = animWebLookup(names.web.c_str());
  if (!webRef) return nullptr;
  if (!declMD6 && webRef->models.num > 0) declMD6 = webRef->models.list[0];
  const animWebNodeIndex_t nodeIndex = webRef->GetNodeIndexForName(
      names.subWeb.c_str(), names.state.c_str());
  if (nodeIndex.value < 0 || nodeIndex.value >= webRef->nodes.num ||
      modelIndex.value < 0) return nullptr;
  const idDeclAnimWebNode* node = webRef->nodes.list[nodeIndex.value];
  if (!node || modelIndex.value >= node->modelInfo.num) return nullptr;
  const idDeclAnimWebNode::idModelInfo& info = node->modelInfo.list[modelIndex.value];
  if (info.animProps.num <= 0 || !info.animProps.list) return nullptr;
  return loadAnim ? info.animProps.list[0].SelectAndLoadAnim(declMD6, 0)
                  : info.animProps.list[0].SelectAnim(declMD6, 0);
}

bool idMD6Util::idJointCache::GetDelta(animDelta_t* delta,
                                       const char* jointName,
                                       const char* refJointName) {
  if (!validJointCache) return false;
  const idDeclMD6* declaration = md6Decl;
  if (webRef) {
    if (webRef->models.num <= 0) return false;
    declaration = webRef->models.list[0];
  }
  if (!declaration || !declaration->model || !declaration->model->skeleton)
    return false;
  const idMD6Skel& skeleton = *declaration->model->skeleton;
  const int joint = skeleton.GetJointIndex(jointName);
  const int reference = skeleton.GetJointIndex(refJointName);
  if (joint < 0 || reference < 0 || joint >= 256 || reference >= 256) return false;
  const idMat3 jointAxis = JointAxis(jointMatrices[joint]);
  if (joint == reference) {
    delta->deltaTranslation = JointTranslation(jointMatrices[joint]);
    delta->deltaAxis = jointAxis;
  } else {
    delta->deltaTranslation = JointTranslation(jointMatrices[joint]) -
                              JointTranslation(jointMatrices[reference]);
    const idAngles refAngles = JointAxis(jointMatrices[reference]).ToAngles();
    const idAngles angles = jointAxis.ToAngles();
    delta->deltaAxis = idAngles{angles.pitch - refAngles.pitch,
                                angles.yaw - refAngles.yaw,
                                angles.roll - refAngles.roll}.ToMat3();
  }
  return true;
}

void idMD6Util::idJointCache::UpdateCache() {
  validJointCache = false;
  const idDeclMD6* declaration = md6Decl;
  if (webRef) {
    if (webRef->models.num <= 0) return;
    declaration = webRef->models.list[0];
  }
  if (!declaration || !declaration->model || !declaration->model->skeleton ||
      !md6anim || !md6anim->animData || !decodeFrame) return;
  constexpr float gameTimePerSecond = 1000.0f;
  const float time = gameTimePerSecond * static_cast<float>(frameNum) /
                     static_cast<float>(md6anim->animData->frameRate);
  decodeFrame(declaration->model, md6anim, time, gameTimePerSecond,
              jointMatrices, 256);
  validJointCache = true;
}

void idMD6Util::idJointCache::Set(const idDeclAnimWeb* newWebRef,
                                  idAnimWebPath* webPath, int requestedFrame) {
  validJointCache = false;
  webRef = newWebRef;
  const idDeclMD6* declaration = nullptr;
  if (newWebRef) {
    if (newWebRef->editorModels.num > 0 && declLookup)
      declaration = declLookup(newWebRef->editorModels.list[0].str);
    if (!declaration && newWebRef->models.num > 0)
      declaration = newWebRef->models.list[0];
  }
  md6Decl = declaration;
  md6anim = idMD6Util::FindMD6Anim(newWebRef, webPath, declaration,
                                   animWebModelIndex_t(0), false);
  if (!md6anim || !md6anim->animData) return;
  frameNum = requestedFrame == -1 ? md6anim->animData->numFrames - 1
                                  : requestedFrame;
  validJointCache = false;
  UpdateCache();
}

idMD6Util::idJointCache::idJointCache(idAnimWebPath* webPath, int frame)
    : webRef(nullptr), md6Decl(nullptr), md6anim(nullptr), frameNum(0),
      validJointCache(false) {
  const WebPathNames names = SplitWebPath(webPath);
  const idDeclAnimWeb* declaration = animWebLookup ? animWebLookup(names.web.c_str()) : nullptr;
  webRef = declaration;
  Set(declaration, webPath, frame);
}

