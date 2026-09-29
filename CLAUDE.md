# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project

An ESP32 wall/desk controller that switches the outputs of a **Shure MXN-AMP**
(PoE+ powered 4-channel Dante amplifier) using **4 push buttons** with
**4 LEDs** for latching feedback.

- Each button is paired with one amplifier output channel (Button 1 → Ch 1, ...).
- Pressing a button **toggles** (latches) that output on/off.
- Each LED shows the **actual state reported by the amp**, not just the last
  button press. LED on = output active (unmuted), LED off = output muted.
- If the amp changes state from elsewhere (Designer, web UI, another
  controller), the LEDs follow it.

Control is done over the network with **Shure third-party command strings**
(ASCII over TCP port 2202). Dante is used only for audio transport between
the source and the amp. The ESP32 does not touch Dante routing. See
`MXN-AMP.md` for the API reference and the reasons for this choice.

## Hardware (proposed, confirm before wiring)

| Item | Notes |
|---|---|
| MCU | ESP32 with wired Ethernet preferred (e.g. Olimex ESP32-POE, WT32-ETH01). Wi-Fi ESP32 DevKit works if the AV VLAN is reachable over Wi-Fi. |
| Buttons | 4 momentary push buttons, wired to GND, using the internal pull-up (`INPUT_PULLUP`), active LOW. |
| LEDs | 4 LEDs, each with a series resistor (~220–470 Ω), driven HIGH = on. Illuminated buttons can share the pin plan. |
| Network | Same subnet/VLAN as the MXN-AMP **control** interface. |

Default pin map (change in `include/config.h` once it exists; avoid strapping
pins 0, 2, 12, 15 and the Ethernet RMII pins on Ethernet boards):

| Channel | Button GPIO | LED GPIO |
|---|---|---|
| 1 | 32 | 4 |
| 2 | 33 | 13 |
| 3 | 34* | 14 |
| 4 | 35* | 15 |

\* GPIO 34–39 are input-only and have **no internal pull-up**, so use an
external 10 kΩ pull-up or choose different pins. Recheck this table against
the board you actually pick.

## Software stack

- **PlatformIO** + **Arduino framework** for ESP32 (`platformio.ini` at repo root).
- Networking: `ETH.h` (wired) or `WiFi.h`, with `WiFiClient` for the TCP socket to the amp.
- No heavy dependencies. Debounce and the parsing of command strings are done
  in plain code in this project.

Planned layout:

```
platformio.ini
include/config.h        # pins, amp IP, port, network creds (secrets NOT committed)
include/secrets.h       # git-ignored; copy from secrets.example.h
src/main.cpp            # setup/loop, wiring the modules together
src/shure_client.*      # TCP connection, send/parse Shure command strings, reconnect
src/buttons.*           # debounced edge detection for 4 buttons
src/leds.*              # LED output + "no connection" blink pattern
src/web.*               # HTTP server: serves web/index.html + /api/* endpoints
web/index.html          # simple control page (also runs standalone in simulation mode)
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
| GET | `/` | Serves `web/index.html` (embedded in flash) |
| GET | `/api/state` | `{"ampIp":"x.x.x.x","connected":bool,"outputs":[bool×4],"log":["..."],"logTotal":n}`. `outputs[i]` is true when the channel is unmuted. `log` holds the recent command lines, and `logTotal` counts every line ever logged. |
| POST | `/api/press?ch=1..4` | Same as pressing the physical button: sends `SET 0n AUDIO_MUTE TOGGLE` |
| POST | `/api/ip?ip=x.x.x.x` | Sets the amp IP **in RAM only** (lost on reboot) and reconnects |

The web press follows the same rule as the buttons: the state changes only
when the amp's `REP` comes back.

## Build / flash / monitor

```bash
pio run                      # build
pio run -t upload            # flash
pio device monitor -b 115200 # serial log
```

## Behaviour rules (keep these true)

1. **The amp is the source of truth.** On a button press, send `SET ... TOGGLE`
   (or ON/OFF) and do **not** change the LED until the amp sends back a `REP`.
2. On (re)connect, send `< GET 00 AUDIO_MUTE >` (or `< GET 0 ALL >`) to sync all LEDs.
3. Parse every incoming `REP` line, including ones the ESP32 did not request.
4. Keep the TCP connection open. Reconnect with backoff if it drops. While
   disconnected, all LEDs slow-blink so the user knows the controls are dead.
5. Debounce buttons (~30–50 ms). Act on the press edge only, and never on
   hold or auto-repeat.
6. Nothing in `loop()` may block for long. Use a non-blocking socket read
   with a line buffer split on `>`.
7. Optional "exclusive" mode (radio-button: only one output on at a time) is
   a compile-time flag in `config.h`. The default is independent latching.

## Conventions

- C++17, 2-space indent, `camelCase` functions, `kConstant` constants.
- Keep IPs and credentials out of git (`include/secrets.h` is ignored).
- Any Shure command added to the code must also be recorded in `MXN-AMP.md`,
  and marked as checked against the official command-string page.
