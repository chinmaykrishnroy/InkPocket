# Development

## Prerequisites

- Arduino CLI 1.x or Arduino IDE 2.x
- ESP32 board package
- `GxEPD2` library

Install the board package and library with Arduino CLI:

```powershell
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli lib install GxEPD2
```

## Build

From the repository root:

```powershell
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C3:PartitionScheme=min_spiffs --build-path build-ota .\
```

The resulting firmware is `build-ota/E_Reader.ino.bin`.

## Flash over USB

```powershell
arduino-cli upload -p COM6 --fqbn esp32:esp32:XIAO_ESP32C3:PartitionScheme=min_spiffs .\
```

## Flash over web OTA

Start Wi-Fi on the device, then use its displayed IP address:

```powershell
$bin = ".\build-ota\E_Reader.ino.bin"
$size = (Get-Item -LiteralPath $bin).Length
curl.exe -F "firmware=@$bin" "http://DEVICE_IP/api/ota?size=$size"
```

## Project layout

```text
.
├── E_Reader.ino          # Application orchestration and routes
├── config/               # Hardware-specific configuration
├── data/                 # LittleFS seed files
├── assets/               # Repository artwork
├── docs/                 # Hardware and developer references
├── .github/              # Issue and pull-request templates
├── *Renderer.h           # Display/rendering modules
├── *Policy.h             # Refresh and radio policies
└── WebUi.h               # Embedded responsive web interface
```

Keep firmware modules small and focused. Do not commit `Secrets.h`, build
output, or generated binaries.
