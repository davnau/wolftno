#pragma once

// Retail idArray is a fixed-capacity aggregate with no bookkeeping fields.
template <typename T, int Count> struct idArray {
  T ptr[Count];
};

