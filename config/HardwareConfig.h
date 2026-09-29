#pragma once

#include <Arduino.h>

// InkPocket hardware profile: Seeed XIAO ESP32-C3 + GxEPD2 D67 200x200 panel.
// GxEPD2 constructor order is CS, DC, RST, BUSY.
namespace InkPocket::Hardware {
constexpr uint8_t kButtonPin = 9;       // Active-low BOOT button.
constexpr uint8_t kDisplaySck = 8;
constexpr uint8_t kDisplayMosi = 10;
constexpr uint8_t kDisplayCs = 7;
constexpr uint8_t kDisplayDc = 21;
constexpr uint8_t kDisplayReset = 20;
constexpr uint8_t kDisplayBusy = 6;

constexpr uint16_t kDisplayWidth = 200;
constexpr uint16_t kDisplayHeight = 200;
constexpr uint32_t kMaxApiPayload = 256UL * 1024UL;
constexpr uint32_t kMaxBitmapBytes = 7UL * 1024UL;
constexpr uint8_t kMaxBitmapFiles = 5;
}  // namespace InkPocket::Hardware

// Short names retain compatibility with the existing firmware modules.
using InkPocket::Hardware::kButtonPin;
using InkPocket::Hardware::kDisplaySck;
using InkPocket::Hardware::kDisplayMosi;
using InkPocket::Hardware::kDisplayCs;
using InkPocket::Hardware::kDisplayDc;
using InkPocket::Hardware::kDisplayReset;
using InkPocket::Hardware::kDisplayBusy;
using InkPocket::Hardware::kMaxApiPayload;
using InkPocket::Hardware::kMaxBitmapBytes;
using InkPocket::Hardware::kMaxBitmapFiles;
