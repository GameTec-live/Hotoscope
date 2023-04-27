#include <Arduino.h>
#include <QuickPID.h>
#include <sTune.h>

// Thermistor
#include <Thermistor.h>
#include <NTC_Thermistor.h>

#define SENSOR_PIN             A0
#define REFERENCE_RESISTANCE   2200
#define NOMINAL_RESISTANCE     100000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3950
Thermistor* thermistor;
static float celsius = 0;

// Temperature Curves
#define SOLDER_CURVE_LENGTH 4
const float solder_curve[SOLDER_CURVE_LENGTH][2] PROGMEM = {
  {10, 140}, // preheat, hold for 10 seconds
  {30, 150}, // soak, hold for 30 seconds
  {60, 200}, // reflow, hold for 60 seconds
  {0, 0} // cooldown
};
#define DESOLDER_CURVE_LENGTH 4
const float desolder_curve[DESOLDER_CURVE_LENGTH][2] PROGMEM = {
  {10, 140}, // preheat, hold for 10 seconds
  {30, 150}, // soak, hold for 30 seconds
  {90, 200}, // reflow, hold for 90 seconds
  {0, 0} // cooldown
};

// PID
#define TEMPLIMIT 400
uint32_t testTimeSec = 1;  // runPid interval = testTimeSec / samples
const uint16_t samples PROGMEM = 1;
const float outputSpan PROGMEM = 1000; // window size
float Kp = 2.00;
float Ki = 0.02;
float Kd = 0.06;
float PID_Output = 0;

// Buttons
#define but_1 0 // (top left - Solder Cycle shortcut)
#define but_2 2 // (bottom left - Manual Heating shortcut / select)
#define but_3 13 // (top right - up)
#define but_4 8 // (bottom right - down)

// other
#define SSR 14
#define buzzer 12

// Screen
#include <Wire.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

// Variables
int current_menu = 0; // 0 = main menu, 1 = solder, 2 = desolder, 3 = manual, 4 = cooldown (skippable)
int slctd = 0; // selected menu item
unsigned int millis_before, millis_before_2; // for refresh rate limiting+
unsigned int millis_now = 0;
float refresh_rate = 60; // Oled refresh rate in ms
float pid_refresh_rate  = 50; // PID refresh rate in ms
float seconds = 0; // for time keeping
float pwm_value = 255; // SSR is off with 255
float cooldown_temp = 40; // temperature to cool down to / safe to touch plate
float setpoint = 0; // temperature to reach

sTune tuner = sTune();
QuickPID myPID(&celsius, &PID_Output, &setpoint);

// Functions
void mainmenu();
int input_handler();
void menu_handler();
void PID();
void cooldownmenu();
void soldermenu();
void add_status_to_display_framebuffer();

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

  // Serial Debug
  Serial.begin(9600);
  Serial.println("Hi");

  // Setup SSR
  pinMode(SSR, OUTPUT);
  digitalWrite(SSR, HIGH); // SSR off

  // Setup Buzzer
  pinMode(buzzer, OUTPUT);

  // Setup Screen
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.enableUTF8Print();

  // Startup Screen
  u8g2.clearBuffer();
  u8g2.drawStr(32,26,"Hotoscope");
  u8g2.sendBuffer();
  delay(1000);
  tone(buzzer, 1800, 200); // Happy startup beep

  // Init PID
  celsius = thermistor->readCelsius();
  tuner.Configure(0, 0, 0, 0, testTimeSec, 0, samples);
  tuner.SetEmergencyStop(TEMPLIMIT);
  myPID.SetOutputLimits(0, outputSpan * 0.1);
  myPID.SetSampleTimeUs((outputSpan - 1) * 1000);
  myPID.SetMode(myPID.Control::automatic); // the PID is turned on
  myPID.SetProportionalMode(myPID.pMode::pOnMeas);
  myPID.SetAntiWindupMode(myPID.iAwMode::iAwClamp);
  myPID.SetTunings(Kp, Ki, Kd); // set PID gains
}

void loop() {
  // PID
  celsius = thermistor->readCelsius();
  millis_now = millis();
  if(millis_now - millis_before_2 > pid_refresh_rate)
  {
    millis_before_2 = millis();
    switch (current_menu)
    {
      case 1: // Solder
        for (int i = 0; i < SOLDER_CURVE_LENGTH; i++) {
          setpoint = solder_curve[i][1];
          if (seconds >= solder_curve[i][0]) {
            break;
          }
        }
        break;
      case 2: // Desolder
        for (int i = 0; i < DESOLDER_CURVE_LENGTH; i++) {
          setpoint = desolder_curve[i][1];
          if (seconds >= desolder_curve[i][0]) {
            break;
          }
        }
        break;
      case 3: // Manual
        break;
      case 4: // Cooldown
        setpoint = 0;
        if (celsius <= cooldown_temp) {
          current_menu = 0;
        }
        seconds = 0;
        break;
      default:
        break;
    }
  }
  PID();
  
  // OLED Handling
  millis_now = millis();
  if(millis_now - millis_before > refresh_rate) // refresh rate limiting
  {
    millis_before = millis();   
    seconds = seconds + (refresh_rate/1000); // time keeping
    menu_handler();
  }
}

