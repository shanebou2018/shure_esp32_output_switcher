#pragma once

#include <stdint.h>

// Mute state of one zone's amp output, as last reported by the amp.
enum class ZoneState : uint8_t {
  kUnknown,  // no REP yet (just connected, or link lost)
  kMuted,    // red LED
  kUnmuted,  // green LED
};
