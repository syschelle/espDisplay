#pragma once

#include <stddef.h>

// Copy a C string into a fixed buffer and always NUL-terminate it.
// The source is truncated when necessary and is always terminated explicitly.
inline void copyText(char* dest, size_t destSize, const char* src) {
  if (!dest || destSize == 0) return;

  size_t i = 0;
  if (src) {
    while (i + 1 < destSize && src[i] != '\0') {
      dest[i] = src[i];
      ++i;
    }
  }
  dest[i] = '\0';
}

template <size_t N>
inline void copyText(char (&dest)[N], const char* src) {
  copyText(dest, N, src);
}
