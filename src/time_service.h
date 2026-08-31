#pragma once

#include <Arduino.h>
#include <time.h>
#include "models.h"

class TimeService {
 public:
  void begin();
  bool bootSynchronize(const AppSettings& settings, bool wifiConnected);
  void tick(const AppSettings& settings, bool wifiConnected);
  void forceResync(const AppSettings& settings, bool wifiConnected);

  bool synchronized() const { return synchronized_; }
  bool syncPending() const { return syncPending_; }
  time_t lastSyncEpoch() const { return lastSyncEpoch_; }
  uint32_t lastAttemptMs() const { return lastAttemptMs_; }

  bool getLocalTm(struct tm& out) const;
  String localTimeString() const;
  String lastSyncString() const;

  void onSntpUpdate(bool fromSntp);

 private:
  bool synchronized_ = false;
  bool syncPending_ = false;
  uint32_t syncStartedMs_ = 0;
  uint32_t lastAttemptMs_ = 0;
  time_t lastSyncEpoch_ = 0;

  void startSync(const AppSettings& settings);
};

extern TimeService timeService;
