#include <Arduino.h>
#include <cstring>
#include <ctime>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "config.h"
#include "mqtt_config.h"
#include "mqtt_protocol.h"

namespace {
const char kRootCertificates[] =
#include "certs/blynk_roots.h"

WiFiClientSecure tls;
PubSubClient mqtt(tls);
bool relayOn = false;
bool configured = false;
bool sessionReady = false;
bool blocked = false;
bool clockStarted = false;
bool stateDirty = false;
bool commandDirty = false;
bool redirectPending = false;
char brokerHost[64] = BLYNK_MQTT_HOST;
char redirectHost[64] = {};
char clientId[40] = {};
char serialBuffer[32] = {};
size_t serialLength = 0;
bool serialOverflow = false;
uint32_t lastWifiAttemptMs = 0;
uint32_t lastMqttAttemptMs = 0;
uint32_t lastHeartbeatMs = 0;

void applyRelay(bool on, const char* reason) {
  relayOn = on;
  if (Config::kRelayOutputEnabled) {
    const uint8_t onLevel = Config::kRelayActiveLow ? LOW : HIGH;
    const uint8_t offLevel = Config::kRelayActiveLow ? HIGH : LOW;
    digitalWrite(Config::kRelayPin, relayOn ? onLevel : offLevel);
  }
  stateDirty = true;
  Serial.printf("[%s] Relay=%s | %s\n",
                Config::kRelayOutputEnabled ? "HARDWARE" : "SIMULATION",
                relayOn ? "ON" : "OFF", reason);
}

void disconnectAndOff(const char* reason) {
  sessionReady = false;
  // Set the local output before any potentially blocking socket operation.
  applyRelay(false, reason);
  commandDirty = true;
  mqtt.disconnect();
  tls.stop();
}

bool publishState() {
  if (!sessionReady || !mqtt.connected()) return false;
  const char* value = relayOn ? "1" : "0";
  if (commandDirty) {
    if (!mqtt.publish(MqttConfig::kCommandReportTopic, value, false)) return false;
    commandDirty = false;
  }
  if (stateDirty) {
    // This is commanded state, never a claim of measured load/contact state.
    if (!mqtt.publish(MqttConfig::kStateTopic, value, false)) return false;
    stateDirty = false;
  }
  return true;
}

void onMqttMessage(char* topic, uint8_t* payload, unsigned int length) {
  // Do not publish or reconnect inside this callback: PubSubClient shares its
  // receive/transmit buffer. Queue work for loop() after parsing the message.
  if (std::strcmp(topic, "downlink/redirect") == 0) {
    if (MqttProtocol::parseRedirect(payload, length, redirectHost,
                                    sizeof(redirectHost))) {
      redirectPending = true;
    } else {
      blocked = true;
      Serial.println("[MQTT] Redirect rejected: only known Blynk TLS endpoints allowed.");
    }
    return;
  }
  if (redirectPending || blocked || !sessionReady) return;
  if (std::strcmp(topic, MqttConfig::kCommandTopic) == 0) {
    bool on = false;
    if (!MqttProtocol::parseRelayCommand(payload, length, on)) {
      Serial.println("[MQTT] Invalid command: expected exactly 0 or 1.");
      commandDirty = true;
      return;
    }
    applyRelay(on, "Blynk MQTT command");
  } else if (std::strcmp(topic, "downlink/diag") == 0) {
    Serial.println("[MQTT] Server diagnostic received; check Blynk Device info.");
  }
  // QoS 1 ping PUBACK is handled by PubSubClient. OTA, reboot and remote Wi-Fi
  // provisioning are not implemented by this prototype.
}

void printStatus() {
  Serial.printf("mode=%s | relay=%s | WiFi=%s | MQTT=%s | uptime=%lu s\n",
                Config::kRelayOutputEnabled ? "HARDWARE" : "SIMULATION",
                relayOn ? "ON" : "OFF",
                WiFi.status() == WL_CONNECTED ? "connected" : "offline",
                sessionReady && mqtt.connected() ? "connected" : "offline",
                static_cast<unsigned long>(millis() / 1000));
}

void processSerialCommand(const char* command) {
  if (std::strcmp(command, "status") == 0) {
    printStatus();
  } else if (std::strcmp(command, "help") == 0) {
    Serial.println("Commands: status, help, on, off (on/off: simulation only)");
  } else if (std::strcmp(command, "on") == 0 ||
             std::strcmp(command, "off") == 0) {
    if (Config::kRelayOutputEnabled) {
      Serial.println("Local on/off disabled in HARDWARE mode. Use Blynk.");
      return;
    }
    if (blocked || redirectPending) {
      Serial.println("[DEMO] Endpoint transition/rejection: keeping simulation OFF.");
      return;
    }
    applyRelay(std::strcmp(command, "on") == 0, "Serial demo command");
    commandDirty = true;
  } else {
    Serial.println("Unknown command. Type help and press Enter.");
  }
}

void readSerial() {
  for (uint8_t count = 0; count < 64 && Serial.available(); ++count) {
    const char ch = static_cast<char>(Serial.read());
    if (ch == '\r' || ch == '\n') {
      if (serialOverflow) {
        Serial.println("Command too long; discarded.");
      } else if (serialLength) {
        serialBuffer[serialLength] = '\0';
        processSerialCommand(serialBuffer);
      }
      serialLength = 0;
      serialOverflow = false;
    } else if (!serialOverflow && serialLength < sizeof(serialBuffer) - 1) {
      serialBuffer[serialLength++] = ch;
    } else {
      serialOverflow = true;
    }
  }
}

void connectMqtt() {
  mqtt.setServer(brokerHost, MqttConfig::kPort);
  Serial.printf("[MQTT] Connecting to %s:8883 over TLS...\n", brokerHost);
  // Full overload explicitly requests a clean session. No LWT claims that a
  // disconnected physical load is OFF; there is no load feedback sensor.
  if (!mqtt.connect(clientId, "device", BLYNK_AUTH_TOKEN,
                    nullptr, 0, false, nullptr, true)) {
    Serial.printf("[MQTT] Connection failed (state=%d); retry later.\n", mqtt.state());
    tls.stop();
    return;
  }

  applyRelay(false, "New MQTT session; starts OFF");
  commandDirty = true;
  sessionReady = true;
  // Reset the command before subscribing; never request stored ON values.
  if (!publishState() || !mqtt.subscribe(MqttConfig::kSubscribeTopic, 1)) {
    disconnectAndOff("MQTT initialization failed");
    return;
  }
  char info[256];
  const int length = snprintf(info, sizeof(info),
      "{\"tmpl\":\"%s\",\"ver\":\"0.2.0-mqtt\","
      "\"build\":\"%s %s\",\"rxbuff\":1024}",
      BLYNK_TEMPLATE_ID, __DATE__, __TIME__);
  if (length <= 0 || static_cast<size_t>(length) >= sizeof(info) ||
      !mqtt.publish("info/mcu", info, false)) {
    disconnectAndOff("MQTT device info publish failed");
    return;
  }
  Serial.println("[MQTT] Connected; subscribed to downlink/#.");
}

void keepConnected() {
  if (!configured) return;
  if (blocked) {
    if (sessionReady || mqtt.connected()) {
      disconnectAndOff("MQTT endpoint rejected; restart after checking config");
    }
    return;
  }
  if (redirectPending) {
    disconnectAndOff("Blynk regional redirect");
    std::memcpy(brokerHost, redirectHost, sizeof(brokerHost));
    redirectPending = false;
    Serial.printf("[MQTT] Next endpoint: %s:8883\n", brokerHost);
    lastMqttAttemptMs = millis();
  }

  const uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (sessionReady || mqtt.connected()) disconnectAndOff("WiFi lost; OFF");
    if (now - lastWifiAttemptMs >= Config::kWifiRetryMs) {
      lastWifiAttemptMs = now;
      Serial.println("[WiFi] Retrying connection...");
      WiFi.reconnect();
    }
    return;
  }
  if (!clockStarted) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    clockStarted = true;
    Serial.println("[TLS] Waiting for network time...");
  }
  if (time(nullptr) < 1700000000) return;

  if (!mqtt.connected()) {
    if (sessionReady) disconnectAndOff("MQTT disconnected; OFF");
    if (now - lastMqttAttemptMs >= Config::kCloudRetryMs) {
      lastMqttAttemptMs = now;
      connectMqtt();
    }
  }
  if (sessionReady && !mqtt.loop()) {
    disconnectAndOff("MQTT connection lost; OFF");
  }
  // A callback may have scheduled a redirect/rejected endpoint. Do not send
  // further state on that session in this pass.
  if (blocked || redirectPending) {
    disconnectAndOff("MQTT endpoint change; OFF");
    return;
  }
  if (sessionReady && !publishState()) disconnectAndOff("MQTT publish failed; OFF");
}
}  // namespace

