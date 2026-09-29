#pragma once

#include <stdint.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#warning "include/secrets.h not found, using placeholder Wi-Fi credentials from secrets.example.h"
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
// TODO: verify against the official MXN-AMP command strings. Must be the
// amplifier output channels, not Dante input 1 (muting that silences every zone).
constexpr int kOutputChannel[kNumZones] = {1, 2, 3, 4};

// ---- Pins (generic ESP32 DevKit) ----
// Buttons: momentary to GND, internal pull-up, active LOW.
constexpr uint8_t kButtonPin[kNumZones] = {32, 33, 25, 26};
// LEDs: via 220-470 ohm resistor to GND, HIGH = on.
constexpr uint8_t kLedPin[kNumZones] = {16, 17, 18, 19};

// ---- Timing ----
constexpr uint32_t kDebounceMs = 40;
constexpr uint32_t kConnectTimeoutMs = 500;
constexpr uint32_t kReconnectMinMs = 1000;
constexpr uint32_t kReconnectMaxMs = 30000;
constexpr uint32_t kPollIntervalMs = 30000;  // periodic resync, also detects dead links
constexpr uint32_t kBlinkPeriodMs = 1000;    // LED blink while disconnected