void mainmenu() {
  u8g2.clearBuffer();
  //Solder
  u8g2.setCursor(5,10);
  u8g2.print(F("Solder"));
  if (slctd == 0) u8g2.print(F(" <-"));
  
  //Desolder
  u8g2.setCursor(5,21);
  u8g2.print(F("Desolder"));
  if (slctd == 1) u8g2.print(F(" <-"));
  
  //Manual
  u8g2.setCursor(5,32);
  u8g2.print(F("Manual"));
  if (slctd == 2) u8g2.print(F(" <-"));

  //Chillax
  u8g2.setCursor(5,43);
  u8g2.print(F("Cooldown"));
  if (slctd == 3) u8g2.print(F(" <-"));

  // status
  add_status_to_display_framebuffer();
  u8g2.sendBuffer();
}

int input_handler() {
  if(digitalRead(but_1) == LOW) return 1;
  if(digitalRead(but_2) == LOW) {
    delay(500);
    if(digitalRead(but_2) == LOW) {
      tone(buzzer, 1800, 200);
      return 5;
    }
    return 2;
  }
  if(digitalRead(but_3) == LOW) return 3;
  if(digitalRead(but_4) == LOW) return 4;
  return 0;
}

void menu_handler() {
  int inpt = input_handler();
  switch (inpt)
  {
  case 1: // Solder Cycle shortcut
    current_menu = 1;
    break;
  case 2: // Select
    if (current_menu == 0) {
      /*
      switch (slctd)
      {
      case 0: // Solder
        current_menu = 1;
        break;
      case 1: // Desolder
        current_menu = 2;
        break;
      case 2: // Manual
        current_menu = 3;
        break;
      case 3: // Cooldown
        current_menu = 4;
        break;
      default:
        break;
      }
      */
      current_menu = slctd+1;
      tone(buzzer, 1800, 200); // confirm beep
    }
    else 
    {
      current_menu = 0; // abort / back
      tone(buzzer, 1800, 200); // confirm beep
    }
    break;
  case 3: // up
    if (slctd < 2 && current_menu == 0) slctd++;
    if (current_menu == 3) setpoint = setpoint - 5;
    break;
  case 4: // down
    if (slctd > 0 && current_menu == 0) slctd--;
    if (current_menu == 3) setpoint = setpoint + 5;
    break;
  case 5: // Manual Heating shortcut
    current_menu = 3;
    tone(buzzer, 1800, 200); // confirm beep
    break;
  default:
    break;
  }

  switch (current_menu)
  {
  case 0: // Main Menu
    mainmenu();
    break;
  case 1: // Solder
    soldermenu();
    break;
  case 2: // Desolder
    soldermenu();
    break;
  case 3: // Manual
    soldermenu();
    break;
  case 4: // Cooldown
    cooldownmenu();
    break;
  default:
    break;
  }
}

void PID()
{ 
  //float optimumOutput = tuner.softPwm(SSR, celsius, PID_Output, setpoint, outputSpan, 0); // ssr mode
  tuner.softPwm(SSR, celsius, PID_Output, setpoint, outputSpan, 0); // ssr mode
  //if (myPID.Compute()) {
    //tuner.plotter(celsius, optimumOutput, setpoint, 0.5f, 3); // output scale 0.5, plot every 3rd sample
  //}

  //Since the SSR is ON with LOW, we invert the pwm singal
  pwm_value = 255 - PID_Output; 
  analogWrite(SSR,pwm_value); //We change the Duty Cycle applied to the SSR
}

void soldermenu() {
  u8g2.clearBuffer();
  u8g2.setCursor(5,10);
  u8g2.print(F("Target"));
  u8g2.setCursor(50, 10);
  u8g2.print(setpoint);
  u8g2.setCursor(5,21);
  u8g2.print(F("Current"));
  u8g2.setCursor(50, 21);
  u8g2.print(celsius);
  u8g2.print("C");
  u8g2.setCursor(5,32);
  u8g2.print(F("Seconds"));
  u8g2.setCursor(50, 32);
  u8g2.print(seconds);
  u8g2.print("s");

  add_status_to_display_framebuffer();
  u8g2.sendBuffer();
}

void cooldownmenu() {
  u8g2.clearBuffer();
  u8g2.setCursor(28,26);
  u8g2.print(F("Cooling down"));

  add_status_to_display_framebuffer();
  u8g2.sendBuffer();
}

void add_status_to_display_framebuffer(){
  u8g2.setCursor(5,53);
  u8g2.print(F("SSR:"));
  u8g2.setCursor(30, 53);
  u8g2.print((int)pwm_value);
  u8g2.setCursor(65,53);
  u8g2.print(F("Temp:"));
  u8g2.setCursor(100, 53);
  u8g2.print((int)celsius);
  u8g2.print("C");
}