#pragma once

#include "engine/gamelib/aas2/aas2callback_avoidlocation.h"

#include <cstddef>

class idAAS2 {
public:
  virtual int Slot00() = 0; virtual int Slot01() = 0;
  virtual int Slot02() = 0; virtual int Slot03() = 0;
  virtual int Slot04() = 0; virtual int Slot05() = 0;
  virtual int Slot06() = 0; virtual int Slot07() = 0;
  virtual int Slot08() = 0; virtual int Slot09() = 0;
  virtual int Slot10() = 0; virtual int Slot11() = 0;
  virtual int Slot12() = 0; virtual int Slot13() = 0;
  virtual int Slot14() = 0; virtual int Slot15() = 0;
  virtual int Slot16() = 0; virtual int Slot17() = 0;
  virtual int Slot18() = 0; virtual int Slot19() = 0;
  virtual int Slot20() = 0; virtual int Slot21() = 0;
  virtual int Slot22() = 0;
  virtual int GetAreaFlags(int areaNum) const = 0;
};

class idAAS2Callback_FindFlaggedArea final
    : public idAAS2Callback_AvoidLocation {
public:
  idAAS2Callback_FindFlaggedArea(int areaFlag, bool set);
  ~idAAS2Callback_FindFlaggedArea() override;
  bool AreaIsGoal(const idAAS2* aas, int areaNum,
                  const idVec3& potentialGoal) override;

  int areaFlag;
  int test;
};

static_assert(offsetof(idAAS2Callback_FindFlaggedArea, areaFlag) == 0x28,
              "idAAS2Callback_FindFlaggedArea ABI drift");
static_assert(sizeof(idAAS2Callback_FindFlaggedArea) == 0x30,
              "idAAS2Callback_FindFlaggedArea ABI drift");
