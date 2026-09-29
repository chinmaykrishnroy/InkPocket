#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

namespace DeviceUi {

enum class RadioMark : uint8_t {
  Off,
  On,
  Client,
  AccessPoint
};

template <typename Display>
void font(Display& display) {
  display.setFont(nullptr);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
}

template <typename Display>
void face(Display& display, const GFXfont* typeface, uint16_t color = GxEPD_BLACK) {
  display.setFont(typeface);
  display.setTextSize(1);
  display.setTextColor(color);
}

template <typename Display>
String fitFace(Display& display, String text, const GFXfont* typeface, int width) {
  face(display, typeface);
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  if (textWidth <= width) return text;
  while (text.length() > 1) {
    text.remove(text.length() - 1);
    String candidate = text + "...";
    display.getTextBounds(candidate, 0, 0, &x1, &y1, &textWidth, &textHeight);
    if (textWidth <= width) return candidate;
  }
  return text;
}

template <typename Display>
void faceInRect(Display& display, String text, const GFXfont* typeface,
                int x, int y, int width, int height, uint16_t color = GxEPD_BLACK,
                bool center = true) {
  text = fitFace(display, text, typeface, width);
  face(display, typeface, color);
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  int cursorX = center ? x + (width - textWidth) / 2 - x1 : x - x1;
  int cursorY = y + (height - textHeight) / 2 - y1;
  display.setCursor(cursorX, cursorY);
  display.print(text);
  font(display);
}

template <typename Display>
void centered(Display& display, const String& text, int y, uint8_t size = 1) {
  font(display);
  display.setTextSize(size);
  int16_t x1, y1;
  uint16_t width, height;
  display.getTextBounds(text, 0, y, &x1, &y1, &width, &height);
  display.setCursor((200 - width) / 2, y);
  display.print(text);
}

template <typename Display>
void centeredIn(Display& display, const String& text, int x, int y, int width,
                uint8_t size = 1) {
  font(display);
  display.setTextSize(size);
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(text, 0, y, &x1, &y1, &textWidth, &textHeight);
  display.setCursor(x + (width - textWidth) / 2, y);
  display.print(text);
}

template <typename Display>
void statusPill(Display& display, int x, int y, int width, const String& label,
                bool filled = false) {
  if (filled) display.fillRoundRect(x, y, width, 19, 9, GxEPD_BLACK);
  else display.drawRoundRect(x, y, width, 19, 9, GxEPD_BLACK);
  faceInRect(display, label, &FreeSansBold9pt7b, x + 4, y, width - 8, 19,
             filled ? GxEPD_WHITE : GxEPD_BLACK);
}

template <typename Display>
void drawRadioMark(Display& display, RadioMark mark, int rssi) {
  if (mark == RadioMark::On || mark == RadioMark::AccessPoint) {
    const char* label = mark == RadioMark::On ? "ON" : "AP";
    display.drawRoundRect(170, 4, 25, 15, 7, GxEPD_WHITE);
    font(display);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(177, 8);
    display.print(label);
  } else if (mark == RadioMark::Client) {
    int bars = rssi >= -55 ? 4 : rssi >= -65 ? 3 : rssi >= -75 ? 2 : 1;
    for (int i = 0; i < bars; i++) {
      int barHeight = 4 + i * 3;
      display.fillRoundRect(178 + i * 4, 18 - barHeight, 3, barHeight, 1,
                            GxEPD_WHITE);
    }
  }
  font(display);
}

template <typename Display>
void header(Display& display, String title, RadioMark mark, int rssi) {
  display.fillRect(0, 0, 200, 23, GxEPD_BLACK);
  int titleWidth = mark == RadioMark::Off ? 184 : 158;
  faceInRect(display, title, &FreeSansBold9pt7b, 8, 0, titleWidth, 23,
             GxEPD_WHITE, false);
  drawRadioMark(display, mark, rssi);
}

template <typename Display>
void homeHeader(Display& display, const String& appName, const String& time,
                RadioMark mark, int rssi) {
  display.fillRect(0, 0, 200, 23, GxEPD_BLACK);
  int rightEdge = mark == RadioMark::Off ? 195 : 164;
  faceInRect(display, appName, &FreeSansBold9pt7b, 8, 0, 95, 23,
             GxEPD_WHITE, false);
  faceInRect(display, time, &FreeSansBold9pt7b, 108, 0, rightEdge - 108, 23,
             GxEPD_WHITE, false);
  drawRadioMark(display, mark, rssi);
}

template <typename Display>
void appIcon(Display& display, int app, int x, int y, uint16_t color) {
  const int centerX = x + 18;
  const int centerY = y + 18;
  uint16_t ink = color == GxEPD_BLACK ? GxEPD_WHITE : GxEPD_BLACK;
  display.fillCircle(centerX, centerY, 18, color);
  if (app == 0) {
    display.drawRoundRect(centerX - 12, centerY - 9, 24, 18, 2, ink);
    display.drawLine(centerX, centerY - 8, centerX, centerY + 9, ink);
    for (int row = -4; row <= 0; row += 4) {
      display.drawLine(centerX - 9, centerY + row, centerX - 3, centerY + row, ink);
      display.drawLine(centerX + 3, centerY + row, centerX + 9, centerY + row, ink);
    }
  } else if (app == 1) {
    display.drawCircle(centerX, centerY, 11, ink);
    display.drawLine(centerX, centerY, centerX, centerY - 7, ink);
    display.drawLine(centerX, centerY, centerX + 6, centerY + 3, ink);
    display.fillCircle(centerX, centerY, 2, ink);
  } else if (app == 2) {
    display.drawLine(centerX - 11, centerY, centerX - 5, centerY - 6, ink);
    display.drawLine(centerX - 11, centerY, centerX - 5, centerY + 6, ink);
    display.drawLine(centerX + 11, centerY, centerX + 5, centerY - 6, ink);
    display.drawLine(centerX + 11, centerY, centerX + 5, centerY + 6, ink);
    display.drawLine(centerX - 2, centerY + 7, centerX + 3, centerY - 7, ink);
  } else {
    display.drawCircle(centerX, centerY, 9, ink);
    display.drawCircle(centerX, centerY, 3, ink);
    for (int direction = -1; direction <= 1; direction += 2) {
      display.drawLine(centerX + direction * 9, centerY, centerX + direction * 14, centerY, ink);
      display.drawLine(centerX, centerY + direction * 9, centerX, centerY + direction * 14, ink);
      display.drawLine(centerX + direction * 7, centerY + direction * 7,
                       centerX + direction * 10, centerY + direction * 10, ink);
      display.drawLine(centerX + direction * 7, centerY - direction * 7,
                       centerX + direction * 10, centerY - direction * 10, ink);
    }
  }
}

template <typename Display>
void tile(Display& display, int app, int x, int y, const String& label, bool selected) {
  uint16_t color = selected ? GxEPD_WHITE : GxEPD_BLACK;
  if (selected) display.fillRoundRect(x, y, 90, 76, 8, GxEPD_BLACK);
  else display.drawRoundRect(x, y, 90, 76, 8, GxEPD_BLACK);
  appIcon(display, app, x + 27, y + 8, color);
  font(display);
  display.setTextColor(color);
  int16_t x1, y1;
  uint16_t width, height;
  display.getTextBounds(label, 0, 0, &x1, &y1, &width, &height);
  display.setCursor(x + (90 - width) / 2, y + 61);
  display.print(label);
  font(display);
}

template <typename Display>
void footer(Display& display, const String& text) {
  display.drawLine(10, 181, 190, 181, GxEPD_BLACK);
  font(display);
  int16_t x1, y1;
  uint16_t width, height;
  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);
  display.setCursor((200 - width) / 2, 188);
  display.print(text);
}

