#include <Arduino.h>

// Thermistor
#include <Thermistor.h>
#include <NTC_Thermistor.h>

#define SENSOR_PIN             A0
#define REFERENCE_RESISTANCE   2200
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950
Thermistor* thermistor;


// Screen
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

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
  u8g2.setFont(u8g2_font_6x13_tf);
  u8g2.enableUTF8Print();
  u8g2.drawStr(0,35,"Hello World!");
  u8g2.sendBuffer();
  delay(1000);
}

void loop() {
  const double celsius = thermistor->readCelsius();
  static double lastCelsius = 0;

  if(lastCelsius!=celsius) {  
    // send to screen
    u8g2.clearBuffer();
    u8g2.drawStr(0,35,"Temp: ");
    u8g2.setCursor(50, 35);
    u8g2.print(celsius);
    u8g2.print("°C");
    u8g2.sendBuffer();
    lastCelsius=celsius;
  }
}