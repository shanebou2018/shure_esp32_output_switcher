#pragma once

#include <stdint.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#warning "secrets.h not found, using placeholder Wi-Fi credentials from secrets.example.h"
#include "secrets.example.h"
#endif

// ---- Network ----
constexpr const char* kWifiSsid = "RDA WIFI";
constexpr const char* kWifiPassword = "RichardDean1";
constexpr const char* kHostname = "mxn-switcher";  // http://mxn-switcher.local/

// Default amp control IP. Can be changed temporarily from the web page (RAM only).
constexpr const char* kDefaultAmpIp = "10.0.1.142";
constexpr uint16_t kAmpPort = 2202;

// ---- Zones ----
constexpr int kNumZones = 4;

// Shure channel number of the amp OUTPUT for each zone (button 1..4).
// Confirmed on the MXN-AMP: amplifier outputs are channels 06-09. Never use
// the input channels here; muting Dante input 1 would silence every zone.
constexpr int kOutputChannel[kNumZones] = {6, 7, 8, 9};

// ---- Pins (generic ESP32 DevKit) ----
// Buttons: momentary to GND, internal pull-up, active LOW.
constexpr uint8_t kButtonPin[kNumZones] = {32, 33, 25, 26};
// LEDs: one WS2811 RGB pixel per zone, chained on a single data pin.
// Pixel 0 = zone 1 ... pixel 3 = zone 4 (first pixel on the chain = zone 1).
constexpr uint8_t kLedDataPin = 16;
constexpr uint8_t kLedBrightness = 80;  // 0-255
// WS2811 pixels are usually RGB order at 800 kHz. If "unmuted" shows red and
// "muted" shows green, change NEO_RGB to NEO_GRB. Some older WS2811 parts
// need NEO_KHZ400 instead of NEO_KHZ800.
#define LED_PIXEL_TYPE (NEO_RGB + NEO_KHZ800)

// ---- Timing ----
constexpr uint32_t kDebounceMs = 40;
constexpr uint32_t kConnectTimeoutMs = 500;
constexpr uint32_t kReconnectMinMs = 1000;
constexpr uint32_t kReconnectMaxMs = 30000;
constexpr uint32_t kPollIntervalMs = 30000;  // periodic resync, also detects dead links
constexpr uint32_t kBlinkPeriodMs = 1000;    // all pixels blink red while disconnected
