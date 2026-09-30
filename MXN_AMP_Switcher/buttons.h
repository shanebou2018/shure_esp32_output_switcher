#pragma once

// Debounced press-edge detection for the zone buttons.
void buttonsBegin();

// Returns the zone index (0..kNumZones-1) of a new press, or -1 if none.
// Call every loop(); reports at most one press per call.
int buttonsPoll();
