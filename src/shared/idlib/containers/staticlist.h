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
  idStaticList(const idStaticList& other) : idStaticList() {
    *this=other;
  }
  idStaticList& operator=(const idStaticList& other) {
    if(this!=&other) {
      this->num=other.num;
      for(int i=0;i<Capacity;++i) staticList[i]=other.staticList[i];
    }
    return *this;
  }
  ~idStaticList() = default;

  T* begin() noexcept { return staticList; }
  const T* begin() const noexcept { return staticList; }
  T* end() noexcept { return staticList+this->num; }
  const T* end() const noexcept { return staticList+this->num; }

  void fill(const T& value) {
    for(int i=0;i<Capacity;++i) staticList[i]=value;
    this->num=Capacity;
  }

  T staticList[Capacity];
};
