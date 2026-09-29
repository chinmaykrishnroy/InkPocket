# InkPocket

<p align="center">
  <img src="assets/inkpocket-mark.svg" alt="InkPocket logo" width="132">
</p>

<p align="center">
  A one-button, Wi-Fi capable pocket reader for a 200×200 monochrome e-paper display.
</p>

InkPocket runs on a Seeed XIAO ESP32-C3 with a 1.54-inch D67 display. It is a
small offline-first reader with a clock, ambient display, screen saver,
LittleFS library, responsive browser control, remote display API, and safe OTA
updates.

## Features

- Read local Markdown and plain-text files from LittleFS.
- Focused and ambient clock modes, with hourly background NTP correction.
- Screen saver and selective partial refreshes to limit e-paper ghosting.
- One button: click, double-click, triple-click, and hold controls.
- Wi-Fi station mode, captive setup AP, inactivity radio shutdown, and OTA.
- Responsive web UI to manage files, Wi-Fi, device apps, time, and firmware.
- Remote API for text, Markdown, and monochrome BMP content.
- Staged file writes, rollback recovery, and a 256 KiB text/API safety limit.

## Build one

See the [shopping list](docs/SHOPPING.md) for the exact board, display, and
battery links, and the [hardware reference](docs/HARDWARE.md) for wiring.

| Part | Required |
| --- | --- |
| Seeed XIAO ESP32-C3 | Yes |
| 1.54-inch WeAct/D67 SPI e-paper panel (200×200, black/white) | Yes |
| Single-cell 3.7 V LiPo battery | Optional, for portable use |

Hardware-profile pins and safety limits are centralized in
[`config/HardwareConfig.h`](config/HardwareConfig.h). The UI remains designed for
the current 200×200 panel. Copy
[`Secrets.example.h`](Secrets.example.h) to `Secrets.h` and add optional
default station credentials and a unique `kApiKey` (at least 8 characters).
`Secrets.h` is ignored by Git.

## Wiring

| Display signal | XIAO GPIO |
| --- | ---: |
| SCK | 8 |
| MOSI | 10 |
| CS | 7 |
| DC | 21 |
| RST | 20 |
| BUSY | 6 |
| BOOT / action button | 9 |

The action button is active-low. GPIO 9 is suitable for the project’s
light-sleep workflow but not ESP32-C3 RTC deep-sleep wake; see
[hardware notes](docs/HARDWARE.md).

## Quick start

1. Install Arduino IDE 2.x or Arduino CLI, the ESP32 board package, and the
   `GxEPD2` library.
2. Clone this repository and copy `Secrets.example.h` to `Secrets.h`.
3. Build using the default partition scheme (large enough for the documented LittleFS payload limits):

   ```powershell
   arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3:PartitionScheme=default --build-path build-ota .\
   ```

4. Upload over USB:

   ```powershell
   arduino-cli upload -p COM6 --fqbn esp32:esp32:XIAO_ESP32C3:PartitionScheme=default .\
   ```

