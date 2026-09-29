# MXN-AMP: Control API Reference (Shure + Dante)

> **Status: draft, verify before relying on it.** The official Shure pages
> could not be opened from the environment where this file was written. The
> commands below follow the standard Shure command-string format that Shure
> Microflex / IntelliMix devices share. Before shipping, check every command
> against the official MXN-AMP page and tick the "Verified" column:
>
> - Command strings: <https://www.shure.com/en-US/docs/commandstrings/mxn-amp>
> - User guide: <https://www.shure.com/en-US/docs/guide/MXN-AMP>
> - Product page: <https://www.shure.com/en-US/products/loudspeakers/mxn-amp>
> - Shure IP ports list: <https://content-files.shure.com/FileRepository/common-ip-ports-v2.pdf>

## 1. The device

| Spec | Value |
|---|---|
| Model | Shure Microflex **MXN-AMP**, PoE/PoE+ powered multichannel amplifier |
| Power | PoE or PoE+ (802.3at), up to 25.5 W in; **14.5 W** combined audio out |
| Audio in | **4 × Dante** inputs, plus 1 unbalanced stereo analog input |
| Dante out | 4 × Dante outputs |
| Speaker out | 4 channels. Up to 4 MXP speakers in 8 Ω (LoZ) mode, or up to 8 MXP-5/MXP-6 in 70 V mode |
| Logic in | 1 logic input that switches between **preset 1 and preset 2** (set up in the web UI) |
| DSP | EQ, delay, limiter, signal/tone generator, MXP speaker presets |
| Config | Built-in web UI, Shure Designer |
| 3rd-party control | Shure command strings, **TCP port 2202** (ASCII) |

## 2. Shure command strings (what the ESP32 uses)

### Transport

- **TCP, port 2202**, to the amp's **control** IP address (not the Dante primary
  IP if they are on separate interfaces/VLANs).
- ASCII text, no login. Leave one connection open for the whole session.
- Every message is wrapped in `< ... >` with a space after `<` and before `>`.
  There is **no newline terminator**, so split the stream on `>`.
- Message types:
  - `GET`: ask for a value. The amp answers with `REP`.
  - `SET`: change a value. The amp answers with `REP` showing the new value.
  - `REP`: report from the amp. It is also sent **unsolicited** whenever the
    value changes from any source, which is how the LEDs stay in sync.
  - `SAMPLE`: meter data (after `METER_RATE` is set).
- Channel index is **two digits**: `01`–`04`. `00` = all channels.

### Commands relevant to this project

| Purpose | Send | Reply | Verified |
|---|---|---|---|
| Get everything | `< GET 0 ALL >` | many `REP` lines | ☐ |
| Model | `< GET MODEL >` | `< REP MODEL {MXN-AMP...} >` | ☐ |
| Device ID | `< GET DEVICE_ID >` | `< REP DEVICE_ID {name} >` | ☐ |
| Firmware | `< GET FW_VER >` | `< REP FW_VER {x.y.z} >` | ☐ |
| Identify (flash LEDs) | `< SET FLASH ON >` / `OFF` | `< REP FLASH ON >` | ☐ |
| **Channel mute state** | `< GET nn AUDIO_MUTE >` | `< REP nn AUDIO_MUTE ON\|OFF >` | ☐ |
| **Mute channel** | `< SET nn AUDIO_MUTE ON >` | `< REP nn AUDIO_MUTE ON >` | ☐ |
| **Unmute channel** | `< SET nn AUDIO_MUTE OFF >` | `< REP nn AUDIO_MUTE OFF >` | ☐ |
| Toggle channel (**not used**: the firmware sends explicit ON/OFF) | `< SET nn AUDIO_MUTE TOGGLE >` | `< REP nn AUDIO_MUTE ON\|OFF >` | ☐ |
| Device (master) mute | `< SET DEVICE_AUDIO_MUTE ON\|OFF\|TOGGLE >` | `< REP DEVICE_AUDIO_MUTE ... >` | ☐ |
| Channel gain | `< GET nn AUDIO_GAIN_HI_RES >` | `< REP nn AUDIO_GAIN_HI_RES 1100 >` | ☐ |
| Set gain | `< SET nn AUDIO_GAIN_HI_RES 0000–1400 >` | as above | ☐ |
| Nudge gain | `< SET nn AUDIO_GAIN_HI_RES INC 10 >` / `DEC 10` | as above | ☐ |
| Recall preset | `< SET PRESET n >` | `< REP PRESET n >` | ☐ |
| Current preset | `< GET PRESET >` | `< REP PRESET n >` | ☐ |
| Meter rate | `< SET METER_RATE 0\|100–99999 >` (ms, 0 = off) | `SAMPLE` stream | ☐ |

