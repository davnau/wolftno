#include "engine/models/skeletalanimation/md6util.h"

idMD6UtilAnimWebLookup g_md6UtilAnimWebLookup = nullptr;
idMD6UtilDeclLookup g_md6UtilDeclLookup = nullptr;
idMD6UtilDecodeFrame g_md6UtilDecodeFrame = nullptr;
int g_md6UtilGameTimePerSecond = 1000;

namespace {
void SplitPath(const char* path, idStr& webName, idStr& subWebName,
               idStr& stateName) {
  webName.Empty();
  subWebName.Empty();
  stateName.Empty();
  if (!path || !*path) return;

  const int length = idStr::Length(path);
  if (path[length - 1] == '/') {
    webName = path;
    return;
  }

  const int lastSlash = idStr::Last(path, '/', static_cast<unsigned>(-1));
  int previousSlash = -1;
  if (lastSlash > 0)
    previousSlash = idStr::Last(path, '/', lastSlash - 1);
  if (lastSlash == -1) {
    stateName = path;
    return;
  }

  const char* subWeb = path;
  int subWebLength = lastSlash;
  if (previousSlash != -1) {
    webName.Append(path, previousSlash);
    subWeb = path + previousSlash + 1;
    subWebLength = lastSlash - previousSlash - 1;
  }
  subWebName.Append(subWeb, subWebLength);
  stateName.Append(path + lastSlash + 1, length - lastSlash - 1);
}
} // namespace

void idAnimWebPath::GetWebName(idStr& webName) const {
  idStr subWebName;
  idStr stateName;
  SplitPath(data, webName, subWebName, stateName);
}

void idAnimWebPath::GetWebPathNames(idStr& webName, idStr& subWebName,
                                    idStr& stateName) const {
  SplitPath(data, webName, subWebName, stateName);
}

idJointIndex_t idMD6Skel::GetJointIndex(const char* jointName) const {
  for (int i = 0; i < jointNames.num; ++i) {
    if (idStr::Icmp(jointNames.list[i].str, jointName) == 0)
      return idJointIndex_t(static_cast<std::int16_t>(i));
  }
  return idJointIndex_t(static_cast<std::int16_t>(-1));
}

void SetMD6UtilAnimWebLookup(idMD6UtilAnimWebLookup value) {
  g_md6UtilAnimWebLookup = value;
}

void SetMD6UtilDeclLookup(idMD6UtilDeclLookup value) {
  g_md6UtilDeclLookup = value;
}

void SetMD6UtilDecodeFrame(idMD6UtilDecodeFrame value) {
  g_md6UtilDecodeFrame = value;
}

void SetMD6UtilGameTimePerSecond(int ticksPerSecond) {
  g_md6UtilGameTimePerSecond = ticksPerSecond;
}