Full development, USB, and OTA commands are in
[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

## Controls

| Input | Action |
| --- | --- |
| Click | Focus an inactive screen, then move/scroll forward |
| Double click | Open/confirm, or scroll backward in a document |
| Triple click | Toggle auto-scroll in Reader and Remote Text modes |
| Hold | Go back (or focus an unfocused non-Home app) |
| Hold Home for 5 seconds | Start Wi-Fi |

Home has an independent 15/30/60/120-second screen-saver timer (default:
30 seconds). Reader, API, and Settings light-sleep after 30 seconds without
showing the screen saver. Clock instead changes into a full-screen ambient
clock after its unfocus timeout and updates once a minute. The BOOT button
wakes to Home; it wakes ambient Clock back into focused Clock.

## Web control and Wi-Fi

Hold Home for five seconds, then visit the IP shown on the device. The web UI
can upload and manage `.md`, `.txt`, and `.bmp` files; configure Wi-Fi; switch
apps; sync time; and upload OTA firmware with live browser and e-paper progress.

If saved Wi-Fi cannot connect, InkPocket opens `InkPocket-Setup` at
`192.168.4.1`. A fresh eight-digit AP password is displayed on the device each
session. Station web mode powers the radio down after 60 seconds of real
inactivity; passive status polls do not keep it awake. The setup AP remains for
five minutes.

Once its clock is valid, InkPocket attempts a fresh NTP sync at every local
`HH:00`, briefly reconnecting in the background and disconnecting immediately
after success or timeout.

## Content limits

- Markdown, plain text, and Remote Display API bodies: **256 KiB maximum**.
- BMP: uncompressed 1-bit BMP, **7 KiB maximum**; keep at most five BMPs.
- `/EReader.md` and `/test.bmp` seed the library on a successful LittleFS mount.
- LittleFS is never auto-formatted when mounting fails.

## Remote Display API

Wi-Fi must be active. All endpoints use the device IP, for example
`http://DEVICE_IP`.

| Method | Route | Purpose |
| --- | --- | --- |
| `GET` | `/api/v1/status` | Device, Wi-Fi, time, heap, and NTP diagnostics |
| `POST` | `/api/v1/app?name=home\|reader\|clock\|api\|settings` | Switch app |
| `POST` | `/api/v1/time/sync` | Trigger NTP sync |
| `POST` | `/api/v1/screen/clear` | Clear Remote Display content |
| `POST` | `/api/v1/screen/text` | Show plain text |
| `POST` | `/api/v1/screen/markdown` | Show Markdown |
| `POST` | `/api/v1/screen/bmp` | Show an uncompressed 1-bit BMP |
| `POST` | `/api/ota?size=BYTES` | Install a firmware binary |

Raw text requests can use positioning headers. Markdown is rendered with the normal
full-screen reader layout. Normal-LAN API requests also require `X-InkPocket-Key`:

```powershell
$message = @'
Hello from InkPocket!

This is plain text with real line breaks.
'@

$message | curl.exe -X POST -H "Content-Type: text/plain" `
  -H "X-InkPocket-X: 12" -H "X-InkPocket-Y: 36" `
  -H "X-InkPocket-W: 176" -H "X-InkPocket-H: 128" `
  -H "X-InkPocket-Key: YOUR_ACCESS_KEY" `
  --data-binary "@-" "http://DEVICE_IP/api/v1/screen/text"
```

Send Markdown to `/api/v1/screen/markdown`. It supports headings, emphasis,
bullets, numbered/task lists, block quotes, rules, inline code, fenced code,
strikethrough, and readable link text. For a BMP use multipart upload. `x` and
`y` position its destination; `w` and `h` scale the complete image:

```powershell
curl.exe -X POST -F "file=@image.bmp" `
  "http://DEVICE_IP/api/v1/screen/bmp?x=50&y=50&w=100&h=100"
```

## OTA

Build a binary, start Wi-Fi on InkPocket, then run:

```powershell
$bin = ".\build-ota\InkPocket.ino.bin"
$size = (Get-Item -LiteralPath $bin).Length
curl.exe -F "firmware=@$bin" "http://DEVICE_IP/api/ota?size=$size"
```

The device performs a staged write and verification, displays live progress,
then restarts only after a verified update.

## Repository layout

```text
.
├── E_Reader.ino          # Application orchestration, controls, sleep, HTTP
├── config/               # Board and display configuration
├── data/                 # Seed files for LittleFS
├── assets/               # Project SVG branding
├── docs/                 # Build, hardware, and shopping guides
├── .github/              # Community templates
├── DeviceUi.h            # E-paper UI components
├── MarkdownParser.h      # Streaming Markdown layout
├── BmpRenderer.h         # Validated and scalable monochrome BMP renderer
├── RadioPolicy.h         # Wi-Fi timing policy
├── RefreshPolicy.h       # Partial/full refresh policy
├── StagedFileWriter.h    # Safe staged LittleFS writes
└── WebUi.h               # Embedded responsive browser UI
```

## Contributing

Please keep modules focused, document user-visible behavior and API changes,
compile for the XIAO ESP32-C3 before opening a pull request, and never commit
credentials, build output, or firmware binaries. See
[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

## Security / access key

Normal LAN API control, HTTP OTA, and ArduinoOTA require `kApiKey` from
`Secrets.h`. The randomly-passworded setup AP is allowed to configure Wi-Fi
without the API key so a new device can be provisioned. The web UI stores the
key only for the current browser tab (`sessionStorage`).

The default partition scheme is used so the documented text/API payload limit can
fit in LittleFS. Internal remote-display scratch files are hidden from the user
library and are cleaned when the remote content type changes.
