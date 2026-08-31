#pragma once

#include <Arduino.h>

class OtaService {
 public:
  struct Status {
    bool checked = false;
    bool ok = false;
    bool updateAvailable = false;
    char currentVersion[20] = "";
    char latestVersion[20] = "";
    char firmwareUrl[192] = "";
    uint32_t firmwareSize = 0;
    char lastError[96] = "";
  };

  void begin();
  bool check();
  bool install();
  const Status& status() const { return status_; }

 private:
  Status status_;

  static bool parseSemver(const char* value, int& major, int& minor, int& patch);
  static int compareSemver(const char* a, const char* b);
  static bool isAllowedFirmwareUrl(const char* url);
  void fail(const char* message);
};

extern OtaService otaService;
