#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "buttons.h"
#include "config.h"
#include "leds.h"
#include "shure_client.h"
#include "web.h"

namespace {

ShureClient amp;
bool wifiWasUp = false;

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("\nMXN-AMP output switcher");

  buttonsBegin();
  ledsBegin();

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(kHostname);
  WiFi.setAutoReconnect(true);
  WiFi.begin(kWifiSsid, kWifiPassword);
  Serial.printf("Wi-Fi: connecting to \"%s\"\n", kWifiSsid);

  IPAddress ampIp;
  ampIp.fromString(kDefaultAmpIp);
  amp.begin(ampIp);
  webBegin(amp);
}

void loop() {
  const bool wifiUp = WiFi.status() == WL_CONNECTED;
  if (wifiUp && !wifiWasUp) {
    Serial.printf("Wi-Fi: connected, IP %s, open http://%s.local/\n",
                  WiFi.localIP().toString().c_str(), kHostname);
    MDNS.end();
    if (MDNS.begin(kHostname)) MDNS.addService("http", "tcp", 80);
  } else if (!wifiUp && wifiWasUp) {
    Serial.println("Wi-Fi: disconnected");
  }
  wifiWasUp = wifiUp;

  amp.loop();

  const int zone = buttonsPoll();
  if (zone >= 0) amp.pressZone(zone);

  webLoop();
  ledsUpdate(amp.connected(), amp.zoneStates());
}
