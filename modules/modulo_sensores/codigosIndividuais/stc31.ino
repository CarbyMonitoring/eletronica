#include <Arduino.h>
#include <SensirionI2CStc3x.h>
#include <Wire.h>
#include <inttypes.h>

SensirionI2CStc3x stc3x;

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    Wire.begin();

    uint16_t error;
    char errorMessage[256];

    stc3x.begin(Wire);

    error = stc3x.prepareProductIdentifier();
    if (error) {
        Serial.print("Error trying to execute prepareProductIdentifier(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        uint32_t productNumber;
        uint8_t serialNumberRaw[32];
        uint8_t serialNumberSize = 32;
        error = stc3x.readProductIdentifier(productNumber, serialNumberRaw,
                                            serialNumberSize);
        if (error) {
            Serial.print("Error trying to execute readProductIdentifier(): ");
            errorToString(error, errorMessage, 256);
            Serial.println(errorMessage);
        } else {
            uint64_t serialNumber = (uint64_t)serialNumberRaw[0] << 56 |
                                    (uint64_t)serialNumberRaw[1] << 48 |
                                    (uint64_t)serialNumberRaw[2] << 40 |
                                    (uint64_t)serialNumberRaw[3] << 32 |
                                    (uint64_t)serialNumberRaw[4] << 24 |
                                    (uint64_t)serialNumberRaw[5] << 16 |
                                    (uint64_t)serialNumberRaw[6] << 8 |
                                    (uint64_t)serialNumberRaw[7];
            Serial.print("ProductNumber: 0x");
            Serial.println(productNumber, HEX);
            char buffer[20];
            sprintf(buffer, "SerialNumber: %llu", serialNumber);
            Serial.println(buffer);
        }
    }

    uint16_t selfTestOutput;
    error = stc3x.selfTest(selfTestOutput);
    if (error) {
        Serial.print("Error trying to execute selfTest(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        char buffer[32];
        sprintf(buffer, "Self Test: 0x%04x (OK = 0x0000)", selfTestOutput);
        Serial.println(buffer);
    }

    error = stc3x.setBinaryGas(0x0001);
    if (error) {
        Serial.print("Error trying to execute selfTest(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.println("Set binary gas to 0x0001");
    }
}

void loop() {
    delay(1000);

    // Measure
    uint16_t gasTicks;
    uint16_t temperatureTicks;

    uint16_t error;
    char errorMessage[256];

    error = stc3x.measureGasConcentration(gasTicks, temperatureTicks);
    if (error) {
        Serial.print("Error trying to execute measureGasConcentration(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.print("GasConcentration:");
        Serial.print(100 * ((float)gasTicks - 16384.0) / 32768.0);
        Serial.print("\t");
        Serial.print("Temperature:");
        Serial.println((float)temperatureTicks / 200.0);
    }
}