#pragma once

#include "zone_state.h"

void ledsBegin();

// One WS2811 RGB pixel per zone:
//   unmuted -> green      muted -> red      unknown -> off
//   link to amp down -> all pixels blink red together
void ledsUpdate(bool connected, const ZoneState states[]);
