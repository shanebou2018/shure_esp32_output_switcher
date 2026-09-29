#include "shure_client.h"

#include <WiFi.h>

void ShureClient::begin(IPAddress ampIp) {
  ampIp_ = ampIp;
  nextConnectAt_ = 0;
}

void ShureClient::setAmpIp(IPAddress ampIp) {
  ampIp_ = ampIp;
  log("Amp IP set to " + ampIp.toString() + " (temporary)");
  client_.stop();
  wasConnected_ = false;
  clearStates();
  backoffMs_ = kReconnectMinMs;
  nextConnectAt_ = 0;  // reconnect on the next loop()
}

void ShureClient::loop() {
  const bool up = client_.connected();

  if (wasConnected_ && !up) {
    log("Disconnected from amp");
    client_.stop();
    clearStates();
    nextConnectAt_ = millis() + backoffMs_;
  }
  wasConnected_ = up;

  if (!up) {
    if (WiFi.status() == WL_CONNECTED && (int32_t)(millis() - nextConnectAt_) >= 0) {
      tryConnect();
    }
    return;
  }

  readIncoming();

  // Periodic resync. Also makes a dead link show up as a failed write.
  if (millis() - lastPollAt_ >= kPollIntervalMs) {
    send("< GET 00 AUDIO_MUTE >");
  }
}

void ShureClient::tryConnect() {
  if (client_.connect(ampIp_, kAmpPort, kConnectTimeoutMs)) {
    client_.setNoDelay(true);
    log("Connected to " + ampIp_.toString() + ":" + String(kAmpPort));
    wasConnected_ = true;
    backoffMs_ = kReconnectMinMs;
    rxLen_ = 0;
    send("< GET 00 AUDIO_MUTE >");  // sync all LEDs
  } else {
    log("Connect to " + ampIp_.toString() + " failed, retry in " + String(backoffMs_ / 1000) + " s");
    nextConnectAt_ = millis() + backoffMs_;
    backoffMs_ = min(backoffMs_ * 2, kReconnectMaxMs);
  }
}

bool ShureClient::pressZone(int zone) {
  if (zone < 0 || zone >= kNumZones || !client_.connected()) return false;
  char cmd[40];
  const int ch = kOutputChannel[zone];
  switch (zoneState_[zone]) {
    case ZoneState::kMuted:
      snprintf(cmd, sizeof(cmd), "< SET %02d AUDIO_MUTE OFF >", ch);
      break;
    case ZoneState::kUnmuted:
      snprintf(cmd, sizeof(cmd), "< SET %02d AUDIO_MUTE ON >", ch);
      break;
    default:
      // Never guess: ask the amp, and let the user press again once the LED shows.
      snprintf(cmd, sizeof(cmd), "< GET %02d AUDIO_MUTE >", ch);
      break;
  }
  send(cmd);
  return true;
}

void ShureClient::clearStates() {
  for (ZoneState& s : zoneState_) s = ZoneState::kUnknown;
}

void ShureClient::send(const char* cmd) {
  lastPollAt_ = millis();
  if (client_.print(cmd) != strlen(cmd)) {
    log(String("Send failed: ") + cmd);
    client_.stop();
    return;
  }
  log(cmd);
}

void ShureClient::readIncoming() {
  while (client_.available() > 0) {
    const int c = client_.read();
    if (c < 0) break;
    if (c == '<') rxLen_ = 0;  // start of a message
    if (rxLen_ < sizeof(rxBuf_) - 1) rxBuf_[rxLen_++] = (char)c;
    if (c == '>') {
      rxBuf_[rxLen_] = '\0';
      if (rxBuf_[0] == '<') handleMessage(rxBuf_);
      rxLen_ = 0;
    }
  }
}

void ShureClient::handleMessage(const char* msg) {
  // Meter samples would flood the log; the firmware never enables them.
  if (strncmp(msg, "< SAMPLE", 8) == 0) return;
  log(msg);

  int ch = 0;
  char value[8] = {};
  if (sscanf(msg, "< REP %d AUDIO_MUTE %7s", &ch, value) != 2) return;
  for (int z = 0; z < kNumZones; z++) {
    if (kOutputChannel[z] == ch) {
      if (strcmp(value, "OFF") == 0) zoneState_[z] = ZoneState::kUnmuted;
      else if (strcmp(value, "ON") == 0) zoneState_[z] = ZoneState::kMuted;
    }
  }
}

void ShureClient::log(const String& line) {
  Serial.println(line);
  log_[logCount_ % kLogSize] = line;
  logCount_++;
}

const String& ShureClient::logLine(int i) const {
  const uint32_t first = logCount_ > kLogSize ? logCount_ - kLogSize : 0;
  return log_[(first + i) % kLogSize];
}
