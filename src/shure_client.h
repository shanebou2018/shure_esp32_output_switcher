#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <WiFiClient.h>

#include "config.h"

// Keeps one TCP connection to the MXN-AMP (Shure command strings, port 2202),
// tracks each zone's mute state from REP messages, and reconnects with backoff.
class ShureClient {
 public:
  static constexpr int kLogSize = 20;

  void begin(IPAddress ampIp);
  void loop();  // non-blocking except for a short connect attempt

  // Change the amp IP (RAM only) and reconnect.
  void setAmpIp(IPAddress ampIp);
  IPAddress ampIp() const { return ampIp_; }

  bool connected() { return client_.connected(); }

  // Toggle the zone's amp output. The state changes only when the amp's REP arrives.
  bool toggleZone(int zone);

  // true = output unmuted (LED on). false if muted or not yet known.
  const bool* zoneOn() const { return zoneOn_; }

  // Recent TX/RX/status lines, oldest first, and total lines ever logged.
  int logCount() const { return logCount_ < kLogSize ? logCount_ : kLogSize; }
  const String& logLine(int i) const;
  uint32_t logTotal() const { return logCount_; }

 private:
  void tryConnect();
  void send(const char* cmd);
  void readIncoming();
  void handleMessage(const char* msg);
  void log(const String& line);

  WiFiClient client_;
  IPAddress ampIp_;
  bool wasConnected_ = false;
  uint32_t nextConnectAt_ = 0;
  uint32_t backoffMs_ = kReconnectMinMs;
  uint32_t lastPollAt_ = 0;

  char rxBuf_[256];
  size_t rxLen_ = 0;

  bool zoneOn_[kNumZones] = {};

  String log_[kLogSize];
  uint32_t logCount_ = 0;
};
