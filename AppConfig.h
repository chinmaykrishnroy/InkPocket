#pragma once

#include <Arduino.h>
#include "config/HardwareConfig.h"

struct DeviceSettings {
  uint8_t unfocus = 10;
  uint16_t saverSec = 30;
  uint8_t fullAfter = 10;
  uint8_t scrollSec = 10;
  uint8_t saverClockMin = 5;
  uint16_t hold = 750;
  bool twelve = false;
  bool saver = true;
  int16_t utc = 330;
};

