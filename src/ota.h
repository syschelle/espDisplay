#pragma once

#include <Arduino.h>

class OtaService {
 public:
  struct Status {
    bool inProgress = false;
    bool completed = false;
    bool ok = true;
    char currentVersion[20] = "";
    char targetVersion[20] = "";
    uint32_t expectedSize = 0;
    uint32_t writtenSize = 0;
    char lastError[96] = "";
  };

  void begin();
  bool beginUpload(const char* targetVersion, uint32_t expectedSize);
  bool writeChunk(uint8_t* data, size_t length);
  bool finishUpload(uint32_t uploadedSize);
  void abortUpload(const char* reason = nullptr);
  const Status& status() const { return status_; }

  static bool parseSemver(const char* value, int& major, int& minor, int& patch);
  static int compareSemver(const char* a, const char* b);

 private:
  Status status_;

  void fail(const char* message);
  void resetUpdaterIfRunning();
};

extern OtaService otaService;
