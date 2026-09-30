#include "buttons.h"

#include <Arduino.h>

#include "config.h"

namespace {

struct Button {
  bool stable = false;  // debounced state, true = pressed
  bool raw = false;
  uint32_t changedAt = 0;
};

Button buttons[kNumZones];

}  // namespace

void buttonsBegin() {
  for (int i = 0; i < kNumZones; i++) {
    pinMode(kButtonPin[i], INPUT_PULLUP);
  }
}

int buttonsPoll() {
  const uint32_t now = millis();
  int pressed = -1;
  for (int i = 0; i < kNumZones; i++) {
    Button& b = buttons[i];
    const bool raw = digitalRead(kButtonPin[i]) == LOW;
    if (raw != b.raw) {
      b.raw = raw;
      b.changedAt = now;
    }
    if (raw != b.stable && now - b.changedAt >= kDebounceMs) {
      b.stable = raw;
      if (raw && pressed < 0) pressed = i;  // press edge only
    }
  }
  return pressed;
}
