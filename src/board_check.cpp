#include <Arduino.h>

namespace {
uint32_t lastHeartbeatMs = 0;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== ESP32 board check ===");
  Serial.printf("Chip: %s | Revision: %u | Cores: %u\n",
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("Flash: %u bytes | Free heap: %u bytes\n",
                static_cast<unsigned>(ESP.getFlashChipSize()),
                static_cast<unsigned>(ESP.getFreeHeap()));
  Serial.println("No relay GPIO is configured by this firmware.");
}

void loop() {
  const uint32_t now = millis();
  if (now - lastHeartbeatMs >= 2000) {
    lastHeartbeatMs = now;
    Serial.printf("[OK] Uptime: %lu s | Free heap: %u bytes\n",
                  static_cast<unsigned long>(now / 1000),
                  static_cast<unsigned>(ESP.getFreeHeap()));
  }
  delay(1);
}
