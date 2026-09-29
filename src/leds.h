#pragma once

#include "zone_state.h"

void ledsBegin();

// Each zone has a green (unmuted) and a red (muted) LED:
//   unmuted -> green on          muted -> red on
//   unknown -> both off          link to amp down -> all red LEDs blink
void ledsUpdate(bool connected, const ZoneState states[]);
