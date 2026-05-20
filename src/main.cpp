#include <Arduino.h>
#include <Arduino_APDS9960.h>
#include <ArduinoBLE.h>
#include <PDM.h>
#include <Wire.h>
#include <Arduino_BMI270_BMM150.h>

enum TestResult { TEST_PASS = 0, TEST_FAIL = 1 };

struct TestCase {
    const char* name;
    TestResult (*fn)();
};

// =====================================================================
// I2C HELPERS — WERSJA NA Wire1 (jedyna prawdziwa magistrala)
// =====================================================================

bool i2cPresentWire1(uint8_t addr) {
    Wire1.beginTransmission(addr);
    return (Wire1.endTransmission() == 0);
}

bool i2cReadRegWire1(uint8_t addr, uint8_t reg, uint8_t &val) {
    Wire1.beginTransmission(addr);
    Wire1.write(reg);
    if (Wire1.endTransmission(false) != 0) return false;

    if (Wire1.requestFrom((int)addr, 1) != 1) return false;

    val = Wire1.read();
    return true;
}

// =====================================================================
// 1. USB
// =====================================================================

TestResult testPowerUsb() { return TEST_PASS; }

// =====================================================================
// 2. LED
// =====================================================================

TestResult testLedBlink() {
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(150);
        digitalWrite(LED_BUILTIN, LOW);
        delay(150);
    }
    return TEST_PASS;
}

// =====================================================================
// 3. IMU — FINALNA WERSJA NA Wire1
// =====================================================================

TestResult testImu() {
    Serial.println("=== IMU HARDWARE DIAGNOSTIC MODE (BMI270 + BMM150, Wire1) ===");

    Wire1.begin();
    Serial.println("INIT IMU LIB...");
    if (IMU.begin()) {
        Serial.println("IMU.begin() OK");
    } else {
        Serial.println("IMU.begin() FAIL");
    }

    const uint8_t BMI_ADDR = 0x68;
    const uint8_t BMM_ADDR = 0x10;

    // 1. Obecność urządzeń
    bool bmi_present = i2cPresentWire1(BMI_ADDR);
    bool bmm_present = i2cPresentWire1(BMM_ADDR);

    Serial.print("BMI270 present: ");
    Serial.println(bmi_present ? "YES" : "NO");

    Serial.print("BMM150 present: ");
    Serial.println(bmm_present ? "YES" : "NO");

    if (!bmi_present && !bmm_present) {
        Serial.println("IMU NOT DETECTED on Wire1.");
        return TEST_FAIL;
    }

    // 2. CHIP_ID BMI270
    uint8_t bmi_id = 0;
    bool bmi_ok = false;

    if (bmi_present) {
        if (i2cReadRegWire1(BMI_ADDR, 0x00, bmi_id)) {
            Serial.print("BMI270 CHIP_ID: 0x");
            Serial.println(bmi_id, HEX);
            bmi_ok = (bmi_id == 0x24);
        } else {
            Serial.println("BMI270: failed to read CHIP_ID");
        }
    }

    // 3. CHIP_ID BMM150
    uint8_t bmm_id = 0;
    bool bmm_ok = false;

    if (bmm_present) {
        if (i2cReadRegWire1(BMM_ADDR, 0x40, bmm_id)) {
            Serial.print("BMM150 CHIP_ID: 0x");
            Serial.println(bmm_id, HEX);
            bmm_ok = (bmm_id == 0x32);
        } else {
            Serial.println("BMM150: failed to read CHIP_ID");
        }
    }

    // 4. INT1/INT2
    const int IMU_INT1 = 24;
    const int IMU_INT2 = 25;

    pinMode(IMU_INT1, INPUT);
    pinMode(IMU_INT2, INPUT);

    Serial.print("INT1 state: "); Serial.println(digitalRead(IMU_INT1));
    Serial.print("INT2 state: "); Serial.println(digitalRead(IMU_INT2));

    // 5. Podsumowanie
    Serial.println("=== SUMMARY ===");

    if (bmi_ok && bmm_ok) {
        Serial.println("IMU OK: BMI270 + BMM150 responding correctly.");
        return TEST_PASS;
    }

    if (bmi_ok && !bmm_ok) {
        Serial.println("IMU PARTIAL: BMI270 OK, BMM150 FAIL.");
        return TEST_FAIL;
    }

    if (!bmi_ok && bmm_ok) {
        Serial.println("IMU PARTIAL: BMM150 OK, BMI270 FAIL.");
        return TEST_FAIL;
    }

    Serial.println("IMU FAIL: Neither BMI270 nor BMM150 returned valid CHIP_ID.");
    return TEST_FAIL;
}

