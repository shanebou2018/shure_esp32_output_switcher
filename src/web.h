#pragma once

class ShureClient;

// HTTP server: serves web/index.html and the /api endpoints (see CLAUDE.md).
void webBegin(ShureClient& amp);
void webLoop();
