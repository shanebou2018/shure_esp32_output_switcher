# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project

An ESP32 wall/desk controller that switches the outputs of a **Shure MXN-AMP**
(PoE+ powered 4-channel Dante amplifier) using **4 push buttons** with
**8 LEDs** (a green and a red per zone) for latching feedback.

- Each button is paired with one amplifier output channel (zone 1–4 → amp output 06–09).
- Pressing a button flips (latches) that output between muted and unmuted.
- Buttons are **independent** (decided): any combination of zones can be on,
  including all or none.
- One press sends **one explicit command**, chosen from the state the amp last
  reported. **`TOGGLE` is never used.**
  - Zone reported muted → `< SET nn AUDIO_MUTE OFF >`
  - Zone reported unmuted → `< SET nn AUDIO_MUTE ON >`
  - State not known yet → `< GET nn AUDIO_MUTE >` only. Never guess.
- LEDs must match the amp **100%**. They show only what the amp reported:

  | Amp state | Green LED | Red LED |
  |---|---|---|
  | Unmuted | on | off |
  | Muted | off | on |
  | Not known yet (just connected) | off | off |
  | ESP32 not connected to the amp | off | all blink together |

- If the amp changes state from elsewhere (Designer, web UI, another
  controller), the LEDs follow it.

Control is done over the network with **Shure third-party command strings**
(ASCII over TCP port 2202). Dante is used only for audio transport between
the source and the amp. The ESP32 does not touch Dante routing. See
`MXN-AMP.md` for the API reference and the reasons for this choice.

### Signal flow

```
Source (Dante) ──► MXN-AMP Dante input 1 ──┬──► Amp output 1 ──► Zone 1   [Button/LED 1]
                                           ├──► Amp output 2 ──► Zone 2   [Button/LED 2]
                                           ├──► Amp output 3 ──► Zone 3   [Button/LED 3]
                                           └──► Amp output 4 ──► Zone 4   [Button/LED 4]
                                                    ▲
                                  ESP32 only mutes/unmutes here
```

- Audio always reaches all four amp channels. This is set up once, either by
  subscribing all four MXN-AMP Dante inputs to the same source channel in
  Dante Controller, or with the amp's internal routing if it has any. The
  ESP32 never changes it.
- The buttons mute/unmute the **amp output** channels only: **06, 07, 08,
  09** for zones 1–4 (confirmed on the amp, `kOutputChannel` in `config.h`).
  Never mute the input channels, because that silences every zone.
  `REP` messages for any other channel are ignored.

## Hardware

| Item | Notes |
|---|---|
| MCU | **Generic ESP32 DevKit (esp32dev) over Wi-Fi** (decided). |
| Buttons | 4 momentary push buttons, wired to GND, using the internal pull-up (`INPUT_PULLUP`), active LOW. |
| LEDs | 8 LEDs: 4 green (unmuted) + 4 red (muted), each with its own series resistor (~220–470 Ω) to GND, driven HIGH = on. |
| Network | The Wi-Fi network must be able to reach the MXN-AMP **control** IP on TCP 2202. |

Pin map (`MXN_AMP_Switcher/config.h`). These pins all have internal pull-ups and avoid
the strapping pins (0, 2, 5, 12, 15) and the flash pins (6–11):

| Zone | Amp output | Button GPIO | Green LED GPIO | Red LED GPIO |
|---|---|---|---|---|
| 1 | 06 | 32 | 16 | 21 |
| 2 | 07 | 33 | 17 | 22 |
| 3 | 08 | 25 | 18 | 23 |
| 4 | 09 | 26 | 19 | 27 |

## Wi-Fi credentials

- They live in `MXN_AMP_Switcher/secrets.h`, which is **git-ignored because
  the repo is public**. In the sketch folder, copy `secrets.example.h` to
  `secrets.h` and set `WIFI_SSID` / `WIFI_PASSWORD`. The current site uses SSID `Sound`; the
  password is in your local file only.
- If `secrets.h` is missing, the build warns and uses placeholders.
- Planned: make the SSID/password changeable at runtime (web page + NVS
  `Preferences`), keeping `secrets.h` as the default.
- After it connects, the device is reachable at `http://mxn-switcher.local/`
  (mDNS) or at the IP printed on the serial monitor.

## Software stack

- **Arduino IDE** (decided: no PlatformIO). The firmware is one sketch folder,
  `MXN_AMP_Switcher/`. The IDE compiles every `.ino`, `.cpp` and `.h` in it.
- Board package: **esp32 by Espressif** (Boards Manager). Board: **ESP32 Dev Module**.
- **No extra libraries.** `WiFi`, `WiFiClient`, `WebServer` and `ESPmDNS` all
  ship with the ESP32 board package.
