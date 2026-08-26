#pragma once

#include "list.h"

template <typename T, int Capacity>
class idStaticList : public idList<T, 5> {
public:
  idStaticList()
#if !defined(WOLFTNO_RETAIL_STATICLIST_THROWING_CTOR_ABI)
      noexcept
#endif
  {
#if defined(WOLFTNO_RETAIL_CUTTABLE_STATICLIST_CTOR_ABI)
    this->size = Capacity;
    this->num = 0;
    this->list = staticList;
    *reinterpret_cast<std::uint32_t*>(&this->granularity) = 0x01050001u;
#elif defined(WOLFTNO_RETAIL_EFFECTS_STATICLIST_CTOR_ABI)
    this->size = Capacity;
    this->num = 0;
    *reinterpret_cast<volatile std::uint16_t*>(&this->granularity) = 1;
    this->list = staticList;
    *reinterpret_cast<std::uint16_t*>(&this->memTag) = 0x0105;
#else
    this->list = staticList;
    this->num = 0;
    this->size = Capacity;
    this->granularity = 1;
    this->listStatic = 1;
#endif
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
