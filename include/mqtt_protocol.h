#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

// Pure protocol checks, independent of Arduino and network state.
namespace MqttProtocol {
inline bool parseRelayCommand(const uint8_t* payload, size_t size, bool& on) {
  if (!payload || size != 1 || (payload[0] != '0' && payload[0] != '1')) {
    return false;
  }
  on = payload[0] == '1';
  return true;
}

inline bool isAllowedHost(const char* host) {
  if (!host) return false;
  static const char* const allowed[] = {
      "blynk.cloud", "fra1.blynk.cloud", "lon1.blynk.cloud",
      "ny3.blynk.cloud", "sgp1.blynk.cloud", "blr1.blynk.cloud"};
  for (const char* entry : allowed) {
    if (std::strcmp(host, entry) == 0) return true;
  }
  return false;
}

inline bool parseRedirect(const uint8_t* payload, size_t size,
                          char* host, size_t hostCapacity) {
  constexpr char prefix[] = "mqtts://";
  constexpr size_t prefixLength = sizeof(prefix) - 1;
  if (!payload || !host || hostCapacity == 0 ||
      size <= prefixLength || size > 95 ||
      std::memcmp(payload, prefix, prefixLength) != 0) {
    return false;
  }
  // Reject embedded NULs instead of accidentally trusting only a URI prefix.
  for (size_t i = 0; i < size; ++i) {
    if (payload[i] == 0) return false;
  }
  size_t end = size;
  if (payload[end - 1] == '/') --end;
  size_t hostEnd = prefixLength;
  while (hostEnd < end && payload[hostEnd] != ':') ++hostEnd;
  if (hostEnd < end &&
      (end - hostEnd != 5 || std::memcmp(payload + hostEnd, ":8883", 5) != 0)) {
    return false;
  }
  const size_t hostLength = hostEnd - prefixLength;
  if (hostLength == 0 || hostLength >= hostCapacity) return false;
  char candidate[64] = {};
  if (hostLength >= sizeof(candidate)) return false;
  std::memcpy(candidate, payload + prefixLength, hostLength);
  if (!isAllowedHost(candidate)) return false;
  std::memcpy(host, candidate, hostLength + 1);
  return true;
}

inline bool validTemplateId(const char* id) {
  if (!id || std::strncmp(id, "TMPL", 4) != 0 ||
      std::strcmp(id, "TMPL_REPLACE_ME") == 0) return false;
  const size_t size = std::strlen(id);
  if (size <= 4 || size > 64) return false;
  for (size_t i = 4; i < size; ++i) {
    const char ch = id[i];
    if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
          (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')) return false;
  }
  return true;
}
}  // namespace MqttProtocol
