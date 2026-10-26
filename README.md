# Smart Plug — ESP32 + Blynk ผ่าน MQTT

ESP32 smart-plug prototype using Blynk over MQTT/TLS. It includes Serial relay simulation and switches the commanded output OFF when a disconnection is detected. Hardware validation and energy metering are planned; electrical measurements are not implemented.

โปรเจกต์ PlatformIO สำหรับ VS Code ใช้ **Blynk Device MQTT API ผ่าน TLS** เป็นโหมดหลัก เริ่มต้นใน **โหมดจำลองรีเลย์** โดยไม่ขับขารีเลย์ และคอมไพล์ได้โดยยังไม่กรอก Wi-Fi หรือ Blynk

ติดตามงานที่ [PLAN.md](PLAN.md)

## สถานะและแผนต่อยอด

| ความสามารถ | สถานะ |
|---|---|
| เฟิร์มแวร์ ESP32 และ Blynk MQTT/TLS | คอมไพล์ผ่าน; ยังรอทดสอบกับบัญชีและบอร์ดจริง |
| จำลอง ON/OFF ผ่าน Serial | มีโค้ด; ยังรอทดสอบ runtime บนบอร์ด |
| ตรวจคำสั่ง MQTT และ redirect | มี native tests ที่เรียก header จริง |
| ตรวจ Build บน GitHub | เตรียม workflow ไว้; ต้อง push และตรวจผล Actions |
| รีเลย์และโหลดจริง | ยังไม่ทดสอบ; GPIO output ปิดไว้ |
| วัดกระแส กำลังไฟ และพลังงาน | ยังไม่ทำ; ต้องเลือกฮาร์ดแวร์วัดและทดสอบต่อไป |

แผนต่อยอดคือยืนยันรุ่น ESP32 → ทดสอบ MQTT/การหลุดเชื่อมต่อ → ทดสอบรีเลย์กับโหลด DC แรงดันต่ำ → พิจารณาระบบวัดพลังงาน

## สิ่งที่เตรียมไว้

| ไฟล์ | หน้าที่ |
|---|---|
| `platformio.ini` | บอร์ด เฟรมเวิร์ก ไลบรารี และ build environments |
| `src/mqtt_main.cpp` | เฟิร์มแวร์หลัก: Blynk ผ่าน MQTT/TLS และจำลองรีเลย์ใน Serial |
| `src/main.cpp` | เฟิร์มแวร์เดิมที่ใช้ Blynk binary protocol |
| `src/board_check.cpp` | ตรวจข้อมูลบอร์ดและ heartbeat ผ่าน USB |
| `include/config.h` | โหมดจำลอง ขารีเลย์ และ Active LOW/HIGH |
| `include/mqtt_config.h` | MQTT endpoint, topics, keepalive และ buffer |
| `include/mqtt_protocol.h` | ตรวจคำสั่ง 0/1, Template ID และ redirect endpoint |
| `include/certs/` | public CA certificates และ license สำหรับ TLS |
| `include/secrets.example.h` | ตัวอย่างข้อมูลเชื่อมต่อที่ยังไม่มีความลับ |
| `docs/MQTT_SETUP.md` | คู่มือหลัก: ตั้งค่า Blynk MQTT และข้อมูลที่ต้องใส่ |
| `docs/BLYNK_SETUP.md` | คู่มือเฟิร์มแวร์เดิม |
| `docs/TEST_CHECKLIST.md` | ขั้นตอนทดสอบเมื่อมีบอร์ด |

## เริ่มใน VS Code

1. เปิดโฟลเดอร์ `smart plug` ที่มี `platformio.ini` อยู่โดยตรง
2. ใช้ PlatformIO IDE ที่ติดตั้งอยู่แล้ว รอให้อ่านโปรเจกต์เสร็จ
3. กด **Ctrl+Shift+B** เพื่อ build เฟิร์มแวร์ `blynk-mqtt` หรือเลือก PlatformIO → Project Tasks → blynk-mqtt → General → Build
4. ผลสำเร็จต้องลงท้ายด้วย `SUCCESS` การ Build ไม่ต้องเสียบ ESP32 และยังไม่อัปโหลดหรือสั่งอุปกรณ์จริง

