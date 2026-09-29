#include "web.h"

#include <WebServer.h>

#include "config.h"
#include "shure_client.h"

// web/index.html, embedded by board_build.embed_txtfiles (NUL-terminated).
extern const char indexHtml[] asm("_binary_web_index_html_start");

namespace {

WebServer server(80);
ShureClient* amp = nullptr;

void appendJsonString(String& out, const String& s) {
  out += '"';
  for (char c : s) {
    if (c == '"' || c == '\\') {
      out += '\\';
      out += c;
    } else if ((uint8_t)c < 0x20) {
      out += ' ';
    } else {
      out += c;
    }
  }
  out += '"';
}

void handleState() {
  String json = "{\"ampIp\":";
  appendJsonString(json, amp->ampIp().toString());
  json += ",\"connected\":";
  json += amp->connected() ? "true" : "false";
  json += ",\"outputs\":[";
  for (int i = 0; i < kNumZones; i++) {
    if (i) json += ',';
    json += amp->zoneOn()[i] ? "true" : "false";
  }
  json += "],\"log\":[";
  for (int i = 0; i < amp->logCount(); i++) {
    if (i) json += ',';
    appendJsonString(json, amp->logLine(i));
  }
  json += "],\"logTotal\":";
  json += amp->logTotal();
  json += '}';
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void handlePress() {
  const int ch = server.arg("ch").toInt();  // 1..kNumZones
  if (ch < 1 || ch > kNumZones) {
    server.send(400, "text/plain", "ch must be 1-4");
    return;
  }
  if (!amp->toggleZone(ch - 1)) {
    server.send(503, "text/plain", "amp not connected");
    return;
  }
  server.send(204);
}

void handleIp() {
  IPAddress ip;
  if (!ip.fromString(server.arg("ip"))) {
    server.send(400, "text/plain", "invalid ip");
    return;
  }
  amp->setAmpIp(ip);
  server.send(204);
}

}  // namespace

void webBegin(ShureClient& a) {
  amp = &a;
  server.on("/", HTTP_GET, [] { server.send(200, "text/html", indexHtml); });
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/press", HTTP_POST, handlePress);
  server.on("/api/ip", HTTP_POST, handleIp);
  server.onNotFound([] { server.send(404, "text/plain", "not found"); });
  server.begin();
}

void webLoop() { server.handleClient(); }
