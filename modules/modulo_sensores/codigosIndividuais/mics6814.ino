#include <MiCS6814-I2C.h>

MiCS6814 mics;
bool sensorConnected;

void setup() {
  // Initialize serial connection
  Serial.begin(115200);

  // Connect to sensor using default I2C address (0x04)
  // Alternatively the address can be passed to begin(addr)
  sensorConnected = mics.begin();

  if (sensorConnected == true) {
    // Print status message
    Serial.println("Connected to MiCS-6814 sensor");

    // Turn heater element on
    mics.powerOn();
    
    // Print header for live values
    Serial.println("Current concentrations:");
    Serial.println("CO\tNO2\tNH3\tC3H8\tC4H10\tCH4\tH2\tC2H5OH");
  } else {
    // Print error message on failed connection
    Serial.println("Couldn't connect to MiCS-6814 sensor");
  }
}

void loop() {
  if (sensorConnected) {
    // Print live values
    Serial.print(mics.measureCO());
    Serial.print("\t");
    Serial.print(mics.measureNO2());
    Serial.print("\t");
    Serial.print(mics.measureNH3());
    Serial.print("\t");
    Serial.print(mics.measureC3H8());
    Serial.print("\t");
    Serial.print(mics.measureC4H10());
    Serial.print("\t");
    Serial.print(mics.measureCH4());
    Serial.print("\t");
    Serial.print(mics.measureH2());
    Serial.print("\t");
    Serial.println(mics.measureC2H5OH());
  }

  // Wait a small amount of time
  delay(1000);
}