PlatformIO Core ในเครื่องนี้อยู่ที่ `%USERPROFILE%\.platformio\penv\Scripts\platformio.exe` หากใช้ terminal ของ PlatformIO สามารถเรียกคำสั่ง `pio` ได้โดยตรง

```text
pio run -e blynk-mqtt
pio run -e esp32-check
```

| Environment | หน้าที่ |
|---|---|
| `blynk-mqtt` (ค่าเริ่มต้น) | MQTT/TLS สำหรับใช้งานตามที่เลือก |
| `esp32-check` | ตรวจข้อมูลบอร์ดผ่าน USB |
| `smart-plug` | เฟิร์มแวร์เดิม ใช้ `pio run -e smart-plug` เมื่อต้องการ |

ใช้ `esp32dev` เป็นค่าเริ่มต้นสำหรับ ESP32 แบบดั้งเดิม ไม่ใช่การยืนยันรุ่นบอร์ดจริง หากเป็น ESP32-C3/S3/C6 ให้เปลี่ยน board และตรวจ GPIO ให้ตรงก่อนอัปโหลด

## โหมดจำลองทำงานอย่างไร

ค่าเริ่มต้นใน `include/config.h` คือ `kRelayOutputEnabled = false` โปรแกรมจึงไม่ตั้ง GPIO รีเลย์เป็นเอาต์พุตและไม่เรียก digitalWrite ให้ขารีเลย์

เมื่ออัปโหลดลง ESP32 ในภายหลัง ให้เปิด Serial Monitor ที่ 115200 baud และส่งคำสั่งตัวพิมพ์เล็กพร้อม newline/Enter:

| คำสั่ง | ผล |
|---|---|
| `on` | เปลี่ยนสถานะจำลองเป็น ON |
| `off` | เปลี่ยนสถานะจำลองเป็น OFF |
| `status` | แสดงสถานะจำลอง Wi-Fi และ Blynk |
| `help` | แสดงรายการคำสั่ง |

ตัวอย่างข้อความที่คาดว่าจะเห็น (ยังไม่ใช่ผลทดสอบบนบอร์ดจริง):

```text
[SIMULATION] Relay=OFF | Boot default
[DEMO] Missing credentials: MQTT disabled; Serial demo available.
[SIMULATION] Relay=ON | Serial demo command
mode=SIMULATION | relay=ON | WiFi=offline | MQTT=offline | uptime=10 s
```

Serial ต้องมี ESP32 หรือระบบจำลองฮาร์ดแวร์ที่รันเฟิร์มแวร์ได้ การคอมไพล์บนคอมอย่างเดียวไม่ได้สร้าง Serial จาก ESP32

## เชื่อม Blynk ในภายหลัง

อ่าน [docs/MQTT_SETUP.md](docs/MQTT_SETUP.md) และคัดลอก `include/secrets.example.h` เป็น `include/secrets.h` แล้วกรอกข้อมูลในเครื่อง ค่าใน `secrets.h` จะถูกอ่านตอนคอมไพล์และฝังอยู่ในเฟิร์มแวร์ จึงไม่ควรเผยแพร่ทั้งไฟล์นี้และไฟล์ `.bin` ที่มีข้อมูลจริง

หากยังไม่มี `secrets.h` หรือข้อมูลไม่ครบ โปรแกรมใช้โหมด Serial demo และไม่เริ่มเชื่อม Blynk

## พฤติกรรมการควบคุม

- เริ่มต้น OFF ทุกครั้งที่บูต
- การเชื่อมต่อ MQTT รอบใหม่เริ่ม OFF และเขียน `Relay Command` (V0)/`Commanded Output` (V1) เป็น 0 ก่อน subscribe โดยไม่ร้องขอคำสั่ง ON ที่เก็บไว้
- เมื่อโปรแกรมตรวจพบ Wi-Fi หรือ MQTT ขาดหลังเคยเชื่อมต่อ จะสั่ง OFF และพยายามเชื่อมต่อใหม่
- การตรวจว่าหลุดอาจใช้เวลาตาม heartbeat ของไลบรารี และ DNS/TLS อาจบล็อกได้นานกว่าค่ารอของโปรโตคอล จึงไม่ใช่อุปกรณ์ตัดไฟฉุกเฉิน
- ใช้ TLS กับ WiFiClientSecure และ public CA bundle จาก Blynk รอเวลาจาก NTP ก่อนเชื่อมต่อ และตรวจ redirect ให้เป็น endpoint ที่รู้จักก่อนส่ง Token
- V1 คือสถานะที่ซอฟต์แวร์สั่ง ไม่ใช่การตรวจหน้าสัมผัสรีเลย์หรือการวัดว่ามีไฟที่เต้ารับ
- คำสั่ง Serial `on/off` ใช้ได้เฉพาะโหมดจำลอง
- ยังไม่ทำ OTA/remote reboot/Wi-Fi provisioning ดูรายละเอียดและข้อจำกัด retained message ในคู่มือ MQTT

