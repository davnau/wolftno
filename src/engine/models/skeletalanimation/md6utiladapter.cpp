#include "engine/models/skeletalanimation/md6util.h"

idMD6UtilAnimWebLookup g_md6UtilAnimWebLookup = nullptr;
idMD6UtilDeclLookup g_md6UtilDeclLookup = nullptr;
idMD6UtilDecodeFrame g_md6UtilDecodeFrame = nullptr;
int g_md6UtilGameTimePerSecond = 1000;

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
