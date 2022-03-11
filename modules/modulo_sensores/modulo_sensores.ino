#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SensirionI2CStc3x.h>
#include <SensirionI2CSfa3x.h>
#include <inttypes.h>
#include "MQ131.h"
#include <MiCS6814-I2C.h>
#include "DGS.h"
#include "Adafruit_CCS811.h"

const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

const char* mqtt_server = "YOUR_MQTT_BROKER_IP_ADDRESS";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

HardwareSerial Serial1(2);

SensirionI2CStc3x stc3x;
SensirionI2CSfa3x sfa3x;
MiCS6814 mics;
bool sensorConnected;
Adafruit_CCS811 ccs;
DGS mySensor(&Serial1);
const char* Barcode = "042617040460 110102 CO 1705 2.72";

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Wire.begin();
  setup_stc31();
  setup_sfa30();
  setup_mq131();
  setup_mics_6814();
  setup_dgs();
  setup_ccs_811();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup_stc31(){
  stc3x.begin(Wire);

  if(stc3x.prepareProductIdentifier()){
    client.publish("esp32/error", 'Error in stc31 prepareProductIdentifier');
  }
  if(stc3x.selfTest()){
    client.publish("esp32/error", 'Error in stc31 selfTest');
  }
  if(stc3x.prepareProductIdentifier()){
    client.publish("esp32/error", 'Error in stc31 prepareProductIdentifier');
  }
}

void setup_sfa30() {
  sfa3x.begin(Wire);

  if(sfa3x.startContinuousMeasurement()){
    client.publish("esp32/error", 'Error in sfa30 startContinuousMeasurement');
  }
}

void setup_mq131(){
  MQ131.begin(2,A0, LOW_CONCENTRATION, 1000000);  
  MQ131.calibrate();
}

void setup_mics_6814(){
  sensorConnected = mics.begin();

  if (sensorConnected == true) {
    mics.powerOn();
  } else {
    client.publish("esp32/error", 'MiCS-6814 not connected');
  }
}

void setup_dgs() {
  Serial1.begin(9600);
  Serial1.flush();
  mySensor.setBC(Barcode);
  mySensor.setToff(1.03);
  mySensor.getLMP();
  mySensor.setLMP(mySensor.LMP[0], mySensor.LMP[1], mySensor.LMP[2]);
  mySensor.getEEPROM();
}

void setup_ccs_811() {
  if(!ccs.begin()){
    client.publish("esp32/error", 'Failed to initializa CCS_811');
  }
  while(!ccs.available());
}

void reconnect() {
  while (!client.connected()) {
    client.connect("ESP8266Client");
    delay(5000);
  }
}

void read_stc_31(){
  uint16_t gasTicks;
  uint16_t temperatureTicks;
  uint16_t error;
  
  error = stc3x.measureGasConcentration(gasTicks, temperatureTicks);
  if(!error) {
    char msg[50];
    sprintf(msg, "%f", 100 * ((float)gasTicks - 16384.0) / 32768.0);
    client.publish("esp32/co2", msg);
    sprintf(msg, "%f", (float)temperatureTicks / 200.0);
    client.publish("esp32/temperature", msg);
  }
}

void read_sfa_30(){
  uint16_t error;
  int16_t hcho;
  int16_t humidity;
  int16_t temperature;

  error = sfa3x.readMeasuredValues(hcho, humidity, temperature);
  if(!error) {
    char msg[50];
    sprintf(msg, "%f", hcho / 5.0;
    client.publish("esp32/hcho", msg);
    sprintf(msg, "%f", humidity / 100.0);
    client.publish("esp32/humidity", msg);
  }
}

void read_mq_131(){
  MQ131.sample();
  client.publish("esp32/O3", MQ131.getO3(PPM));
}

void read_mics_6814() {
  if (sensorConnected) {
    client.publish("esp32/CO", mics.measureCO());
    client.publish("esp32/NO2", mics.measureNO2());
  }
}

void read_dgs() {
  char msg[50];
  sprintf(msg, "%f", mySensor.getConc());
  client.publish("esp32/SO2", msg);
}

void read_ccs_811() {
  if(ccs.available()){
    if(!ccs.readData()){
      client.publish("esp32/COV", ccs.getTVOC());
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  read_stc_31();
  read_sfa_30();
  read_mq_131();
  read_mics_6814();
  read_dgs();
  read_ccs_811();
}