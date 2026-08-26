#pragma once

#include "shared/idlib/math/vector.h"
#include "shared/idlib/text/str.h"

#include <cstddef>
#include <cstdint>

class idSoundShader;
class idTimelineEnv;

template <typename T> struct timelineListABI_t {
  T* list;
  int num;
  int size;
  std::int16_t granularity;
  std::uint8_t memTag;
  std::uint8_t listStatic;
};

template <> class idStrStatic<33> : public idStr {
public:
  idStrStatic() : idStr() {}
  ~idStrStatic();
};

class mgTimelineKeyAnimData {
public:
  mgTimelineKeyAnimData();
  ~mgTimelineKeyAnimData();

  timelineListABI_t<float> times;
  timelineListABI_t<unsigned char> data;
  int type;
  int keySize;
};

enum tlEventType_t {
  TL_EVENT_NONE = 0,
};

enum tlKeyName_t {
  TL_KEY_INVALID = -1,
};

struct tlEventData_t {
  tlEventData_t();

  float time;
  float duration;
  tlEventType_t type;
  unsigned char padding0[4];
  idStrStatic<33> anim;
  bool useAnimPos;
  bool useAnimScale;
  unsigned char padding1[6];
  const idSoundShader* sound;
  idVec3 worldPos;
  idAngles worldRot;
  tlKeyName_t keyName;
  unsigned char padding2[4];
  idStr keyValue;
  mgTimelineKeyAnimData keyAnimData;
  bool activateStart;
  bool activateEnd;
  unsigned char padding3[6];
  idTimelineEnv* env;
  idStr lightEntDefText;
};

static_assert(sizeof(mgTimelineKeyAnimData) == 0x38,
              "mgTimelineKeyAnimData ABI drift");
static_assert(sizeof(timelineListABI_t<float>) == 0x18,
              "timeline list ABI drift");
static_assert(sizeof(idStrStatic<33>) == 0x28,
              "idStrStatic<33> ABI drift");
static_assert(offsetof(tlEventData_t, anim) == 0x10,
              "tlEventData_t ABI drift");
static_assert(offsetof(tlEventData_t, sound) == 0x40,
              "tlEventData_t ABI drift");
static_assert(offsetof(tlEventData_t, keyValue) == 0x68,
              "tlEventData_t ABI drift");
static_assert(offsetof(tlEventData_t, keyAnimData) == 0x90,
              "tlEventData_t ABI drift");
static_assert(offsetof(tlEventData_t, activateStart) == 0xC8,
              "tlEventData_t ABI drift");
static_assert(offsetof(tlEventData_t, lightEntDefText) == 0xD8,
              "tlEventData_t ABI drift");
static_assert(sizeof(tlEventData_t) == 0x100, "tlEventData_t ABI drift");