template <typename Display>
void toast(Display& display, String text) {
  text = fitFace(display, text, &FreeSansBold9pt7b, 166);
  face(display, &FreeSansBold9pt7b);
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  int width = min(184, (int)textWidth + 18);
  int x = (200 - width) / 2;
  display.fillRoundRect(x, 153, width, 28, 8, GxEPD_BLACK);
  faceInRect(display, text, &FreeSansBold9pt7b, x + 7, 153,
             width - 14, 28, GxEPD_WHITE);
}

template <typename Display>
void infoCard(Display& display, int y, const String& label, const String& value,
              const GFXfont* valueFont = &FreeSansBold9pt7b) {
  display.drawRoundRect(8, y, 184, 42, 8, GxEPD_BLACK);
  font(display);
  display.setCursor(16, y + 7);
  display.print(label);
  faceInRect(display, value, valueFont, 15, y + 13, 170, 25,
             GxEPD_BLACK, false);
}

template <typename Display>
void clockApp(Display& display, bool focused, bool valid,
              const String& hour, const String& minute, const String& meridiem,
              const String& day, const String& date, const String& month,
              const String& year, const String& fullDate, uint8_t unfocusSeconds,
              RadioMark mark, int rssi) {
  if (focused) {
    header(display, "Clock", mark, rssi);
    if (!valid) {
      faceInRect(display, "--:--", &FreeMonoBold24pt7b, 4, 55, 192, 54);
      centered(display, "Time not synced", 132);
      footer(display, "Hold: home");
      return;
    }
    faceInRect(display, hour + ":" + minute, &FreeMonoBold24pt7b,
               4, 43, 192, 58);
    if (meridiem.length()) statusPill(display, 80, 104, 40, meridiem, true);
    faceInRect(display, fullDate, &FreeSansBold9pt7b, 8, 123, 184, 25);
    centered(display, "Ambient after " + String(unfocusSeconds) + " seconds", 159);
    footer(display, "Hold: home");
    return;
  }

  if (!valid) {
    faceInRect(display, "--:--", &FreeMonoBold24pt7b, 4, 64, 192, 58);
    centered(display, "Time not synced", 140);
    return;
  }

  display.drawLine(99, 13, 99, 187, GxEPD_BLACK);
  faceInRect(display, hour, &FreeMonoBold24pt7b, 2, 35, 94, 51);
  display.fillCircle(39, 101, 2, GxEPD_BLACK);
  display.fillCircle(57, 101, 2, GxEPD_BLACK);
  faceInRect(display, minute, &FreeMonoBold24pt7b, 2, 116, 94, 51);
  faceInRect(display, day, &FreeSansBold9pt7b, 104, 23, 92, 24);
  faceInRect(display, date, &FreeMonoBold24pt7b, 104, 53, 92, 52);
  faceInRect(display, month, &FreeSansBold9pt7b, 104, 110, 92, 25);
  faceInRect(display, year, &FreeMonoBold9pt7b, 104, 139, 92, 25);
  statusPill(display, 126, 168, 48, meridiem.length() ? meridiem : "24H");
}

