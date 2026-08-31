#include "logging.h"

LogBuffer appLog;

static const char* levelName(LogLevel level) {
  switch (level) {
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERROR";
    default: return "INFO";
  }
}

void LogBuffer::begin() {
  clear();
}

void LogBuffer::info(const char* component, const char* message) {
  add(LogLevel::Info, component, message);
}

void LogBuffer::warn(const char* component, const char* message) {
  add(LogLevel::Warn, component, message);
}

void LogBuffer::error(const char* component, const char* message) {
  add(LogLevel::Error, component, message);
}

void LogBuffer::clear() {
  memset(lines_, 0, sizeof(lines_));
  next_ = 0;
  count_ = 0;
}

void LogBuffer::add(LogLevel level, const char* component, const char* message) {
  const uint32_t seconds = millis() / 1000UL;
  snprintf(
      lines_[next_],
      LOG_LINE_LENGTH,
      "[%06lus] [%s] [%s] %s",
      static_cast<unsigned long>(seconds),
      levelName(level),
      component ? component : "APP",
      message ? message : "");

  Serial.println(lines_[next_]);

  next_ = static_cast<uint8_t>((next_ + 1) % LOG_LINE_COUNT);
  if (count_ < LOG_LINE_COUNT) {
    ++count_;
  }
}

size_t LogBuffer::count() const {
  return count_;
}

const char* LogBuffer::lineAtChronological(size_t index) const {
  if (index >= count_) {
    return "";
  }

  const size_t first = (next_ + LOG_LINE_COUNT - count_) % LOG_LINE_COUNT;
  return lines_[(first + index) % LOG_LINE_COUNT];
}
