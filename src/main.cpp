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

// Temperature Curves
#define SOLDER_CURVE_LENGTH 4
float solder_curve[SOLDER_CURVE_LENGTH][2] = {
  {10, 140}, // preheat, hold for 10 seconds
  {30, 150}, // soak, hold for 30 seconds
  {60, 200}, // reflow, hold for 60 seconds
  {0, 0} // cooldown
};
#define DESOLDER_CURVE_LENGTH 4
float desolder_curve[DESOLDER_CURVE_LENGTH][2] = {
  {10, 140}, // preheat, hold for 10 seconds
  {30, 150}, // soak, hold for 30 seconds
  {90, 200}, // reflow, hold for 90 seconds
  {0, 0} // cooldown
};

// PID
float Kp = 2;
float Ki = 0.0025;
float Kd = 9;
float PID_Output = 0;
float PID_P, PID_I, PID_D;
float PID_ERROR, PREV_ERROR;
float MIN_PID_VALUE = 0;
float MAX_PID_VALUE = 180;

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
int current_menu = 0; // 0 = main menu, 1 = solder, 2 = desolder, 3 = manual, 4 = cooldown (skippable)
int slctd = 0; // selected menu item
unsigned int millis_before, millis_before_2; // for refresh rate limiting+
unsigned int millis_now = 0;
float refresh_rate = 500; // Oled refresh rate in ms
float pid_refresh_rate  = 50; // PID refresh rate in ms
float seconds = 0; // for time keeping
float pwm_value = 255; // SSR is off with 255
float cooldown_temp = 40; // temperature to cool down to / safe to touch plate
float setpoint = 0; // temperature to reach

// Functions
void mainmenu(int selection);
int input_handler();
void menu_handler();
void PID(double setpoint, double current_temp);
void cooldownmenu(int selection);
void soldermenu(int selection);

void setup() {
  // Setup Thermistor
  thermistor = new NTC_Thermistor(
    SENSOR_PIN,
    REFERENCE_RESISTANCE,
    NOMINAL_RESISTANCE,
    NOMINAL_TEMPERATURE,
    B_VALUE
  );

  // Setup Buttons
  pinMode(but_1, INPUT_PULLUP);
  pinMode(but_2, INPUT_PULLUP);
  pinMode(but_3, INPUT_PULLUP);
  pinMode(but_4, INPUT_PULLUP);

  // Setup SSR
  pinMode(SSR, OUTPUT);
  digitalWrite(SSR, HIGH); // SSR off

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
  // PID
  millis_now = millis();
  if(millis_now - millis_before_2 > pid_refresh_rate)
  {
    millis_before_2 = millis();
    celsius = thermistor->readCelsius();

    switch (current_menu)
    {
      case 1: // Solder
        for (int i = 0; i < SOLDER_CURVE_LENGTH; i++) {
          PID(solder_curve[i][1], celsius);
          if (seconds >= solder_curve[i][0]) {
            break;
          }
        }
        break;
      case 2: // Desolder
        for (int i = 0; i < DESOLDER_CURVE_LENGTH; i++) {
          PID(desolder_curve[i][1], celsius);
          if (seconds >= desolder_curve[i][0]) {
            break;
          }
        }
        break;
      case 3: // Manual
        PID(setpoint, celsius);
        break;
      case 4: // Cooldown
        PID(0, celsius);
        if (celsius <= cooldown_temp) {
          current_menu = 0;
        }
        seconds = 0;
        break;
      default:
        break;
    }
  }

  // OLED Handling
  millis_now = millis();
  if(millis_now - millis_before > refresh_rate) // refresh rate limiting
  {
    millis_before = millis();   
    seconds = seconds + (refresh_rate/1000); // time keeping
    menu_handler();
  }
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
  u8g2.print(pwm_value);
  u8g2.drawStr(65,53,"Temp:");
  u8g2.setCursor(100, 53);
  u8g2.print(celsius);
  u8g2.print("°C");
  u8g2.sendBuffer();
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
      tone(buzzer, 1800, 200); // confirm beep
    }
    else 
    {
      current_menu = 0; // abort / back
      tone(buzzer, 1800, 200); // confirm beep
    }
    break;
  case 3:
    // up
    if (slctd < 2 && current_menu == 0) {
      slctd++;
    }
    if (current_menu == 3) {
      setpoint = setpoint + 5;
    }
    break;
  case 4:
    // down
    if (slctd > 0 && current_menu == 0) {
      slctd--;
    }
    if (current_menu == 3) {
      setpoint = setpoint - 5;
    }
    break;
  case 5:
    // Manual Heating shortcut
    current_menu = 3;
    tone(buzzer, 1800, 200); // confirm beep
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
    soldermenu(slctd);
    break;
  case 2:
    // Desolder
    soldermenu(slctd);
    break;
  case 3:
    // Manual
    soldermenu(slctd);
    break;
  case 4:
    // Cooldown
    cooldownmenu(slctd);
    break;
  default:
    break;
  }
}

void PID(double setpoint, double current_temp)
{       
  //Calculate PID
  PID_ERROR = setpoint - current_temp;
  PID_P = Kp*PID_ERROR;
  PID_I = PID_I+(Ki*PID_ERROR);      
  PID_D = Kd * (PID_ERROR-PREV_ERROR);
  PID_Output = PID_P + PID_I + PID_D;

  //Define maximun PID values
  if(PID_Output > MAX_PID_VALUE){
    PID_Output = MAX_PID_VALUE;
  }
  else if (PID_Output < MIN_PID_VALUE){
    PID_Output = MIN_PID_VALUE;
  }

  //Since the SSR is ON with LOW, we invert the pwm singal
  pwm_value = 255 - PID_Output; 
  analogWrite(SSR,pwm_value); //We change the Duty Cycle applied to the SSR
  
  PREV_ERROR = PID_ERROR;
}

void soldermenu(int selection) {
  u8g2.clearBuffer();
  u8g2.drawStr(5,5,"Target");
  u8g2.setCursor(50, 5);
  u8g2.print(setpoint);
  u8g2.drawStr(5,16,"Current");
  u8g2.setCursor(50, 16);
  u8g2.print(celsius);
  u8g2.print("°C");
  u8g2.drawStr(5,27,"Seconds");
  u8g2.setCursor(50, 27);
  u8g2.print(seconds);
  u8g2.print("s");

  // status
  u8g2.drawStr(5,53,"SSR:");
  u8g2.setCursor(30, 53);
  u8g2.print(pwm_value);
  u8g2.drawStr(65,53,"Temp:");
  u8g2.setCursor(100, 53);
  u8g2.print(celsius);
  u8g2.print("°C");
  u8g2.sendBuffer();
}

void cooldownmenu(int selection) {
  u8g2.clearBuffer();
  u8g2.drawStr(28,26,"Cooling down");

  // status
  u8g2.drawStr(5,53,"SSR:");
  u8g2.setCursor(30, 53);
  u8g2.print(pwm_value);
  u8g2.drawStr(65,53,"Temp:");
  u8g2.setCursor(100, 53);
  u8g2.print(celsius);
  u8g2.print("°C");
  u8g2.sendBuffer();
}