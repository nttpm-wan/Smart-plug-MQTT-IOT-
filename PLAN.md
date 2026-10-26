# แผนโปรเจกต์ Smart Plug — ESP32 + Blynk

วันที่เริ่ม: 15 ตุลาคม 2026

โฟลเดอร์ปัจจุบัน: `D:\Vscode\smart plug\smart plug` (ผู้ใช้ย้ายจากไดรฟ์ C และลบต้นฉบับแล้ว)

## เป้าหมาย

สร้างโปรเจกต์ ESP32 ใน VS Code ด้วย PlatformIO แล้วพัฒนาการสั่งเปิด/ปิดผ่าน Blynk IoT ทีละขั้น เริ่มจากตรวจบอร์ดผ่าน USB และจำลองสถานะรีเลย์ใน Serial ก่อนต่ออุปกรณ์จริง

## ข้อมูลตั้งต้นและข้อจำกัด

- พบ VS Code, PlatformIO IDE และ PlatformIO Core 6.1.19 ติดตั้งแล้ว
- ยังไม่พบพอร์ต USB ของ ESP32 (พบเฉพาะ COM1 ของเครื่อง)
- ใช้ `esp32dev` เป็นค่าเริ่มต้นชั่วคราวสำหรับ ESP32 แบบดั้งเดิม ต้องยืนยันรุ่นก่อนอัปโหลด โดยเฉพาะ ESP32-C3/S3/C6
- ยังไม่มี Wi-Fi credentials, Blynk Template ID และ Device Auth Token
- เก็บข้อมูลลับใน `include/secrets.h` ที่ถูกละเว้นจาก Git ไม่เขียนข้อมูลจริงในตัวอย่างหรือรายงาน
- ปิดการขับ GPIO รีเลย์ไว้โดยค่าเริ่มต้น จนกว่าจะยืนยันรุ่นโมดูล ขาควบคุม และ Active LOW/HIGH
- ทดสอบอุปกรณ์ด้วย USB/โหลด DC แรงดันต่ำ การต่อไฟบ้านต้องให้อาจารย์หรือช่างไฟตรวจวงจรและอุปกรณ์

## ขั้นตอนและเกณฑ์สำเร็จ

1. [x] ตรวจเครื่องมือเดิม พอร์ตอนุกรม และคำสั่งของโฟลเดอร์
2. [x] สร้างโฟลเดอร์ `smart plug` และบันทึกแผนนี้ก่อนเริ่มพัฒนา
3. [x] สร้างโครงสร้าง PlatformIO และเปิดโฟลเดอร์ใน VS Code
   - มี `platformio.ini`, `src/`, `include/`, `.vscode/` และ `.gitignore`
   - ใช้สอง build environments: `esp32-check` และ `smart-plug`
4. [x] ติดตั้งแพลตฟอร์ม ESP32 และไลบรารี Blynk ที่โปรเจกต์ต้องใช้
   - ล็อกเวอร์ชันที่ติดตั้งได้จริงเพื่อให้ build ซ้ำได้
5. [x] ทำเฟิร์มแวร์ตรวจบอร์ด `esp32-check`
   - แสดงชื่อชิปและ heartbeat ทาง Serial 115200 โดยไม่ขับขารีเลย์
6. [x] ทำเฟิร์มแวร์ Blynk `smart-plug`
   - V0 รับคำสั่ง OFF/ON; V1 ส่งสถานะเอาต์พุตที่ซอฟต์แวร์สั่ง (ไม่ใช่การวัดกระแสหรือยืนยันหน้าสัมผัส)
   - แยกไฟล์ข้อมูลลับ มีตัวอย่างให้กรอก
   - เริ่มต้น OFF; เมื่อการเชื่อมต่อขาดให้สั่ง OFF และไม่ดึงคำสั่ง ON เก่ากลับมาเมื่อเชื่อมต่อใหม่
   - ค่าเริ่มต้นจำลองรีเลย์ใน Serial จึงเตรียมโค้ดได้โดยยังไม่ต่อฮาร์ดแวร์
7. [x] คอมไพล์ทั้งสอง environments และแก้ข้อผิดพลาดที่พบ
   - บันทึกผลจริง แยกผลคอมไพล์ออกจากผลบนฮาร์ดแวร์
