#pragma once

#include <cstdint>

// May also be defined in secrets.h. blynk.cloud supports validated redirects.
#ifndef BLYNK_MQTT_HOST
#define BLYNK_MQTT_HOST "blynk.cloud"
#endif

namespace MqttConfig {
constexpr uint16_t kPort = 8883;
constexpr uint16_t kKeepAliveSeconds = 45;
constexpr uint16_t kSocketTimeoutSeconds = 3;
constexpr uint16_t kBufferSize = 1024;
constexpr char kCommandTopic[] = "downlink/ds/Relay Command";
constexpr char kCommandReportTopic[] = "ds/Relay Command";
constexpr char kStateTopic[] = "ds/Commanded Output";
constexpr char kSubscribeTopic[] = "downlink/#";
}  // namespace MqttConfig
