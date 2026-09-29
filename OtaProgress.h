#pragma once

#include <Arduino.h>

struct OtaProgressState {
  bool active = false;
  bool screenReady = false;
  bool ok = false;
  bool verified = false;
  uint8_t shownStep = 255;
  uint32_t written = 0;
  uint32_t total = 0;
  uint32_t lastDraw = 0;
  uint32_t lastActivity = 0;
  String message;

  void begin(uint32_t expectedBytes) {
    active = true;
    screenReady = false;
    ok = false;
    verified = false;
    shownStep = 255;
    written = 0;
    total = expectedBytes;
    lastDraw = 0;
    lastActivity = millis();
    message = "Preparing firmware";
  }

  uint8_t percent() const {
    if (!total) return 0;
    return min(100UL, (uint32_t)(((uint64_t)written * 100UL) / total));
  }

  uint8_t displayStep() const {
    return total ? percent() / 5 : min(255UL, written / 16384UL);
  }
};