8. [x] เขียน README และคู่มือ Blynk/การทดสอบ แล้วอัปเดตสถานะในแผน
9. [ ] ยืนยันรุ่น ESP32 และตรวจพอร์ต USB ก่อนอัปโหลด `esp32-check`
10. [ ] ตั้งค่า Template/Device ในบัญชี Blynk และกรอก credentials ในเครื่อง
11. [ ] อัปโหลด `smart-plug` และทดสอบ V0/V1 ในโหมดจำลอง
12. [ ] ยืนยันโมดูลรีเลย์และทดสอบกับโหลด DC แรงดันต่ำ

## บันทึกความคืบหน้า

- ตรวจเครื่องมือแล้ว: VS Code และ PlatformIO ใช้งานได้ แต่ยังไม่มีแพลตฟอร์ม ESP32 ในเครื่อง
- ยังไม่ได้อัปโหลดเฟิร์มแวร์หรือสั่งอุปกรณ์จริง
- ผู้ใช้ยืนยันว่าใช้ ESP32 Wi-Fi แต่ยังไม่ได้เสียบ USB และยังไม่มีรีเลย์ ต้องการเตรียมโค้ดก่อนและยอมรับโหมดจำลองใน Serial
- จึงทำขั้นตอนเตรียมโค้ดและคอมไพล์ในรอบนี้ ส่วนขั้นตอน 9–12 รอฮาร์ดแวร์และข้อมูลเชื่อมต่อ
- เปิดโฟลเดอร์และ PLAN.md ผ่าน VS Code CLI แล้ว เครื่องมือ Computer Use ปฏิเสธการเข้าถึงหน้าต่าง VS Code จึงทำงานต่อผ่านไฟล์และ PlatformIO CLI
- ติดตั้งและล็อก Espressif 32 `7.1.3`, Blynk `1.4.0` แล้ว (Blynk มี dependency BlynkNcpDriver `0.7.0`)
- ตรวจ JSON ของการตั้งค่า VS Code และตรวจ PlatformIO project config ผ่านแล้ว
- สร้าง IntelliSense metadata ด้วย `platformio project init --ide vscode --environment smart-plug` สำเร็จ และเตรียม task ให้กด Ctrl+Shift+B เพื่อ build ได้
- คอมไพล์ด้วย `platformio run -e esp32-check -e smart-plug` สำเร็จทั้งสอง environment (exit code 0)

## ผลคอมไพล์ 15 ตุลาคม 2026

| Environment | ผล | RAM ที่ใช้ | Flash ที่ใช้ |
|---|---|---|---|
| `esp32-check` | SUCCESS | 21,472 / 327,680 bytes (6.6%) | 269,021 / 1,310,720 bytes (20.5%) |
| `smart-plug` | SUCCESS | 39,616 / 327,680 bytes (12.1%) | 665,909 / 1,310,720 bytes (50.8%) |

Framework ที่ใช้จริง: `framework-arduinoespressif32 4.20017.260907+sha.dcc1105b`; toolchain `8.4.0+2021r2-patch5`

เสร็จขั้นตอน 1–8 ในขอบเขตเตรียมโค้ดตามที่ผู้ใช้ขอ ยังไม่ได้ทดสอบการรัน Serial, การเชื่อม Blynk, การเชื่อมต่อหลุด หรือเอาต์พุตจริงบนฮาร์ดแวร์ ขั้นตอน 9–12 เก็บไว้ทำเมื่อมีบอร์ด/รีเลย์และข้อมูลเชื่อมต่อ

## เพิ่ม Blynk ผ่าน MQTT — แผนที่ตกลงกับผู้ใช้

ผู้ใช้เลือก Blynk Device MQTT API โดยยังเตรียมโค้ดก่อน ไม่มีบอร์ด USB หรือรีเลย์สำหรับทดสอบจริง

1. [x] ตรวจเอกสารทางการเรื่อง TLS, authentication, topic และ regional endpoint/redirect
2. [x] เพิ่ม environment `blynk-mqtt` และกำหนดเป็นค่าเริ่มต้นในการ Build
3. [x] ทำ MQTT firmware: TLS 8883, username `device`, password เป็น Device Token, clean session และ keepalive 45 วินาที
4. [x] รับคำสั่งผ่าน `downlink/ds/Relay Command` และส่งสถานะผ่าน `ds/Commanded Output` ให้ตรงกับชื่อ Datastream เดิม
5. [x] คงการจำลองรีเลย์ใน Serial เริ่ม OFF และสั่ง OFF เมื่อโปรแกรมตรวจพบการเชื่อมต่อหลุด
6. [x] เพิ่มคู่มือ MQTT และข้อจำกัด; จัดการ region ให้ไม่ส่ง token ไปปลายทางที่ไม่ได้ตรวจสอบ
7. [x] คอมไพล์ `blynk-mqtt` พร้อมตรวจ environments เดิม และอัปเดต IntelliSense/ผลจริงในแผน
8. [ ] ทดสอบ MQTT บน ESP32 กับบัญชี Blynk จริงภายหลัง (รอฮาร์ดแวร์และ credentials)

