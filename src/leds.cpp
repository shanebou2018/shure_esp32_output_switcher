#include "leds.h"

#include <Arduino.h>

#include "config.h"

void ledsBegin() {
  for (int i = 0; i < kNumZones; i++) {
    pinMode(kLedPin[i], OUTPUT);
    digitalWrite(kLedPin[i], LOW);
  }
}

void ledsUpdate(bool connected, const bool on[]) {
  const bool blink = (millis() % kBlinkPeriodMs) < kBlinkPeriodMs / 2;
  for (int i = 0; i < kNumZones; i++) {
    digitalWrite(kLedPin[i], (connected ? on[i] : blink) ? HIGH : LOW);
  }
}
