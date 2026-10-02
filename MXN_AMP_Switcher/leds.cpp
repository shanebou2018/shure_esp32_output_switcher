#include "leds.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "config.h"

namespace {

Adafruit_NeoPixel pixels(kNumZones, kLedDataPin, LED_PIXEL_TYPE);

constexpr uint32_t kOff = 0;
uint32_t shown[kNumZones];
bool shownValid = false;

}  // namespace

void ledsBegin() {
  pixels.begin();
  pixels.setBrightness(kLedBrightness);
  pixels.clear();
  pixels.show();
}

void ledsUpdate(bool connected, const ZoneState states[]) {
  const uint32_t green = Adafruit_NeoPixel::Color(0, 255, 0);
  const uint32_t red = Adafruit_NeoPixel::Color(255, 0, 0);
  const bool blink = (millis() % kBlinkPeriodMs) < kBlinkPeriodMs / 2;

  uint32_t want[kNumZones];
  for (int i = 0; i < kNumZones; i++) {
    if (!connected) {
      want[i] = blink ? red : kOff;
    } else if (states[i] == ZoneState::kUnmuted) {
      want[i] = green;
    } else if (states[i] == ZoneState::kMuted) {
      want[i] = red;
    } else {
      want[i] = kOff;
    }
  }

  // show() briefly blocks interrupts, so only push data when something changed.
  if (shownValid && memcmp(want, shown, sizeof(want)) == 0) return;
  for (int i = 0; i < kNumZones; i++) pixels.setPixelColor(i, want[i]);
  pixels.show();
  memcpy(shown, want, sizeof(shown));
  shownValid = true;
}
