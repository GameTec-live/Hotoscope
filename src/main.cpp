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
static double celsius = thermistor->readCelsius();

// Buttons
int but_1 = 12; // (top left - Solder Cycle shortcut)
int but_2 = 7; // (bottom left - Manual Heating shortcut / select)
int but_3 = 11; // (top right - up)
int but_4 = 8; // (bottom right - down)

// other
int SSR = 3;
int buzzer = 10;

// Screen
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

// Variables
int ssr_state; // 0 = off, not 0 = on
int current_menu = 0; // 0 = main menu, 1 = solder, 2 = desolder, 3 = manual, 4 = cooldown (skippable)
int slctd = 0; // selected menu item

// Functions
void mainmenu(int selection);
int input_handler();
void menu_handler();

void setup() {
  // Setup Thermistor
  thermistor = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );

  // Setup Buttons+
  pinMode(but_1, INPUT_PULLUP);
  pinMode(but_2, INPUT_PULLUP);
  pinMode(but_3, INPUT_PULLUP);
  pinMode(but_4, INPUT_PULLUP);

  // Setup SSR
  pinMode(SSR, OUTPUT);
  digitalWrite(SSR, HIGH); // SSR off
  ssr_state = 0;

  // Setup Buzzer
  pinMode(buzzer, OUTPUT);

  // Setup Screen
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x13_tf);
  u8g2.enableUTF8Print();
  u8g2.drawStr(32,26,"Hotoscope");
  u8g2.sendBuffer();
  delay(1000);
  tone(buzzer, 1800, 200); // Happy startup beep
}

void loop() {
  celsius = thermistor->readCelsius();
  /*
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
  */
  menu_handler();
  delay(100);
}

void mainmenu(int selection) {
  u8g2.clearBuffer();
  if (selection == 0) {
    u8g2.setDrawColor(0);
  }
  u8g2.drawStr(5,5,"Solder");
  u8g2.setDrawColor(1);
  if (selection == 1) {
    u8g2.setDrawColor(0);
  }
  u8g2.drawStr(5,16,"Desolder");
  u8g2.setDrawColor(1);
  if (selection == 2) {
    u8g2.setDrawColor(0);
  }
  u8g2.drawStr(5,27,"Manual");
  u8g2.setDrawColor(1);

  // status
  u8g2.drawStr(5,53,"SSR:");
  u8g2.setCursor(30, 53);
  u8g2.print(ssr_state);
  u8g2.drawStr(65,53,"Temp:");
  u8g2.setCursor(100, 53);
  u8g2.print(celsius);
  u8g2.print("°C");
}

int input_handler() {
  int input = 0;
  if(digitalRead(but_1) == LOW) {
    input = 1;
  }
  if(digitalRead(but_2) == LOW) {
    input = 2;
  }
  if(digitalRead(but_3) == LOW) {
    input = 3;
  }
  if(digitalRead(but_4) == LOW) {
    input = 4;
  }
  // read in double press of button 2
  if(input == 2) {
    delay(100);
    if(digitalRead(but_2) == LOW) {
      input = 5;
    }
  }
  return input;
}

void menu_handler() {
  int inpt = input_handler();
  switch (inpt)
  {
  case 1:
    // Solder Cycle shortcut
    current_menu = 1;
    break;
  case 2:
    // select
    if (current_menu == 0) {
      switch (slctd)
      {
      case 0:
        // Solder
        current_menu = 1;
        break;
      case 1:
        // Desolder
        current_menu = 2;
        break;
      case 2:
        // Manual
        current_menu = 3;
        break;
      default:
        break;
      }
    }
    break;
  case 3:
    // up
    if (slctd < 2 && current_menu == 0) {
      slctd++;
    }
    break;
  case 4:
    // down
    if (slctd > 0 && current_menu == 0) {
      slctd--;
    }
    break;
  case 5:
    // Manual Heating shortcut
    current_menu = 3;
    break;
  default:
    break;
  }

  switch (current_menu)
  {
  case 0:
    // Main Menu
    mainmenu(slctd);
    break;
  case 1:
    // Solder
    // TODO
    break;
  case 2:
    // Desolder
    // TODO
    break;
  case 3:
    // Manual
    // TODO
    break;
  case 4:
    // Cooldown
    // TODO
    break;
  default:
    break;
  }
}