## ก่อนใช้รีเลย์จริง

ยังไม่มีข้อมูลโมดูลรีเลย์ จึงคงโหมดจำลองไว้ ค่า GPIO 23 และ Active LOW เป็นเพียงค่าตั้งต้นที่ต้องตรวจใหม่ เปลี่ยน `kRelayOutputEnabled` หลังยืนยันวงจรและโมดูลแล้วเท่านั้น

รีเลย์ต้องมีวงจรขับที่เหมาะกับลอจิก 3.3V ของ ESP32 ห้ามต่อคอยล์รีเลย์เปล่าเข้าขา GPIO โดยตรง และต้องจัดฮาร์ดแวร์ให้รีเลย์ OFF ระหว่างรีเซต/บูต เพราะโค้ดรับประกันระดับขาในช่วงก่อน setup() ไม่ได้

ทดสอบกับ USB และโหลด DC แรงดันต่ำก่อน การประกอบเต้ารับไฟบ้านต้องให้อาจารย์หรือช่างไฟตรวจการแยกฉนวน ฟิวส์ กล่องปิด จุดยึดสาย สายดิน และพิกัดโหลดจริง ห้ามต่อไฟบ้านบน breadboard

## เวอร์ชันที่ล็อกไว้

- PlatformIO Espressif 32: `7.1.3`
- Blynk library: `1.4.0`
- PubSubClient: `2.8` สำหรับโหมด MQTT
- Framework: Arduino จากแพ็กเกจที่แพลตฟอร์มกำหนด

ดูผลคอมไพล์และงานที่ยังรอฮาร์ดแวร์ได้ใน `PLAN.md`

## ทดสอบก่อนส่งโค้ด

Native tests ตรวจคำสั่ง 0/1, ข้อมูลผิดรูปแบบ, null/buffer boundaries, Template ID และ redirect ที่แอบอ้างปลายทาง โดยเรียก `include/mqtt_protocol.h` ตัวเดียวกับเฟิร์มแวร์ ไม่ต้องใช้ ESP32 หรือ credentials

ต้องมี Python 3 และ compiler GCC/Clang ใน PATH จากนั้นรัน:

```text
python scripts/run_protocol_tests.py
```

หาก compiler ไม่ได้ชื่อ `c++` ให้ใช้ `--compiler g++` หรือ `--compiler clang++`/พาธเต็มของ compiler ส่วน `--compiler-arg` ใช้เพิ่ม argument สำหรับ compiler wrapper ได้ ผลตรวจนี้ไม่ครอบคลุม Wi-Fi, TLS handshake กับ Blynk หรือ GPIO บนฮาร์ดแวร์

GitHub Actions ใน `.github/workflows/ci.yml` จะรัน native tests และคอมไพล์แยกทั้งสาม environments เมื่อ push หรือเปิด pull request โดยใช้ไฟล์ตัวอย่างที่ไม่มี Token/Wi-Fi จริง ไม่ต้องเพิ่ม GitHub Secrets

## License

โค้ดของโปรเจกต์ใช้ [MIT License](LICENSE) ส่วน CA bundle ที่นำมาจาก Blynk คง license และ attribution ของต้นฉบับไว้ที่ [include/certs/LICENSE.Blynk](include/certs/LICENSE.Blynk)

คำอธิบาย repository ที่ตรงกับสถานะปัจจุบันเตรียมไว้ใน `docs/GITHUB_DESCRIPTION.txt`
