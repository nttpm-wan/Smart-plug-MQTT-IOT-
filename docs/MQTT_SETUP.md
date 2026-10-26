# Blynk ผ่าน MQTT — คู่มือตั้งค่า

โหมดหลักของโปรเจกต์คือ `blynk-mqtt` ใช้ ESP32 → Wi-Fi → MQTT/TLS → Blynk.Cloud และควบคุมผ่านแอป Blynk

ตอนนี้ยังไม่มีบอร์ดต่อ USB และไม่มีข้อมูลบัญชีสำหรับทดสอบ การ Build สำเร็จยืนยันว่าโค้ดคอมไพล์ได้ แต่ยังไม่ยืนยันการเชื่อม Blynk หรือพฤติกรรมบนฮาร์ดแวร์

## 1. สร้าง Template และ Datastream

ใน Blynk.Console เปิด Developer Mode แล้วสร้าง Template `Smart Plug` เลือก ESP32 / Wi-Fi หรือใช้ Template เดิมถ้าสร้างไว้แล้ว เพิ่ม Datastream ตามตาราง **ชื่อรวมช่องว่างและตัวพิมพ์ต้องตรงทุกตัว** MQTT API อ้างอิงชื่อ ไม่ได้ใช้เลข V0/V1 เป็นชื่อ topic โดยอัตโนมัติ

| Virtual Pin | Name | Type | Min / Max |
|---|---|---|---|
| V0 | `Relay Command` | Integer | 0 / 1 |
| V1 | `Commanded Output` | Integer | 0 / 1 |

ใน Mobile Dashboard เพิ่ม Switch สำหรับ V0 แบบค้างสถานะ และ LED/Value Display สำหรับ V1 ตั้งป้ายของ V1 ให้สื่อว่าเป็นสถานะจำลอง/สถานะที่โปรแกรมสั่ง หากใช้ Web Dashboard ให้ตั้ง widget ที่นั่นด้วย

ไม่เปิดการ sync คำสั่ง ON เก่าของ V0 ไม่ต้องเปิด MQTT Gateway API เพราะโปรเจกต์นี้ใช้ Device MQTT API สำหรับอุปกรณ์เดียว

## 2. สร้าง Device และเก็บข้อมูลในเครื่อง

สร้าง Device จาก Template แล้วเปิด Device Info เพื่อคัดลอก Template ID และ Device Auth Token

คัดลอก `include/secrets.example.h` เป็น `include/secrets.h` แล้วกรอก:

```cpp
#pragma once
#define BLYNK_TEMPLATE_ID "TMPLxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "Smart Plug"
#define BLYNK_AUTH_TOKEN "ใส่ Device Auth Token จริง"
#define WIFI_SSID "ชื่อ Wi-Fi 2.4GHz"
#define WIFI_PASSWORD "รหัสผ่าน Wi-Fi"
// Optional: regional hostname copied from your Blynk.Console
// #define BLYNK_MQTT_HOST "sgp1.blynk.cloud"
```

เมื่อยังไม่มีไฟล์นี้ หรือข้อมูลว่าง/Template ID ยังเป็นตัวอย่าง เฟิร์มแวร์ไม่เชื่อม MQTT และเปิด Serial demo ให้ใช้ `on`, `off`, `status`, `help` ได้หลังอัปโหลดลง ESP32

ไม่ส่งข้อมูลลับในแชต `secrets.h` ถูกละเว้นจาก Git แล้ว แต่ค่าเหล่านี้จะถูกฝังในเฟิร์มแวร์ที่ Build จึงไม่ควรแชร์ `.bin` ที่ใช้ credentials จริง

## 3. MQTT connection

| รายการ | ค่า |
|---|---|
| Host เริ่มต้น | `blynk.cloud` |
| Port | `8883` ใช้ TLS พร้อมตรวจใบรับรองและ hostname |
| Username | `device` |
| Password | Device Auth Token |
| Protocol | MQTT 3.1.1 |
| Keepalive | 45 วินาที |
| Session | Clean session |
| Subscribe | `downlink/#` ขอ QoS 1 |
| Publish | QoS 0 และไม่ retain |
| Buffer | 1024 bytes |

