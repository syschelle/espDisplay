#include <assert.h>
#include <stdio.h>
#include "validation.h"

int main() {
  assert(isValidHostValue("192.168.178.50"));
  assert(isValidHostValue("energy-backend.local"));
  assert(isValidHostValue("backend-01"));
  assert(!isValidHostValue("http://192.168.178.50"));
  assert(!isValidHostValue("192.168.178.50/api/current-values"));
  assert(!isValidHostValue("192.168.178.50:8080"));
  assert(!isValidHostValue("../metadata"));
  assert(!isValidHostValue(""));
  puts("host validation tests passed");
  return 0;
}