Gain scaling (Shure convention): `AUDIO_GAIN_HI_RES` runs from `0000` to
`1400` in 0.1 dB steps with an offset of 1100, so `1100` = 0 dB, `0000` =
−110 dB, and `1400` = +30 dB. Confirm the MXN-AMP range.

> **Channel numbers (confirmed on the amp):** the 4 amplifier **outputs**
> are channels **06–09** (zone 1 = 06 ... zone 4 = 09). The lower numbers are
> inputs, so never mute those from the switcher.
>
> **Still check on the amp:** (b) that `SET nn AUDIO_MUTE ON/OFF` gets a `REP`
> back every time, even when the value doesn't change (the LEDs depend on
> it), (c) whether a matrix/routing command exists
> (`MATRIX_MXR_ROUTE` / `MATRIX_MXR_GAIN` on other Shure DSPs). If there is
> one, "switch output" could mean routing a source to an output instead of
> muting it.

### Example session

```
ESP32 → < GET 00 AUDIO_MUTE >
AMP   → < REP 01 AUDIO_MUTE OFF >        # input channels: ignored
...
AMP   → < REP 06 AUDIO_MUTE OFF >
AMP   → < REP 07 AUDIO_MUTE ON >
AMP   → < REP 08 AUDIO_MUTE ON >
AMP   → < REP 09 AUDIO_MUTE OFF >        # LEDs: 1 green, 2 red, 3 red, 4 green
[user presses button 2; zone 2 was reported muted, so send an explicit unmute]
ESP32 → < SET 07 AUDIO_MUTE OFF >
AMP   → < REP 07 AUDIO_MUTE OFF >        # now zone 2 goes red -> green
```

### Quick test from a laptop (before writing firmware)

```bash
nc <amp-ip> 2202
< GET 0 ALL >
< SET 06 AUDIO_MUTE ON >
< SET 06 AUDIO_MUTE OFF >
```

## 3. Dante: what it can and cannot do here

Dante (Audinate) carries the audio **into** the MXN-AMP. Control of Dante is a
separate matter:

| Option | What it is | Fits an ESP32? |
|---|---|---|
| **Dante Controller** | Desktop app for routing (subscriptions) | No. Manual set-up only |
| **Dante Domain Manager (DDM) API** | GraphQL-over-HTTPS API (DDM 1.5+) that can query domains, devices and channels and **change channel subscriptions**. Needs a licensed DDM server, and works with devices on Dante firmware 4.0+ | Possible (HTTPS POST from the ESP32), but needs DDM infrastructure and auth tokens |
| **Dante API / Dante Connect** | Audinate SDKs for OEMs and service providers | Not for this project |
| Dante ARC protocol (UDP 4440) | Undocumented, reverse-engineered by open-source tools (e.g. `netaudio`) | Not recommended: unsupported and fragile |

**Decision:** the ESP32 **does not** change Dante routing. Dante subscriptions
are set once in Dante Controller (source → MXN-AMP inputs 1–4). Output
switching is done on the amp with Shure command strings: mute/unmute per
output channel, or preset recall. It is simpler, fast, supported, and needs
no extra servers.

If routing-based switching is ever needed, add a DDM GraphQL client module.
See the DDM API announcement:
<https://www.audinate.com/news/press-releases/audinate-releases-new-api-targeting-system-integrators-and-service-providers>
and the Audinate third-party integration page:
<https://global.audinate.com/products/dante-enabled/third-party-applications>.

## 4. Network checklist

- [ ] Amp has a static IP, or a DHCP reservation, on the control interface
- [ ] TCP 2202 is reachable from the ESP32's subnet (no firewall/ACL blocking)
- [ ] Third-party control is enabled in the MXN-AMP web UI/Designer if there is such a setting
- [ ] `nc <amp-ip> 2202` + `< GET MODEL >` answers from a laptop on the same VLAN