ไลบรารี PubSubClient รองรับ publish ที่ QoS 0 จึงไม่ควรตีความว่า publish สำเร็จเป็นการยืนยันว่าฮาร์ดแวร์ทำงานแล้ว ส่วน QoS 1 ping/ข้อความรับเข้าจะมี PUBACK ตามการทำงานของไลบรารี

บอร์ดต้องเข้าถึง Wi-Fi, DNS, NTP และ Blynk MQTT/TLS ได้ เฟิร์มแวร์รอเวลาจาก NTP ก่อนตรวจอายุใบรับรอง และไม่ใช้ `setInsecure()`

เมื่อ Blynk ส่ง `downlink/redirect` เฟิร์มแวร์จะสั่ง OFF แล้ว reconnect ไปปลายทางที่อนุญาตเฉพาะ `mqtts://` พอร์ต 8883 และ hostname เหล่านี้: `blynk.cloud`, `fra1.blynk.cloud`, `lon1.blynk.cloud`, `ny3.blynk.cloud`, `sgp1.blynk.cloud`, `blr1.blynk.cloud`

ตั้ง `BLYNK_MQTT_HOST` เป็น regional hostname ของบัญชีได้ โดยดู server ที่มุมล่างขวาใน Blynk.Console ไม่จำเป็นต้องเป็น Singapore แม้อยู่ประเทศไทย หาก Blynk เพิ่มภูมิภาคใหม่ต้องตรวจเอกสารแล้วอัปเดต allowlist ก่อนใช้ ไม่เปิดทางให้ส่ง Token ไป hostname อื่น

## 4. Topic ที่ใช้

| ทิศทาง | Topic | Payload |
|---|---|---|
| รับคำสั่งจากแอป | `downlink/ds/Relay Command` | `0` หรือ `1` เท่านั้น |
| อัปเดตปุ่มในแอป | `ds/Relay Command` | `0` หรือ `1` |
| ส่งสถานะที่โปรแกรมสั่ง | `ds/Commanded Output` | `0` หรือ `1` |
| ส่งข้อมูลเฟิร์มแวร์ | `info/mcu` | JSON ที่มี Template ID, version, build และ buffer size |
| รับการเปลี่ยน region | `downlink/redirect` | URI ของ Blynk |

เมื่อเปลี่ยนชื่อ Datastream ให้แก้ค่าที่ตรงกันใน `include/mqtt_config.h` แล้ว Build ใหม่ ห้ามเปลี่ยนเฉพาะใน Blynk โดยไม่แก้เฟิร์มแวร์

## 5. Build และทดสอบภายหลัง

ใน VS Code กด **Ctrl+Shift+B** เพื่อ Build `blynk-mqtt` หรือใช้ PlatformIO terminal:

```text
pio run -e blynk-mqtt
```

เมื่อมี ESP32 ให้ยืนยันรุ่นบอร์ดก่อน ถ้าเป็น ESP32-C3/S3/C6 ให้ตั้ง board ให้ตรง ใช้ `pio device list` หาพอร์ตจริง แล้วแทน `COMx` ในคำสั่ง:

```text
pio run -e blynk-mqtt -t upload --upload-port COMx
pio device monitor --port COMx --baud 115200
```

ตัวอย่างข้อความที่คาดว่าจะเห็นหลังตั้งค่าแล้ว (ไม่ใช่ผลทดสอบจริง):

```text
=== Smart Plug: Blynk over MQTT/TLS ===
[SIMULATION] Relay=OFF | Boot default
[WiFi] Connecting; credentials are not printed.
[TLS] Waiting for network time...
[MQTT] Connecting to blynk.cloud:8883 over TLS...
[MQTT] Connected; subscribed to downlink/#.
[SIMULATION] Relay=ON | Blynk MQTT command
```