### ผลการเพิ่ม MQTT

- เพิ่ม `src/mqtt_main.cpp`, `include/mqtt_config.h`, `include/mqtt_protocol.h` และ `docs/MQTT_SETUP.md`
- ใช้ PubSubClient `2.8` และ public CA bundle จาก Blynk `1.4.0` โดยเก็บ MIT license และตรวจ SHA256 ว่าสำเนา bundle ตรงกับต้นฉบับ
- ค่าเริ่มต้นของ PlatformIO และ task Ctrl+Shift+B เป็น `blynk-mqtt`; อัปเดต IntelliSense สำหรับ PubSubClient
- ตรวจ config/task: เลือก environment ถูกต้อง, compiler มีอยู่จริง, ยังปิด relay output และยังไม่มี `secrets.h`/credentials จริง
- ตรวจโค้ด reconnect, การตรวจค่าคำสั่งแบบตรงตัว 0/1, redirect allowlist และการไม่ publish ภายใน MQTT callback แล้ว
- คำสั่งตรวจสุดท้าย `platformio run -e blynk-mqtt -e smart-plug -e esp32-check` สำเร็จทั้งสาม environment (exit code 0)

| Environment | ผล | RAM | Flash |
|---|---|---|---|
| `esp32-check` | SUCCESS | 21,472 bytes (6.6%) | 269,021 bytes (20.5%) |
| `smart-plug` | SUCCESS | 39,616 bytes (12.1%) | 665,909 bytes (50.8%) |
| `blynk-mqtt` | SUCCESS | 39,360 bytes (12.0%) | 650,561 bytes (49.6%) |

ผลข้างต้นเป็นการคอมไพล์และตรวจ config/source เท่านั้น ยังไม่ได้ทดสอบการเชื่อม broker, redirect, คำสั่งจากแอป, reconnect หรือ Serial runtime บน ESP32 จริง ไม่ได้สร้างบัญชี/Device หรือส่ง Token ไปยัง Blynk

## ตรวจหลังย้ายโฟลเดอร์ — 22 ตุลาคม 2026

- [x] ตรวจไฟล์ source, headers, CA bundle, license และคู่มือที่จำเป็น: อยู่ครบ
- [x] ตรวจ task Ctrl+Shift+B: มี default task เดียว เลือก `blynk-mqtt` และใช้ `${workspaceFolder}` เป็น working directory
- [x] พบและแก้ IntelliSense/Debug ที่ยังอ้างอิงโฟลเดอร์เดิม โดยสร้าง metadata ใหม่ด้วย `platformio project init --ide vscode --environment blynk-mqtt`
- [x] ตรวจ include paths ที่ไม่ใช่ค่าว่าง 232 รายการ: ไม่พบพาธที่หาย และไม่พบการอ้างอิงโฟลเดอร์เดิมในไฟล์ `.vscode/*.json`
- [x] ตรวจ Debug configurations ทั้งสาม: firmware.elf ชี้ที่ไดรฟ์ D และไฟล์มีอยู่จริง (ยังไม่ได้ทดสอบ debugger กับฮาร์ดแวร์)
- [x] ล้างเฉพาะ build artifacts ใน `.pio/build` แล้วคอมไพล์ใหม่ทั้งสาม environments: SUCCESS, exit code 0
- [x] ตรวจ CA bundle ตรงกับต้นฉบับ Blynk และใบรับรองทั้งสามยังไม่หมดอายุ ณ วันที่ตรวจ ไม่พบการเรียก `setInsecure()` ใน source
- [x] ยืนยันว่าเอาต์พุตรีเลย์ยังปิด (`kRelayOutputEnabled = false`) และยังไม่มี `include/secrets.h`

| Environment | ผล clean build | RAM | Flash |
|---|---|---|---|
| `esp32-check` | SUCCESS | 21,472 bytes (6.6%) | 269,021 bytes (20.5%) |
| `smart-plug` | SUCCESS | 39,616 bytes (12.1%) | 665,909 bytes (50.8%) |
| `blynk-mqtt` | SUCCESS | 39,360 bytes (12.0%) | 650,561 bytes (49.6%) |

