#pragma once

#include <Arduino.h>
#include "config.h"

enum class LogLevel : uint8_t {
  Info,
  Warn,
  Error
};

class LogBuffer {
 public:
  void begin();
  void info(const char* component, const char* message);
  void warn(const char* component, const char* message);
  void error(const char* component, const char* message);
  void clear();

  size_t count() const;
  const char* lineAtChronological(size_t index) const;

 private:
  void add(LogLevel level, const char* component, const char* message);

  char lines_[LOG_LINE_COUNT][LOG_LINE_LENGTH] = {};
  uint8_t next_ = 0;
  uint8_t count_ = 0;
};

extern LogBuffer appLog;