- The web page is compiled in from `index_html.h`, which is **generated** from
  `web/index.html`. After editing the page, run `python3 tools/embed_web.py`
  and commit both files. Never hand-edit `index_html.h`.
- Debounce and the parsing of command strings are done in plain code in this project.

Layout:

```
MXN_AMP_Switcher/
  MXN_AMP_Switcher.ino  # setup/loop, wiring the modules together
  config.h              # pins, amp IP/port, zone->amp output channel map, timings
  secrets.example.h     # template; copy to secrets.h (git-ignored) for Wi-Fi creds
  shure_client.*        # TCP connection, send/parse Shure command strings, reconnect
  buttons.*             # debounced edge detection for 4 buttons
  zone_state.h          # ZoneState: kUnknown / kMuted / kUnmuted
  leds.*                # green/red LED output + "no connection" blink pattern
  web.*                 # HTTP server: serves the page + /api/* endpoints
  index_html.h          # GENERATED from web/index.html by tools/embed_web.py
web/index.html          # control page source (opens standalone in simulation mode)
tools/embed_web.py      # regenerates index_html.h
MXN-AMP.md              # Shure MXN-AMP + Dante API reference notes
```

## Web page (`web/index.html`)

A single self-contained HTML file with no external dependencies. The ESP32
serves it at `/`. It lets someone temporarily change the amp IP, press the
4 outputs, and see the LEDs mirror the amp's state, plus a log of the Shure
strings sent and received.

- **Simulation mode:** if `/api/state` cannot be reached (for example when
  the file is opened straight from disk), a fake amp inside the page answers
  with `REP` strings. Use this to try the UI without hardware.
- **Live mode:** when the ESP32 serves the page, it polls the API every 500 ms.

HTTP API the firmware must implement (keep it in sync with the page):

| Method | Path | Behaviour |
|---|---|---|
| GET | `/` | Serves the page (`kIndexHtml` from `index_html.h`) |
| GET | `/api/state` | `{"ampIp":"x.x.x.x","connected":bool,"outputs":[true|false|null ×4],"log":["..."],"logTotal":n}`. `outputs[i]`: true = unmuted, false = muted, null = not known. `log` holds the recent command lines, and `logTotal` counts every line ever logged. |
| POST | `/api/press?ch=1..4` | Same as pressing the physical button: explicit `SET nn AUDIO_MUTE ON/OFF` (or `GET` if unknown). 503 if not connected. |
| POST | `/api/ip?ip=x.x.x.x` | Sets the amp IP **in RAM only** (lost on reboot) and reconnects |

The web press follows the same rule as the buttons: the state changes only
when the amp's `REP` comes back.

## Build / flash / monitor (Arduino IDE)

1. One-time setup: in **File → Preferences → Additional boards manager URLs**,
   add `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
   Then in **Tools → Board → Boards Manager**, install **esp32 by Espressif Systems**.
2. Open `MXN_AMP_Switcher/MXN_AMP_Switcher.ino`.
3. Copy `secrets.example.h` to `secrets.h` in the same folder and fill in the Wi-Fi credentials.
4. **Tools → Board → esp32 → ESP32 Dev Module**, and select the USB port.
5. Click **Upload**.
6. Open **Tools → Serial Monitor** at **115200** baud to see the IP address and the amp log.

## Behaviour rules (keep these true)

1. **The amp is the source of truth.** On a button press, send an explicit
   `SET nn AUDIO_MUTE ON|OFF` (never `TOGGLE`) and do **not** change the LED
   until the amp sends back a `REP`. If the state is unknown, send only a `GET`.
2. On (re)connect, send `< GET 00 AUDIO_MUTE >` (or `< GET 0 ALL >`) to sync all LEDs.
3. Parse every incoming `REP` line, including ones the ESP32 did not request.
4. Keep the TCP connection open. Reconnect with backoff if it drops. While
   disconnected, every zone goes back to unknown, green LEDs go off, and all red
   LEDs blink together so the user knows the controls are dead.
5. Debounce buttons (~30–50 ms). Act on the press edge only, and never on
   hold or auto-repeat.
6. Nothing in `loop()` may block for long. Use a non-blocking socket read
   with a line buffer split on `>`.
7. Buttons are independent. Do not add an "exclusive"/radio mode unless asked.

## Conventions

- C++17, 2-space indent, `camelCase` functions, `kConstant` constants.
- Keep IPs and credentials out of git (`MXN_AMP_Switcher/secrets.h` is ignored).
- Any Shure command added to the code must also be recorded in `MXN-AMP.md`,
  and marked as checked against the official command-string page.
