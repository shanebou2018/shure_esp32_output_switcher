#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <WiFiClient.h>

#include "config.h"
#include "zone_state.h"

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

  // Button press for a zone: sends an explicit mute/unmute that flips the state
  // the amp last reported (muted -> OFF, unmuted -> ON). If the state is not
  // known yet it only asks the amp for it. The zone's state changes only when
  // the amp's REP arrives. Returns false if nothing could be sent.
  bool pressZone(int zone);

  const ZoneState* zoneStates() const { return zoneState_; }

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
  void clearStates();

  WiFiClient client_;
  IPAddress ampIp_;
  bool wasConnected_ = false;
  uint32_t nextConnectAt_ = 0;
  uint32_t backoffMs_ = kReconnectMinMs;
  uint32_t lastPollAt_ = 0;

  char rxBuf_[256];
  size_t rxLen_ = 0;

  ZoneState zoneState_[kNumZones] = {};  // all kUnknown

  String log_[kLogSize];
  uint32_t logCount_ = 0;
};
