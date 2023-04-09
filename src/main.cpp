#include <Arduino.h>

// Thermistor
#include <Thermistor.h>
#include <NTC_Thermistor.h>

#define SENSOR_PIN             A0
#define REFERENCE_RESISTANCE   2000
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950
Thermistor* thermistor;


// Screen
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  // Setup Thermistor
  thermistor = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );

  // Setup Screen
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.firstPage();
  u8g2.drawStr(0,10,"Hello World!");
  u8g2.sendBuffer();
  delay(1000);
}

void loop() {
  const double celsius = thermistor->readCelsius();
  u8g2.firstPage();
  u8g2.drawStr(0,20,"Celsius: ");
  u8g2.drawStr(0,30, String(celsius).c_str());
  u8g2.sendBuffer();
  delay(1000);
}