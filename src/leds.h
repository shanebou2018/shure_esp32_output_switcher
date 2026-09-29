#pragma once

void ledsBegin();

// connected: link to the amp is up. on[i]: zone i output is unmuted.
// While disconnected all LEDs blink together.
void ledsUpdate(bool connected, const bool on[]);