template <typename Display>
void wifiApp(Display& display, RadioMark mark, const String& ssid,
             const String& ip, const String& password, int rssi) {
  header(display, "Wi-Fi", mark, rssi);

  if (mark == RadioMark::On) {
    statusPill(display, 55, 31, 90, "RADIO ON");
    faceInRect(display, "Connecting", &FreeSansBold9pt7b, 8, 55, 184, 24);
    infoCard(display, 85, "NETWORK", ssid);
    font(display);
    centered(display, "Looking for the saved network", 148);
  } else if (mark == RadioMark::Client) {
    statusPill(display, 58, 31, 84, "CLIENT", true);
    faceInRect(display, "Connected", &FreeSansBold9pt7b, 8, 54, 184, 24);
    infoCard(display, 81, "NETWORK", ssid);
    infoCard(display, 127, "ADDRESS", ip, &FreeMonoBold9pt7b);
    font(display);
    String signal = "Signal " + String(rssi) + " dBm";
    int16_t x1, y1;
    uint16_t width, height;
    display.getTextBounds(signal, 0, 0, &x1, &y1, &width, &height);
    display.setCursor((200 - width) / 2, 171);
    display.print(signal);
  } else if (mark == RadioMark::AccessPoint) {
    statusPill(display, 48, 29, 104, "SETUP AP", true);
    faceInRect(display, "Join from phone", &FreeSansBold9pt7b, 8, 51, 184, 23);
    infoCard(display, 77, "NETWORK", ssid);
    infoCard(display, 122, "PASSWORD", password, &FreeMonoBold9pt7b);
    font(display);
    String address = "Open " + ip;
    int16_t x1, y1;
    uint16_t width, height;
    display.getTextBounds(address, 0, 0, &x1, &y1, &width, &height);
    display.setCursor((200 - width) / 2, 171);
    display.print(address);
  } else {
    statusPill(display, 68, 42, 64, "OFF");
    faceInRect(display, "Wi-Fi is off", &FreeSansBold9pt7b, 8, 76, 184, 30);
    centered(display, "Hold Home for 5 seconds", 128);
    centered(display, "to start the radio", 144);
  }

  footer(display, "Hold: back");
}

}  // namespace DeviceUi

