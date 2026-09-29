#include "leds.h"

#include <Arduino.h>

#include "config.h"

void ledsBegin() {
  for (int i = 0; i < kNumZones; i++) {
    pinMode(kGreenLedPin[i], OUTPUT);
    pinMode(kRedLedPin[i], OUTPUT);
    digitalWrite(kGreenLedPin[i], LOW);
    digitalWrite(kRedLedPin[i], LOW);
  }
}

void ledsUpdate(bool connected, const ZoneState states[]) {
  const bool blink = (millis() % kBlinkPeriodMs) < kBlinkPeriodMs / 2;
  for (int i = 0; i < kNumZones; i++) {
    bool green = false;
    bool red = false;
    if (!connected) {
      red = blink;
    } else if (states[i] == ZoneState::kUnmuted) {
      green = true;
    } else if (states[i] == ZoneState::kMuted) {
      red = true;
    }
    digitalWrite(kGreenLedPin[i], green ? HIGH : LOW);
    digitalWrite(kRedLedPin[i], red ? HIGH : LOW);
  }
}
