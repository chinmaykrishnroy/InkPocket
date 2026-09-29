#pragma once

// Copy this file to Secrets.h and replace the placeholder values.
// Secrets.h is ignored by Git.
constexpr char kDefaultWifiSsid[] = "YOUR_WIFI_SSID";
constexpr char kDefaultWifiPassword[] = "YOUR_WIFI_PASSWORD";

// Required for control/OTA over the normal LAN. Use at least 8 characters.
// The temporary setup AP remains usable without this key because its WPA2
// password is randomly generated and shown on the device.
constexpr char kApiKey[] = "CHANGE_ME";
