#include "ota.h"

#include <ESP8266WiFi.h>
#include <Updater.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "text_utils.h"
#include "version.h"

OtaService otaService;

namespace {

void logHeapState(const char* phase) {
  char message[120];
  snprintf(
      message,
      sizeof(message),
      "%s (free %lu, max block %lu, frag %u%%)",
      phase ? phase : "OTA",
      static_cast<unsigned long>(ESP.getFreeHeap()),
      static_cast<unsigned long>(ESP.getMaxFreeBlockSize()),
      static_cast<unsigned>(ESP.getHeapFragmentation()));
  appLog.info("OTA", message);
}

}  // namespace

void OtaService::begin() {
  status_ = Status();
  copyText(status_.currentVersion, FW_VERSION);
}

void OtaService::resetUpdaterIfRunning() {
  if (Update.isRunning()) {
    // end(false) intentionally fails an incomplete update and resets the
    // updater's internal buffer/state without scheduling a reboot.
    (void)Update.end(false);
  }
}

void OtaService::fail(const char* message) {
  status_.ok = false;
  status_.inProgress = false;
  status_.completed = false;
  copyText(status_.lastError, message ? message : "Unknown OTA error");
  appLog.warn("OTA", status_.lastError);
}

bool OtaService::beginUpload(const char* targetVersion, uint32_t expectedSize) {
  resetUpdaterIfRunning();
  status_ = Status();
  copyText(status_.currentVersion, FW_VERSION);

  int targetMajor = 0, targetMinor = 0, targetPatch = 0;
  if (!parseSemver(targetVersion, targetMajor, targetMinor, targetPatch)) {
    fail("OTA target version is not a semantic version");
    return false;
  }
  if (compareSemver(FW_VERSION, targetVersion) >= 0) {
    fail("OTA target version is not newer than installed firmware");
    return false;
  }
  if (expectedSize == 0) {
    fail("OTA firmware size is invalid");
    return false;
  }
  if (expectedSize > static_cast<uint32_t>(ESP.getFreeSketchSpace())) {
    fail("Firmware image does not fit the available OTA flash space");
    return false;
  }

  copyText(status_.targetVersion, targetVersion);
  status_.expectedSize = expectedSize;
  status_.writtenSize = 0;
  status_.lastError[0] = '\0';

  logHeapState("Browser OTA upload starting");
  Update.clearError();
  if (!Update.begin(expectedSize, U_FLASH)) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Flash initialization failed: %u %.48s",
        static_cast<unsigned>(Update.getError()),
        Update.getErrorString().c_str());
    fail(message);
    return false;
  }

  status_.inProgress = true;
  status_.ok = true;
  appLog.info("OTA", "Browser firmware upload accepted");
  return true;
}

bool OtaService::writeChunk(uint8_t* data, size_t length) {
  if (!status_.inProgress || !Update.isRunning()) {
    fail("OTA upload is not initialized");
    return false;
  }
  if (!data || length == 0) return true;

  if (status_.writtenSize == 0 && data[0] != 0xE9) {
    resetUpdaterIfRunning();
    fail("Uploaded file is not a valid ESP8266 firmware image");
    return false;
  }

  if (status_.writtenSize + length > status_.expectedSize) {
    resetUpdaterIfRunning();
    fail("OTA upload exceeds expected firmware size");
    return false;
  }

  const size_t written = Update.write(data, length);
  status_.writtenSize += static_cast<uint32_t>(written);
  if (written != length) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware write failed at %lu/%lu bytes: %u %.32s",
        static_cast<unsigned long>(status_.writtenSize),
        static_cast<unsigned long>(status_.expectedSize),
        static_cast<unsigned>(Update.getError()),
        Update.getErrorString().c_str());
    fail(message);
    return false;
  }

  yield();
  return true;
}

bool OtaService::finishUpload(uint32_t uploadedSize) {
  if (!status_.inProgress || !Update.isRunning()) {
    fail("OTA upload is not active");
    return false;
  }

  if (uploadedSize != status_.expectedSize || status_.writtenSize != status_.expectedSize) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware upload incomplete: %lu/%lu bytes",
        static_cast<unsigned long>(status_.writtenSize),
        static_cast<unsigned long>(status_.expectedSize));
    resetUpdaterIfRunning();
    fail(message);
    return false;
  }

  if (!Update.end()) {
    char message[96];
    snprintf(
        message,
        sizeof(message),
        "Firmware finalization failed: %u %.48s",
        static_cast<unsigned>(Update.getError()),
        Update.getErrorString().c_str());
    fail(message);
    return false;
  }

  status_.inProgress = false;
  status_.completed = true;
  status_.ok = true;
  status_.lastError[0] = '\0';
  appLog.info("OTA", "Browser upload flashed successfully; reboot scheduled");
  return true;
}

void OtaService::abortUpload(const char* reason) {
  resetUpdaterIfRunning();
  if (status_.inProgress || reason) {
    status_.inProgress = false;
    status_.completed = false;
    if (reason && *reason) {
      fail(reason);
      return;
    }
  }
  status_.inProgress = false;
}

bool OtaService::parseSemver(const char* value, int& major, int& minor, int& patch) {
  if (!value || !*value) return false;
  const char* p = value;
  if (*p == 'v' || *p == 'V') ++p;

  char* end = nullptr;
  long a = strtol(p, &end, 10);
  if (end == p || *end != '.') return false;

  p = end + 1;
  long b = strtol(p, &end, 10);
  if (end == p || *end != '.') return false;

  p = end + 1;
  long c = strtol(p, &end, 10);
  if (end == p || *end != '\0') return false;

  if (a < 0 || b < 0 || c < 0 || a > 32767 || b > 32767 || c > 32767) {
    return false;
  }

  major = static_cast<int>(a);
  minor = static_cast<int>(b);
  patch = static_cast<int>(c);
  return true;
}

int OtaService::compareSemver(const char* a, const char* b) {
  int am = 0, an = 0, ap = 0;
  int bm = 0, bn = 0, bp = 0;

  const bool aOk = parseSemver(a, am, an, ap);
  const bool bOk = parseSemver(b, bm, bn, bp);

  if (!aOk && bOk) return -1;
  if (aOk && !bOk) return 1;
  if (!aOk && !bOk) return 0;

  if (am != bm) return am < bm ? -1 : 1;
  if (an != bn) return an < bn ? -1 : 1;
  if (ap != bp) return ap < bp ? -1 : 1;
  return 0;
}
