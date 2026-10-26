#include <cstdio>
#include <cstring>

#include "mqtt_protocol.h"

namespace {
int checks = 0;
int failures = 0;

void expect(bool condition, const char* label) {
  ++checks;
  if (!condition) {
    ++failures;
    std::fprintf(stderr, "FAIL: %s\n", label);
  }
}

const uint8_t* bytes(const char* text) {
  return reinterpret_cast<const uint8_t*>(text);
}

void testCommands() {
  bool on = true;
  expect(MqttProtocol::parseRelayCommand(bytes("0"), 1, on) && !on,
         "0 means OFF");
  expect(MqttProtocol::parseRelayCommand(bytes("1"), 1, on) && on,
         "1 means ON");
  const char* const invalid[] = {"", "2", "-1", "ON", "true", "01", "1\n", " 1"};
  for (const char* input : invalid) {
    on = true;
    expect(!MqttProtocol::parseRelayCommand(bytes(input), std::strlen(input), on),
           "invalid command rejected");
    expect(on, "rejected command leaves output unchanged");
  }
  const uint8_t embeddedNull[] = {'1', 0};
  expect(!MqttProtocol::parseRelayCommand(embeddedNull, sizeof(embeddedNull), on),
         "command cannot contain hidden trailing NUL");
  expect(!MqttProtocol::parseRelayCommand(nullptr, 1, on), "null command rejected");
  expect(!MqttProtocol::parseRelayCommand(bytes("1"), 1024, on),
         "oversize command rejected before accessing payload");
}

void testHosts() {
  const char* const allowed[] = {"blynk.cloud", "fra1.blynk.cloud", "lon1.blynk.cloud",
                               "ny3.blynk.cloud", "sgp1.blynk.cloud", "blr1.blynk.cloud"};
  for (const char* host : allowed) expect(MqttProtocol::isAllowedHost(host), host);
  const char* const invalid[] = {"", "BLYNK.CLOUD", "127.0.0.1", "evil.example",
      "blynk.cloud.evil.example", "evбlynk.cloud", "blynk.cloud.", "https://blynk.cloud"};
  for (const char* host : invalid) expect(!MqttProtocol::isAllowedHost(host), "spoofed host rejected");
  expect(!MqttProtocol::isAllowedHost(nullptr), "null host rejected");
}

void testRedirects() {
  struct ValidRedirect { const char* uri; const char* expectedHost; };
  const ValidRedirect valid[] = {
      {"mqtts://blynk.cloud", "blynk.cloud"},
      {"mqtts://fra1.blynk.cloud:8883", "fra1.blynk.cloud"},
      {"mqtts://lon1.blynk.cloud:8883/", "lon1.blynk.cloud"},
      {"mqtts://ny3.blynk.cloud", "ny3.blynk.cloud"},
      {"mqtts://sgp1.blynk.cloud/", "sgp1.blynk.cloud"},
      {"mqtts://blr1.blynk.cloud:8883", "blr1.blynk.cloud"}};
  char host[64] = {};
  for (const ValidRedirect& entry : valid) {
    expect(MqttProtocol::parseRedirect(bytes(entry.uri), std::strlen(entry.uri), host, sizeof(host)),
           "valid TLS redirect accepted");
    expect(std::strcmp(host, entry.expectedHost) == 0, "correct regional host extracted");
  }
  const char* const invalid[] = {
      "", "mqtts://", "mqtt://blynk.cloud:1883", "wss://blynk.cloud:443/mqtt",
      "https://blynk.cloud", "MQTTS://blynk.cloud", "mqtts://blynk.cloud:1883",
      "mqtts://blynk.cloud:443", "mqtts://blynk.cloud:88830", "mqtts://:8883",
      "mqtts://user:token@blynk.cloud:8883", "mqtts://blynk.cloud@evil.example:8883",
      "mqtts://blynk.cloud.evil.example:8883", "mqtts://blynk.cloud:8883/extra",
      "mqtts://blynk.cloud?host=evil", "mqtts://blynk.cloud#evil",
      "mqtts://blynk.cloud//", "mqtts://blynk.cloud:8883\n"};
  for (const char* uri : invalid) {
    std::strcpy(host, "unchanged");
    expect(!MqttProtocol::parseRedirect(bytes(uri), std::strlen(uri), host, sizeof(host)),
           "unsafe or malformed redirect rejected");
    expect(std::strcmp(host, "unchanged") == 0, "rejected redirect leaves output unchanged");
  }
  const char embeddedNull[] = "mqtts://blynk.cloud\0.evil.example:8883";
  expect(!MqttProtocol::parseRedirect(bytes(embeddedNull), sizeof(embeddedNull) - 1,
                                      host, sizeof(host)), "embedded NUL cannot hide malicious suffix");
  const char* uri = "mqtts://blynk.cloud:8883";
  const size_t uriLength = std::strlen(uri);
  const size_t hostLength = std::strlen("blynk.cloud");
  expect(MqttProtocol::parseRedirect(bytes(uri), uriLength, host, hostLength + 1),
         "output capacity includes terminating NUL");
  expect(!MqttProtocol::parseRedirect(bytes(uri), uriLength, host, hostLength),
         "output too short for terminating NUL rejected");
  expect(!MqttProtocol::parseRedirect(bytes(uri), uriLength, host, 0), "zero capacity rejected");
  expect(!MqttProtocol::parseRedirect(bytes(uri), uriLength, nullptr, sizeof(host)),
         "null output rejected");
  expect(!MqttProtocol::parseRedirect(nullptr, uriLength, host, sizeof(host)), "null URI rejected");
  char longUri[97];
  std::memset(longUri, 'a', sizeof(longUri));
  std::memcpy(longUri, "mqtts://", 8);
  expect(!MqttProtocol::parseRedirect(bytes(longUri), sizeof(longUri), host, sizeof(host)),
         "oversize redirect rejected");
}

void testTemplateIds() {
  expect(MqttProtocol::validTemplateId("TMPLabcd1234"), "normal Template ID accepted");
  expect(MqttProtocol::validTemplateId("TMPLabc_-XYZ9"), "allowed Template ID characters accepted");
  const char* const invalid[] = {"", "TMPL", "wrong", "tmplabc", "TMPL_REPLACE_ME",
      "TMPL abc", "TMPLabc\"injection", "TMPLabc/def", "TMPLabc\n"};
  for (const char* id : invalid) expect(!MqttProtocol::validTemplateId(id), "invalid Template ID rejected");
  expect(!MqttProtocol::validTemplateId(nullptr), "null Template ID rejected");
  char boundary[66];
  std::memset(boundary, 'a', sizeof(boundary));
  std::memcpy(boundary, "TMPL", 4);
  boundary[64] = 0;
  expect(MqttProtocol::validTemplateId(boundary), "64-character Template ID accepted");
  boundary[64] = 'a';
  boundary[65] = 0;
  expect(!MqttProtocol::validTemplateId(boundary), "65-character Template ID rejected");
}
}  // namespace

int main() {
  testCommands();
  testHosts();
  testRedirects();
  testTemplateIds();
  if (failures) {
    std::fprintf(stderr, "%d of %d protocol checks failed.\n", failures, checks);
    return 1;
  }
  std::printf("Passed %d MQTT protocol checks.\n", checks);
  return 0;
}