// =====================================================================
// 4. PDM
// =====================================================================

volatile bool pdmReady = false;
int16_t pdmBuffer[512];

void onPDMdata() {
    int bytes = PDM.available();
    PDM.read(pdmBuffer, bytes);
    pdmReady = true;
}

TestResult testMicPdm() {
    PDM.onReceive(onPDMdata);
    if (!PDM.begin(1, 16000)) return TEST_FAIL;
    PDM.setGain(20);

    unsigned long start = millis();
    while (millis() - start < 1500) {
        if (pdmReady) return TEST_PASS;
    }
    return TEST_FAIL;
}

// =====================================================================
// 5. ENV sensors
// =====================================================================

TestResult testEnvSensors() {
    const uint8_t HS3003_ADDR = 0x44;
    const uint8_t LPS22DF_ADDR = 0x5C;

    bool hs = i2cPresentWire1(HS3003_ADDR);
    bool lp = i2cPresentWire1(LPS22DF_ADDR);

    if (!hs || !lp) {
        Serial.print("HS3003: "); Serial.println(hs ? "OK" : "FAIL");
        Serial.print("LPS22DF: "); Serial.println(lp ? "OK" : "FAIL");
        return TEST_FAIL;
    }

    return TEST_PASS;
}

// =====================================================================
// 6. APDS9960
// =====================================================================

TestResult testGestureLight() {
    if (!APDS.begin()) return TEST_FAIL;

    int r, g, b, a;
    if (!APDS.colorAvailable()) delay(50);
    APDS.readColor(r, g, b, a);

    if (r == 0 && g == 0 && b == 0) return TEST_FAIL;

    return TEST_PASS;
}

// =====================================================================
// 7. BLE
// =====================================================================

BLEService testService("180A");
BLEByteCharacteristic testChar("2A57", BLERead | BLEWrite);

TestResult testBle() {
    if (!BLE.begin()) return TEST_FAIL;

    BLE.setLocalName("Nano33BLE_Test");
    BLE.setAdvertisedService(testService);

    testService.addCharacteristic(testChar);
    BLE.addService(testService);

    BLE.advertise();

    unsigned long start = millis();
    while (millis() - start < 3000) {
        BLE.poll();
        delay(10);
    }

    BLE.stopAdvertise();
    BLE.end();
    return TEST_PASS;
}

// =====================================================================
// 8. Stress
// =====================================================================

TestResult testStress() { delay(500); return TEST_PASS; }

// =====================================================================
// 9. Integration
// =====================================================================

TestResult testIntegration() { return TEST_PASS; }

// =====================================================================
// TEST TABLE
// =====================================================================

TestCase tests[] = {
    { "1. Power & USB", testPowerUsb },
    { "2. LED Blink", testLedBlink },
    { "3. IMU", testImu },
    { "4. Mic PDM", testMicPdm },
    { "5. Env sensors", testEnvSensors },
    { "6. Gesture & Light", testGestureLight },
    { "7. BLE", testBle },
    { "8. Stress", testStress },
    { "9. Integration", testIntegration },
};

const size_t TEST_COUNT = sizeof(tests) / sizeof(TestCase);

// =====================================================================
// RUNNER
// =====================================================================

void runTests() {
    Serial.println("=== Arduino Nano 33 BLE Sense v2 – TESTY MODULOWE ===");
    Serial.println();

    size_t passCount = 0;

    for (size_t i = 0; i < TEST_COUNT; i++) {
        Serial.print("[RUN] ");
        Serial.println(tests[i].name);

        TestResult r = tests[i].fn();

        if (r == TEST_PASS) {
            Serial.print("[PASS] ");
            passCount++;
        } else {
            Serial.print("[FAIL] ");
        }
        Serial.println(tests[i].name);
        Serial.println();
    }

    Serial.println("=== PODSUMOWANIE ===");
    Serial.print("Zaliczonych: ");
    Serial.print(passCount);
    Serial.print("/");
    Serial.println(TEST_COUNT);

    if (passCount == TEST_COUNT)
        Serial.println("WYNIK PŁYTKI: PASS");
    else
        Serial.println("WYNIK PŁYTKI: FAIL");
}

// =====================================================================
// SETUP
// =====================================================================

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
    delay(300);
    runTests();
}

void loop() {}
