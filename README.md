# Arduino Nano 33 BLE Sense Rev2 — Hardware Diagnostic & Unit Test Suite

This project provides a **modular hardware test suite** for the **Arduino Nano 33 BLE Sense Rev2**, powered by the Nordic **nRF52840** MCU.  
It verifies the functionality of all major onboard peripherals:

- Power & USB  
- Built‑in LED  
- IMU (BMI270 + BMM150)  
- PDM microphone  
- Environmental sensors (HS3003 + LPS22DF)  
- APDS9960 gesture & color sensor  
- BLE radio  
- Stress test  
- Integration test  

The goal is to provide a **simple, reliable, and repeatable** way to validate boards during development, manufacturing, or classroom use.

---

## 🧠 Overview

The test suite is structured around a simple `TestCase` table.  
Each test is a function returning:

- `TEST_PASS` (0)  
- `TEST_FAIL` (1)

The runner executes all tests sequentially, prints results to the serial console, and provides a final summary.

---

## 📦 Hardware Under Test

| Component | Function | Interface |
|----------|----------|-----------|
| BMI270 | Accelerometer + Gyroscope | I²C (Wire1) |
| BMM150 | Magnetometer | I²C (Wire1) |
| HS3003 | Humidity & Temperature | I²C (Wire1) |
| LPS22DF | Pressure | I²C (Wire1) |
| APDS9960 | Gesture + RGB + Proximity | I²C |
| PDM Mic | Digital microphone | PDM |
| BLE Radio | Wireless communication | BLE 5.0 |

---

## 🧪 Test Descriptions

### **1. Power & USB**
**Purpose:** Ensure the board powers up and USB serial is functional.  
**Method:** Always returns PASS (placeholder for future voltage checks).

---

### **2. LED Blink**
**Purpose:** Verify GPIO and LED driver.  
**Method:** Blink the built‑in LED three times.

---

### **3. IMU (BMI270 + BMM150)**

**Purpose:** Validate the 6‑axis IMU and magnetometer.  
**Checks performed:**

- Device presence on **Wire1**  
- Reading **CHIP_ID** registers  
  - BMI270 → expected `0x24`  
  - BMM150 → expected `0x32`  
- INT1 / INT2 pin states  
- Library initialization (`IMU.begin()`)

**Outcome:**  
PASS only if both sensors respond correctly.

---

### **4. PDM Microphone**

**Purpose:** Confirm microphone and PDM interface.  
**Method:**  
- Start PDM at 16 kHz  
- Wait for data callback  
- PASS if any audio data arrives within 1.5 seconds

---

### **5. Environmental Sensors**

**Purpose:** Check humidity, temperature, and pressure sensors.  
**Method:**  
- Probe HS3003 (`0x44`)  
- Probe LPS22DF (`0x5C`)  
- PASS only if both respond on I²C

---

### **6. APDS9960 Gesture & Color**

**Purpose:** Validate gesture/color sensor.  
**Method:**  
- Initialize APDS  
- Read RGB values  
- PASS if non‑zero color data is detected

---

### **7. BLE Radio**

**Purpose:** Verify BLE stack and advertising.  
**Method:**  
- Start BLE  
- Advertise a custom service for 3 seconds  
- Stop BLE  
- PASS if initialization succeeds

---

### **8. Stress Test**

**Purpose:** Simple delay‑based placeholder for future load tests.

---

### **9. Integration Test**

**Purpose:** Placeholder for combined sensor fusion or multi‑module tests.

---

## ▶️ Running the Tests

1. Upload the sketch to the board.  
2. Open **Serial Monitor** at **115200 baud**.  
3. Reset the board.  
4. Observe output similar to: module tests / summary

```
=== Arduino Nano 33 BLE Sense v2 – TESTY MODULOWE ===

[RUN] 1. Power & USB
[PASS] 1. Power & USB

[RUN] 2. LED Blink
[PASS] 2. LED Blink

=== PODSUMOWANIE ===
Zaliczonych: 9/9
WYNIK PŁYTKI: PASS
```

---

## 📁 File Structure

```
/src
└── main.cpp   # Full test suite implementation
README.md         # This document
```

---

## 🎓 Educational Notes

This project is ideal for:

- Electronics classes  
- Hardware validation labs  
- Manufacturing test jigs  
- Sensor‑fusion research  
- Debugging faulty boards  

It demonstrates:

- I²C probing  
- Register‑level diagnostics  
- BLE service setup  
- PDM audio capture  
- Modular test architecture  

---

## 📜 License

GNU General Public License — free to use, modify, and integrate into your own projects.