การคอมไพล์ครั้งแรกภายใน sandbox เปิด compiler ไม่ได้ด้วย `Illegal System DLL Relocation` จึงตรวจ `g++ --version` และคอมไพล์นอก sandbox สำเร็จ ปัญหานี้เกิดกับสภาพแวดล้อมรันคำสั่งของตัวช่วย ไม่ได้พบ error จาก source ใน clean build ที่สำเร็จ

ผลตรวจยืนยันว่าไฟล์ครบและพร้อม Build ที่ตำแหน่งใหม่ แต่ยังไม่ยืนยันการรันบน ESP32, Serial, MQTT/TLS handshake กับ Blynk, reconnect หรือรีเลย์จริง ต้องกรอก credentials และทดสอบฮาร์ดแวร์ต่อไป

## เตรียมเผยแพร่ GitHub — 22 ตุลาคม 2026

ปลายทาง: https://github.com/nttpm-wan/Smart-plug-MQTT-IOT-

ผู้ใช้อนุมัติให้จัด .gitignore, คำอธิบายสถานะจริง, tests, GitHub Actions และ MIT License ก่อนอัปโหลด

- [x] เพิ่ม ignore สำหรับ Debug metadata, native test outputs, firmware exports และไฟล์ environment ส่วนตัว
- [x] เพิ่ม MIT License โดยใช้ชื่อบัญชีเจ้าของ repository เป็น copyright holder และเก็บ license ของ Blynk bundle ไว้เดิม
- [x] เพิ่ม tests ที่เรียกใช้ `mqtt_protocol.h` จริง โดยไม่ใช้ ESP32
- [x] เพิ่ม GitHub Actions สำหรับ native tests และ compile ทั้งสาม firmware environments
- [x] เพิ่มสถานะ prototype และ roadmap ใน README รวมถึงข้อความคำอธิบาย GitHub ที่ตรงกับความสามารถปัจจุบันใน `docs/GITHUB_DESCRIPTION.txt`
- [x] ตรวจ tests, firmware builds, รายการไฟล์และข้อมูลลับก่อนเตรียมขึ้น Git
- [ ] ให้ผู้ใช้ตรวจผลและอนุมัติ push หลังเตรียมเสร็จ

### ผลตรวจเตรียมเผยแพร่

- Native tests ผ่าน 105 checks โดยคอมไพล์ header จริงด้วย Zig C++ บน Windows (ใช้ compiler แบบพกพาเฉพาะในโฟลเดอร์ทำงาน ไม่รวมใน repository)
- คอมไพล์ `esp32-check`, `smart-plug`, `blynk-mqtt` ผ่านทั้งหมดหลังเพิ่มไฟล์ tests/CI
- ตรวจ syntax ของ test runner, YAML workflow, read-only contents permission, pinned action commits และ build matrix ผ่าน
- เริ่ม Git branch `main` และตั้ง origin ตาม URL ของผู้ใช้แล้ว เตรียมไฟล์ใน staging โดยไม่รวม `.pio`, `build`, `secrets.h`, `.env`, firmware binaries หรือ generated Debug metadata
- MIT License ใช้ชื่อบัญชี `nttpm-wan` และคง license ของ third-party CA bundle ไว้เดิม
- ยังไม่ commit/push และ GitHub Actions ยังไม่ได้รันบน GitHub จริง
- ข้อความคำอธิบายใหม่พร้อมแล้ว แต่ยังแก้ repository description บน GitHub ไม่ได้: ไม่พบ Git credential แบบใช้งานได้โดยไม่ถาม และเบราว์เซอร์สำหรับงานนี้แสดงสถานะ signed out ผู้ใช้ต้องเข้าสู่ระบบ GitHub ก่อนขั้นเผยแพร่ ไม่ต้องส่ง Token ในแชต

## แหล่งอ้างอิง

- https://docs.platformio.org/en/stable/boards/espressif32/esp32dev.html
- https://docs.platformio.org/en/latest/integration/ide/vscode.html
- https://docs.blynk.io/en/getting-started/activating-devices/manual-device-activation
- https://docs.blynk.io/en/getting-started/using-virtual-pins-to-control-physical-devices
- https://docs.blynk.io/en/blynk.cloud-mqtt-api/device-mqtt-api/authentication
- https://docs.blynk.io/en/blynk.cloud-mqtt-api/device-mqtt-api/datastreams
- https://github.com/Blynk-Technologies/Blynk-MQTT-Samples/tree/main/Arduino_Blynk_MQTT
