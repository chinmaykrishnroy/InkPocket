#pragma once

#include <Arduino.h>

enum class AppView : uint8_t {
  Home,
  ReaderList,
  ReaderDocument,
  Clock,
  Api,
  Settings,
  Wifi
};

enum class WifiState : uint8_t {
  Off,
  Joining,
  Connected,
  CaptivePortal
};

enum class RemoteContent : uint8_t {
  Empty,
  Text,
  Markdown,
  Bitmap
};

// Short aliases keep the event-state code readable while making the actual
// state vocabulary live in this one dedicated module.
using View = AppView;
using Radio = WifiState;
using Remote = RemoteContent;

constexpr View HOME = View::Home;
constexpr View LIST = View::ReaderList;
constexpr View BOOK = View::ReaderDocument;
constexpr View CLOCK = View::Clock;
constexpr View API = View::Api;
constexpr View SETTINGS = View::Settings;
constexpr View WIFI = View::Wifi;

constexpr Radio OFF = Radio::Off;
constexpr Radio JOINING = Radio::Joining;
constexpr Radio ONLINE = Radio::Connected;
constexpr Radio PORTAL = Radio::CaptivePortal;

constexpr Remote NONE = Remote::Empty;
constexpr Remote TEXT = Remote::Text;
constexpr Remote MARKDOWN = Remote::Markdown;
constexpr Remote BITMAP = Remote::Bitmap;

