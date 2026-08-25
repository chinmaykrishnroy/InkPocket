#pragma once

#include <Arduino.h>

struct RefreshPolicy {
  bool regional = false;
  bool automatic = false;
  int16_t x = 0;
  int16_t y = 0;
  int16_t right = 200;
  int16_t bottom = 200;

  void all() {
    regional = false;
    automatic = false;
    x = 0;
    y = 0;
    right = 200;
    bottom = 200;
  }

  void region(int16_t left, int16_t top, int16_t width, int16_t height,
              bool isAutomatic = false) {
    left = constrain(left, 0, 199);
    top = constrain(top, 0, 199);
    int16_t nextRight = constrain(left + width, left + 1, 200);
    int16_t nextBottom = constrain(top + height, top + 1, 200);
    if (!regional) {
      regional = true;
      x = left;
      y = top;
      right = nextRight;
      bottom = nextBottom;
    } else {
      x = min(x, left);
      y = min(y, top);
      right = max(right, nextRight);
      bottom = max(bottom, nextBottom);
    }
    automatic = automatic || isAutomatic;
  }

  int16_t width() const { return right - x; }
  int16_t height() const { return bottom - y; }
};

