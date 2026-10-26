#pragma once

#include <Arduino.h>

namespace Config {
// Keep disabled until the relay module, GPIO and polarity are confirmed.
constexpr bool kRelayOutputEnabled = false;
constexpr uint8_t kRelayPin = 23;
constexpr bool kRelayActiveLow = true;

constexpr uint32_t kWifiRetryMs = 15000;
constexpr uint32_t kCloudRetryMs = 10000;
constexpr uint32_t kHeartbeatMs = 5000;
constexpr uint32_t kCloudConnectBudgetMs = 1000;
}  // namespace Config
