#pragma once

#include "list.h"

template <typename T, int Capacity>
class idStaticList : public idList<T, 5> {
public:
  idStaticList() {
    this->list = staticList;
    this->num = 0;
    this->size = Capacity;
    this->listStatic = 1;
  }
  ~idStaticList() = default;

  T staticList[Capacity];
};
