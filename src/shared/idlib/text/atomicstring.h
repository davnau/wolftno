#pragma once

class idAtomicStringManager {
public:
  const char* MakeAtomic(const char* text);
  void FreeDynamic();
};

class idAtomicString {
public:
  const char* str;
  idAtomicString() : str("") {}
  explicit idAtomicString(const char* text) : str("") { Set(text); }
  void Set(const char* text);
  static void FreeDynamic();
  operator const char*() const { return str; }
};

extern idAtomicStringManager atomicStringManager;
extern idAtomicString atsTestStatic;
static_assert(sizeof(idAtomicString) == 8, "idAtomicString ABI drift");
