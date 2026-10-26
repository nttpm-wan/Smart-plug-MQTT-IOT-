#include <Arduino.h>
#include <cstring>
#include <ctime>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

#include <WiFi.h>
#include <BlynkSimpleEsp32_SSL.h>

#include "config.h"

namespace {
bool relayOn = false;
bool cloudConfigured = false;
bool wasCloudConnected = false;
bool clockStarted = false;
uint32_t lastWifiAttemptMs = 0;
uint32_t lastCloudAttemptMs = 0;
uint32_t lastHeartbeatMs = 0;
char serialBuffer[32] = {};
size_t serialLength = 0;
bool serialOverflow = false;

bool hasCredentials() {
  return std::strlen(WIFI_SSID) > 0 &&
         std::strlen(BLYNK_AUTH_TOKEN) > 0 &&
         std::strlen(BLYNK_TEMPLATE_ID) > 0 &&
         std::strcmp(BLYNK_TEMPLATE_ID, "TMPL_REPLACE_ME") != 0;
}

void writeRelayPin() {
  if (!Config::kRelayOutputEnabled) return;
  const uint8_t onLevel = Config::kRelayActiveLow ? LOW : HIGH;
  const uint8_t offLevel = Config::kRelayActiveLow ? HIGH : LOW;
  digitalWrite(Config::kRelayPin, relayOn ? onLevel : offLevel);
}

void publishState() {
  if (!cloudConfigured || !Blynk.connected()) return;
  // V1 reports the commanded output, not contact/current feedback.
  Blynk.virtualWrite(V1, relayOn ? 1 : 0);
}

void setRelay(bool on, const char* reason) {
  relayOn = on;
  writeRelayPin();
  Serial.printf("[%s] Relay=%s | %s\n",
                Config::kRelayOutputEnabled ? "HARDWARE" : "SIMULATION",
                relayOn ? "ON" : "OFF", reason);
  publishState();
}

void printStatus() {
  Serial.printf("mode=%s | relay=%s | WiFi=%s | Blynk=%s | uptime=%lu s\n",
                Config::kRelayOutputEnabled ? "HARDWARE" : "SIMULATION",
                relayOn ? "ON" : "OFF",
                WiFi.status() == WL_CONNECTED ? "connected" : "offline",
                cloudConfigured && Blynk.connected() ? "connected" : "offline",
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
      Serial.println("Local on/off is disabled in HARDWARE mode. Use Blynk V0.");
      return;
    }
    setRelay(std::strcmp(command, "on") == 0, "Serial demo command");
  } else if (command[0] != '\0') {
    Serial.println("Unknown command. Type help and press Enter.");
  }
}

void readSerial() {
  // Bound each pass so incoming Serial data cannot starve connection checks.
  for (uint8_t count = 0; count < 64 && Serial.available(); ++count) {
    const char ch = static_cast<char>(Serial.read());
    if (ch == '\r' || ch == '\n') {
      if (serialOverflow) {
        Serial.println("Command too long; discarded.");
      } else if (serialLength > 0) {
        serialBuffer[serialLength] = '\0';
        processSerialCommand(serialBuffer);
      }
      serialLength = 0;
      serialOverflow = false;
    } else if (serialLength < sizeof(serialBuffer) - 1 && !serialOverflow) {
      serialBuffer[serialLength++] = ch;
    } else {
      serialOverflow = true;
    }
  }
}

void keepConnected() {
  if (!cloudConfigured) return;

  const uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (wasCloudConnected) {
      Blynk.disconnect();
      wasCloudConnected = false;
      setRelay(false, "WiFi lost; output forced OFF");
    }
    if (now - lastWifiAttemptMs >= Config::kWifiRetryMs) {
      lastWifiAttemptMs = now;
      Serial.println("[WiFi] Retrying connection...");
      WiFi.reconnect();
    }
    return;
  }

  if (!clockStarted) {
    // TLS certificate validation needs a usable clock; do not disable TLS checks.
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    clockStarted = true;
    Serial.println("[TLS] Waiting for network time...");
  }

  if (!Blynk.connected()) {
    if (wasCloudConnected) {
      wasCloudConnected = false;
      setRelay(false, "Blynk disconnected; output forced OFF");
    }
    if (time(nullptr) >= 1700000000 &&
        now - lastCloudAttemptMs >= Config::kCloudRetryMs) {
      lastCloudAttemptMs = now;
      Serial.println("[Blynk] Connecting over TLS...");
      // DNS/TLS can take longer than this protocol budget. This is not a
      // hard real-time safety cutoff; hardware protection is still required.
      if (!Blynk.connect(Config::kCloudConnectBudgetMs)) {
        Serial.println("[Blynk] Connection failed; will retry.");
      }
    }
  }

  if (Blynk.connected()) {
    wasCloudConnected = true;
    Blynk.run();
    if (!Blynk.connected()) {
      wasCloudConnected = false;
      setRelay(false, "Blynk connection lost; output forced OFF");
    }
  }
}
}  // namespace

BLYNK_CONNECTED() {
  setRelay(false, "New cloud session; starts OFF");
  // Do not syncVirtual(V0): a saved ON command must not restore the output.
  Blynk.virtualWrite(V0, 0);
  publishState();
  Serial.println("[Blynk] Ready: V0=command, V1=commanded output status.");
}

BLYNK_WRITE(V0) {
  const int command = param.asInt();
  if (command != 0 && command != 1) {
    Serial.println("[Blynk] Ignored command outside 0/1.");
    Blynk.virtualWrite(V0, relayOn ? 1 : 0);
    return;
  }
  setRelay(command == 1, "Blynk V0 command");
}

void setup() {
  // Software initialization does not guarantee GPIO levels during reset/boot.
  // Actual relay hardware needs an appropriate default-OFF input circuit.
  if (Config::kRelayOutputEnabled) {
    const uint8_t offLevel = Config::kRelayActiveLow ? HIGH : LOW;
    digitalWrite(Config::kRelayPin, offLevel);
    pinMode(Config::kRelayPin, OUTPUT);
    digitalWrite(Config::kRelayPin, offLevel);
  }

  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Smart Plug: ESP32 + Blynk ===");
  setRelay(false, "Boot default");
  Serial.println("Status is a software command, not measured load feedback.");
  Serial.println("Type help and press Enter. Serial monitor: 115200 baud.");

  cloudConfigured = hasCredentials();
  if (!cloudConfigured) {
    Serial.println("[DEMO] Blynk credentials missing; cloud connection disabled.");
    Serial.println("Copy include/secrets.example.h to include/secrets.h locally.");
    if (Config::kRelayOutputEnabled) {
      Serial.println("[HARDWARE] No credentials: output remains OFF.");
    }
    return;
  }

  Blynk.config(BLYNK_AUTH_TOKEN);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiAttemptMs = millis();
  Serial.println("[WiFi] Connecting; credentials are not printed.");
}

void loop() {
  keepConnected();
  readSerial();

  const uint32_t now = millis();
  if (now - lastHeartbeatMs >= Config::kHeartbeatMs) {
    lastHeartbeatMs = now;
    printStatus();
    publishState();
  }
  delay(1);
}