void setup() {
  if (Config::kRelayOutputEnabled) {
    // Hardware must also keep the relay OFF during reset, before setup runs.
    const uint8_t offLevel = Config::kRelayActiveLow ? HIGH : LOW;
    digitalWrite(Config::kRelayPin, offLevel);
    pinMode(Config::kRelayPin, OUTPUT);
    digitalWrite(Config::kRelayPin, offLevel);
  }
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Smart Plug: Blynk over MQTT/TLS ===");
  applyRelay(false, "Boot default");
  Serial.println("Type help and press Enter. Serial monitor: 115200 baud.");
  Serial.println("Relay state is commanded/simulated, not measured feedback.");

  configured = std::strlen(WIFI_SSID) > 0 && std::strlen(BLYNK_AUTH_TOKEN) > 0 &&
               MqttProtocol::validTemplateId(BLYNK_TEMPLATE_ID);
  if (!configured) {
    Serial.println("[DEMO] Missing credentials: MQTT disabled; Serial demo available.");
    Serial.println("Copy include/secrets.example.h to include/secrets.h locally.");
    return;
  }
  if (!MqttProtocol::isAllowedHost(brokerHost) ||
      !mqtt.setBufferSize(MqttConfig::kBufferSize)) {
    configured = false;
    Serial.println("[MQTT] Invalid broker or buffer allocation failed; cloud disabled.");
    return;
  }
  tls.setCACert(kRootCertificates);
  tls.setHandshakeTimeout(5);
  mqtt.setKeepAlive(MqttConfig::kKeepAliveSeconds);
  mqtt.setSocketTimeout(MqttConfig::kSocketTimeoutSeconds);
  mqtt.setCallback(onMqttMessage);
  snprintf(clientId, sizeof(clientId), "smartplug-%012llX",
           static_cast<unsigned long long>(ESP.getEfuseMac()));
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiAttemptMs = millis();
  Serial.println("[WiFi] Connecting; credentials are not printed.");
}

void loop() {
  keepConnected();
  readSerial();
  if (sessionReady && !publishState()) disconnectAndOff("MQTT publish failed; OFF");
  const uint32_t now = millis();
  if (now - lastHeartbeatMs >= Config::kHeartbeatMs) {
    lastHeartbeatMs = now;
    printStatus();
  }
  delay(1);
}
