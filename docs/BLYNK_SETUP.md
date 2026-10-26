# ตั้งค่า Blynk IoT

เอกสารนี้สำหรับ environment เดิม `smart-plug` หากใช้โหมดหลัก `blynk-mqtt` ให้อ่าน [MQTT_SETUP.md](MQTT_SETUP.md) ซึ่งใช้ Datastream เดิมแต่รับส่งผ่าน MQTT

ขั้นตอนนี้ทำเมื่อพร้อมเชื่อมต่อจริง ตอนนี้โปรเจกต์ยังใช้โหมดจำลองโดยไม่กรอกข้อมูลก็ build ได้

1. เข้า Blynk.Console ด้วยบัญชีของคุณ และเปิด Developer Mode หากยังไม่เปิด
2. สร้าง Template ชื่อ `Smart Plug` เลือก hardware เป็น ESP32 และ connectivity เป็น Wi-Fi
3. เพิ่ม Datastreams แบบ Virtual Pin ตามตารางด้านล่าง
4. ใน Blynk mobile app เปิด Template เดียวกันและตั้ง Mobile Dashboard เพิ่ม Switch ให้ V0 และ LED/Value Display ให้ V1 หากใช้ Web Dashboard ให้ตั้ง widget ที่นั่นแยกด้วย
5. สร้าง Device จาก Template แล้วคัดลอก Template ID, Template Name และ Auth Token ของ Device เก็บในเครื่อง
6. คัดลอก `include/secrets.example.h` เป็น `include/secrets.h` และกรอกข้อมูลด้านล่าง
7. Build `smart-plug` ใหม่ เมื่อยืนยันรุ่นบอร์ดและต่อ USB แล้วจึงอัปโหลด

| Datastream | Type | Min/Max | หน้าที่ |
|---|---|---|---|
| V0: Relay Command | Integer | 0 / 1 | 0=OFF, 1=ON ใช้กับ Switch แบบค้างสถานะ |
| V1: Commanded Output | Integer | 0 / 1 | แสดงสถานะที่ซอฟต์แวร์สั่ง; เป็นสถานะจำลองเมื่อปิด hardware output |

## ข้อมูลใน secrets.h

```cpp
#pragma once
#define BLYNK_TEMPLATE_ID "ใส่ Template ID ที่ขึ้นต้นด้วย TMPL"
#define BLYNK_TEMPLATE_NAME "Smart Plug"
#define BLYNK_AUTH_TOKEN "ใส่ Device Auth Token"
#define WIFI_SSID "ชื่อ Wi-Fi"
#define WIFI_PASSWORD "รหัสผ่าน Wi-Fi"
```

ใช้ Wi-Fi 2.4GHz สำหรับ ESP32 แบบดั้งเดิม บอร์ดต้องเข้าถึงอินเทอร์เน็ต, NTP และ Blynk ได้ Wi-Fi ที่มีหน้าเว็บให้กดยอมรับ/ล็อกอินไม่รองรับในตัวอย่างนี้ โทรศัพท์ควบคุมผ่านอินเทอร์เน็ตจึงไม่จำเป็นต้องอยู่ Wi-Fi เดียวกัน

กรอกค่าจริงในไฟล์ภายในเครื่อง ไม่ส่งรหัสผ่านหรือ Auth Token ในแชต `secrets.h` ถูกละเว้นจาก Git แล้ว ส่วนไฟล์ตัวอย่างต้องคงเป็นข้อมูลว่าง

อย่าเปิดการซิงก์/กู้คืนค่า ON เดิมของ V0 หรือเพิ่ม `Blynk.syncVirtual(V0)` ลงในโค้ด หากต้องการคงพฤติกรรม reconnect แล้ว OFF

เมื่อเชื่อมสำเร็จ Serial จะแสดง `[Blynk] Ready` กด ON/OFF ในแอปควรเห็น `[SIMULATION] Relay=ON/OFF` และ V1 เปลี่ยนตาม จนกว่าจะเปิด hardware output โดยตั้งใจ

หากไม่มี `[Blynk] Ready` ให้ตรวจข้อมูลใน `secrets.h`, Wi-Fi, การเข้าถึง NTP และสถานะ Device ใน Blynk ก่อน อย่าแก้ด้วยการปิดการตรวจ TLS

## อ้างอิง

- [Manual Device Activation](https://docs.blynk.io/en/getting-started/activating-devices/manual-device-activation)
- [Virtual Pins](https://docs.blynk.io/en/getting-started/using-virtual-pins-to-control-physical-devices)
- [Set Up Mobile App Dashboard](https://docs.blynk.io/en/getting-started/template-quick-setup/set-up-mobile-app-dashboard)
