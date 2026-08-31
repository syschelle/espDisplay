#include "validation.h"

#include <ctype.h>
#include <string.h>

bool isValidHostValue(const char* value) {
  if (!value) return false;
  const size_t len = strlen(value);
  if (len == 0 || len > 64) return false;
  if (value[0] == '.' || value[0] == '-' || value[len - 1] == '.' || value[len - 1] == '-') {
    return false;
  }

  for (size_t i = 0; i < len; ++i) {
    const char c = value[i];
    if (!(isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-')) {
      return false;
    }
  }

  // A host/IP is accepted, not a URL, path, port or traversal-like token.
  return strstr(value, "..") == nullptr;
}
