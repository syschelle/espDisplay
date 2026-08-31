#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "models.h"

class ExternalApiService {
 public:
  void begin();
  void tick(const AppSettings& settings, bool wifiConnected);
  void forcePoll();
  void clearCache();

  const ExternalValues& values() const { return values_; }
  bool configured(const AppSettings& settings) const { return settings.apiHost[0] != '\0'; }
  bool requestInProgress() const { return requestState_ != RequestState::Idle; }

 private:
  enum class RequestState : uint8_t {
    Idle,
    Headers,
    Body
  };

  enum class ChunkState : uint8_t {
    Size,
    Data,
    DataCrlf
  };

  ExternalValues values_;
  WiFiClient client_;
  RequestState requestState_ = RequestState::Idle;
  ChunkState chunkState_ = ChunkState::Size;
  bool forcePoll_ = true;
  bool chunked_ = false;
  bool sawHeaderTerminator_ = false;
  int32_t contentLength_ = -1;
  size_t chunkRemaining_ = 0;
  uint8_t chunkCrlfSeen_ = 0;
  uint32_t requestStartedMs_ = 0;
  uint32_t lastActivityMs_ = 0;
  uint32_t lastFinishedMs_ = 0;
  size_t headerLength_ = 0;
  size_t bodyLength_ = 0;
  size_t chunkLineLength_ = 0;
  char headerBuffer_[1025] = "";
  char bodyBuffer_[8193] = "";
  char chunkLine_[24] = "";

  void startRequest(const AppSettings& settings);
  void serviceRequest(const AppSettings& settings);
  void abortRequest();
  void resetRequestBuffers();
  bool consumeByte(uint8_t value);
  bool consumeBodyByte(uint8_t value);
  bool consumeChunkedByte(uint8_t value);
  bool parseHeaders();
  bool finishResponse();
  bool parsePayload(const char* payload, size_t payloadLength, ExternalValues& next, char* error, size_t errorLen);
  void failRequest(int httpStatus, const char* message);
  void markFailure(int httpStatus, const char* message);
  void updateStale(uint16_t pollSeconds);
};

extern ExternalApiService externalApiService;
