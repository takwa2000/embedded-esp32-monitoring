// Capteur AHT10
// Library I2C
#include <Wire.h>
#include "DigiKeyboard.h"

#define Address_AHT10 0x38 // the RW bit will be added automatically

#define AHT10_SOFT_RESET_CMD 0xBA // soft reset command
#define AHT10_SOFT_RESET_DELAY 20 // less than 20 ms

#define AHT10_INIT_CMD 0xE1 // initialization command
#define AHT10_TRIGGER_MEASURMENT_CMD 0xAC // trigger measurement command
#define AHT10_CMD_DELAY 350 // 350ms

#define AHT10_DATA0 0x33 // see trigger measurement data
#define AHT10_DATA1 0x00 // see trigger measurement data
#define AHT10_MEASUREMENT_DELAY 80 // at least 75 milliseconds

byte DataBuffer[6];

void setup() {
  DigiKeyboard.delay(2000);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(1000);
  DigiKeyboard.println("notepad");
  DigiKeyboard.delay(1000);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
  DigiKeyboard.delay(2000);
  // initialization of Serial + I2C
  Serial.begin(115200); // Init SerialUSB for debugging
  Wire.begin(); // start the I2C protocol
  delay(AHT10_SOFT_RESET_DELAY);

  // Soft reset
  Wire.beginTransmission(Address_AHT10); // 0x38
  Wire.write(AHT10_SOFT_RESET_CMD);
  Wire.endTransmission();
  delay(AHT10_SOFT_RESET_DELAY); // minimum 20ms

  // Init measurements
  Wire.beginTransmission(Address_AHT10); // 0x38
  Wire.write(AHT10_INIT_CMD); // 0xE1
  Wire.write(AHT10_TRIGGER_MEASURMENT_CMD); // 0xAC
  Wire.endTransmission();
  delay(AHT10_CMD_DELAY);
}

void loop() {
  // Start the AHT10 sensor measurement
  Wire.beginTransmission(Address_AHT10); // 0x38
  Wire.write(AHT10_TRIGGER_MEASURMENT_CMD); // 0xAC
  Wire.write(AHT10_DATA0); // 0x33
  Wire.write(AHT10_DATA1); // 0x00
  Wire.endTransmission();
  delay(AHT10_MEASUREMENT_DELAY);

  // Read the AHT10 sensor
  Wire.requestFrom(Address_AHT10, 6); // read measurements
  while (Wire.available()) {
    DataBuffer[0] = Wire.read(); // receive the 1st byte: STATUS
    DataBuffer[1] = Wire.read();
    DataBuffer[2] = Wire.read();
    DataBuffer[3] = Wire.read();
    DataBuffer[4] = Wire.read();
    DataBuffer[5] = Wire.read();
  }

  // Display the measurements
  uint32_t Raw_humidity = (((uint32_t)DataBuffer[1] << 16) | ((uint16_t)DataBuffer[2] << 8 | (DataBuffer[3])) << 4); // 20-bit raw humidity
  int humidity = Raw_humidity * 0.000095; // integer part
  if (humidity < 1) humidity = 0;
  if (humidity > 99) humidity = 100;
  //Serial.print("Humidité = ");
  //Serial.print(humidity);
  //Serial.println("% précision 2%");
  DigiKeyboard.print("humidity:");
  DigiKeyboard.println(humidity); 

  uint32_t Raw_temperature = (((uint32_t)(DataBuffer[3] & 0x0F) << 16) | ((uint16_t)DataBuffer[4] << 8 | (DataBuffer[5]))); // raw temperature
  float temperature = Raw_temperature * 0.000191 - 50; // temperature data in °C
  int IntegerPart = int(temperature);
  int DecimalPart = int((temperature - IntegerPart) * 10); // 1 digit after the decimal point
  //Serial.print("Température = ");
  //Serial.print(IntegerPart);
  //Serial.print(".");
  //Serial.print(DecimalPart);
  //Serial.print("°C précision 0.3°C");
  //delay(500);
  DigiKeyboard.print("temperature:");
  DigiKeyboard.println(IntegerPart);

  DigiKeyboard.delay(200);
  DigiKeyboard.sendKeyStroke(KEY_ENTER);
}