#pragma once

#include "list.h"

template <typename T, int Capacity>
class idStaticList : public idList<T, 5> {
public:
  idStaticList() noexcept {
    this->list = staticList;
    this->num = 0;
    this->size = Capacity;
    this->granularity = 1;
    this->listStatic = 1;
  }
  ~idStaticList() = default;

  T staticList[Capacity];
};
