#pragma once

#include <Arduino.h>

enum class RadioPurpose : uint8_t {
  None,
  TimeSync,
  Web,
  Setup
};

struct RadioPolicy {
  RadioPurpose purpose = RadioPurpose::None;

  static constexpr uint32_t kBackgroundJoinMs = 12000;
  static constexpr uint32_t kManualJoinMs = 20000;
  static constexpr uint32_t kNtpTimeoutMs = 12000;
  static constexpr uint32_t kWebIdleMs = 60000;
  static constexpr uint32_t kSetupIdleMs = 300000;

};
