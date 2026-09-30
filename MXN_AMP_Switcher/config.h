#pragma once

#include <stdint.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#warning "secrets.h not found, using placeholder Wi-Fi credentials from secrets.example.h"
#include "secrets.example.h"
#endif

// ---- Network ----
constexpr const char* kWifiSsid = WIFI_SSID;
constexpr const char* kWifiPassword = WIFI_PASSWORD;
constexpr const char* kHostname = "mxn-switcher";  // http://mxn-switcher.local/

// Default amp control IP. Can be changed temporarily from the web page (RAM only).
constexpr const char* kDefaultAmpIp = "192.168.1.50";
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
// LEDs: via 220-470 ohm resistor to GND, HIGH = on. Two per zone.
constexpr uint8_t kGreenLedPin[kNumZones] = {16, 17, 18, 19};  // unmuted
constexpr uint8_t kRedLedPin[kNumZones] = {21, 22, 23, 27};    // muted

// ---- Timing ----
constexpr uint32_t kDebounceMs = 40;
constexpr uint32_t kConnectTimeoutMs = 500;
constexpr uint32_t kReconnectMinMs = 1000;
constexpr uint32_t kReconnectMaxMs = 30000;
constexpr uint32_t kPollIntervalMs = 30000;  // periodic resync, also detects dead links
constexpr uint32_t kBlinkPeriodMs = 1000;    // red LED blink while disconnected
