#pragma once

#include "shared/idlib/text/str.h"

// NEEDS WORK: engine/framework owns these tables and implementations.
enum keyNum_t : int {
  K_NONE = 0x00, K_ESCAPE = 0x01, K_BACKSPACE = 0x0E, K_ENTER = 0x1C,
  K_LSHIFT = 0x2A, K_RSHIFT = 0x36, K_HOME = 0xC7, K_LEFTARROW = 0xCB,
  K_RIGHTARROW = 0xCD, K_END = 0xCF, K_DEL = 0xD3, K_MOUSE1 = 0x11E,
  K_LAST_KEY = 0x128,
};
enum bindContext_t : int {
  BIND_DEFAULT = 0,
  BIND_MENUS = 1,
  BIND_DRIVE = 2,
  BIND_POSSESS = 3,
  BIND_REMOTE = 4,
  BIND_VEHICLE_GUNNER = 5,
  BIND_VEHICLE_PASSENGER = 6,
};
struct keyBindings_t {
  idStr keyboard;
  idStr mouse;
  idStr gamepad;
  idStr keyboardKey;
  idStr mouseKey;
  idStr gamepadKey;
};
struct userCmdString_t {
  const char* string;
  int button;
};
class idKeyInput {
public:
  static bool IsShiftDown(int inputDevice);
  static bool IsCtrlDown(int inputDevice);
  static const char* KeyNumToString(keyNum_t keyNum);
  static const char* GetBinding(int inputDevice, bindContext_t context, keyNum_t keyNum,
                                int bindingIndex);
  static void KeyBindingsFromBinding(keyBindings_t& result, int inputDevice,
                                     bindContext_t context, const char* binding);
  static idStr KeysFromBinding(int inputDevice, bindContext_t context,
                               const char* binding, bool localized);
};

extern userCmdString_t userCmdStrings[];
