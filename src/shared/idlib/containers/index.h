#pragma once

template <typename T, typename InvalidTag> class idIndex {
public:
  T value = static_cast<T>(-1);

  constexpr idIndex() = default;
  constexpr explicit idIndex(T initial) : value(initial) {}
};