อาจเห็นการ redirect แล้วเชื่อมอีกครั้งก่อน Device อยู่ในสถานะ Online ข้อความ subscribed หมายถึงไลบรารีส่งคำขอ subscribe ได้ ยังต้องทดสอบรับคำสั่งจากแอปจริง

## 6. พฤติกรรมและข้อจำกัด

- ยังปิดเอาต์พุต GPIO รีเลย์ด้วย `Config::kRelayOutputEnabled = false`
- เริ่ม OFF ทุกครั้งที่บูต และเมื่อเชื่อม MQTT รอบใหม่ Reset ค่า `Relay Command` กับ `Commanded Output` เป็น 0 ก่อน subscribe ไม่เรียก `get/ds` เพื่อดึง ON เดิม
- สั่ง OFF เมื่อโปรแกรมตรวจพบ Wi-Fi/MQTT หลุด เมื่อเปลี่ยน endpoint หรือเมื่อส่งข้อมูลไม่สำเร็จ การตรวจหลุดและ DNS/TLS อาจบล็อก จึงไม่ใช่การตัดไฟฉุกเฉินแบบทันที
- V1 เป็นสถานะที่ซอฟต์แวร์สั่ง/จำลอง ไม่ใช่การวัดกระแสหรือหน้าสัมผัสรีเลย์ ไม่มี Last Will ที่อ้างว่าโหลดจริง OFF เมื่อบอร์ดหลุด
- คำสั่ง Serial `on/off` ใช้เฉพาะโหมดจำลอง และจะอัปเดตปุ่มกับสถานะในแอปเมื่อ MQTT ต่ออยู่
- ไม่เก็บคำสั่ง ON เป็น retained message อย่างไรก็ตาม PubSubClient callback ไม่เปิดเผย retained flag จึงไม่ได้กรองข้อความ retain ที่ต้นทางส่งมา ต้องคงการตั้งค่าไม่กู้คืน ON เก่าและทดสอบ reconnect จริง
- ยังไม่ทำ Blynk.Air OTA, remote reboot หรือ Wi-Fi provisioning ผ่านแอป
- ใช้ Device Token เดียวกับ MQTT client ที่ต่อพร้อมกันหลายตัวไม่ได้ อย่าเปิด MQTT playground หรือ MQTT client อื่นด้วย Token เดียวกันขณะทดสอบบอร์ด

## แก้ปัญหา

- `Missing credentials`: ตรวจชื่อไฟล์ `secrets.h` และค่าที่กรอก รวมถึง Template ID ที่ขึ้นต้น TMPL
- `Connection failed (state=4)` หรือ `5`: ตรวจ Auth Token และ Device/Template
- ค้างรอ network time: ตรวจ NTP/อินเทอร์เน็ตของ Wi-Fi
- TLS ล้มเหลว: ตรวจเวลา DNS เครือข่าย และ trust roots ตาม `include/certs/README.md` ไม่ปิดการตรวจใบรับรอง
- ต่อได้แต่กดแอปไม่เปลี่ยนสถานะ: ตรวจชื่อ Datastream รวมช่องว่าง, widget ผูก V0/V1 ถูก Device และไม่มี client อื่นแย่ง Token
- `Redirect rejected`: ตรวจ region จาก Blynk.Console แล้วตรวจ hostname กับเอกสารทางการก่อนแก้ allowlist

## อ้างอิง

- [Blynk MQTT Authentication](https://docs.blynk.io/en/blynk.cloud-mqtt-api/device-mqtt-api/authentication)
- [Blynk MQTT Datastreams](https://docs.blynk.io/en/blynk.cloud-mqtt-api/device-mqtt-api/datastreams)
- [Official Arduino/PlatformIO sample](https://github.com/Blynk-Technologies/Blynk-MQTT-Samples/tree/main/Arduino_Blynk_MQTT)
- [PubSubClient API](https://pubsubclient.knolleary.net/api)
