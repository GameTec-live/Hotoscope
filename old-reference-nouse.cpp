/*//             
 * notes for don: 2 modes. 1 solder mode , where it follows to solderpastes temp curve, 2 desolder mode, where it 
 * mainiains a desired tempredures .(mode decide at start up) 1 button for each.  buttons 3 and 4 are temp controls for mode 2.
 * id like to add some more beeps. during stage changes (beep beep look at me im about to reflow.) we are using the 
 * 100K Value B NTC Temperature Sensor 3950 Thermistor i duno if this code is for the 100k or 10k version im going to change the 
 * 1602 lcd screen for a 0.9'' i2c oled. ssd1306 ( ibrary - u8g2 by oliver)
 * 
 * ---Have in mind---
 *This code is a first version for the reflow hot plate project.
 *Tutorial here: http://electronoobs.com/eng_arduino_tut161.php
 *Please be careful working with "High Voltage" and double check everything. 
 *Only use this project with supervision, never leave it plugged.
 *You can use this code at your own risk. I don't offer any guarantee that you 
 *will get the same results as I did and you might have to adjsut some PID values */

#include <thermistor.h>             //Download it here: http://electronoobs.com/eng_arduino_thermistor.php
thermistor therm1(A0,0);            //The 3950 Thermistor conencted on A0  2.2k Resistor on devider

//LCD config
#include <Wire.h> 
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI 

         



//Inputs and Outputs
int but_1 = 12; //(top left - Solder)
int but_2 = 7; //(bottom left desolder_
int but_3 = 11; //(top right  heat up)
int but_4 = 8; //( bottom right heat down)
int SSR = 3;
int buzzer = 10;
int Thermistor_PIN = A0;

//Variables
unsigned int millis_before, millis_before_2;    //We use these to create the loop refresh rate
unsigned int millis_now = 0;
float refresh_rate = 500;                       //LCD refresh rate. You can change this if you want
float pid_refresh_rate  = 50;                   //PID Refresh rate
float seconds = 0;                              //Variable used to store the elapsed time                   
int running_mode = 0;                           //We store the running selected mode here
int selected_mode = 0;                          //Selected mode for the menu
int max_modes = 3;                              //For now, we only work with 1 mode...
bool but_3_state = true;                        //Store the state of the button (HIGH OR LOW)
bool but_4_state  =true;                        //Store the state of the button (HIGH OR LOW)
float temperature = 0;                          //Store the temperature value here
float preheat_setoint = 140;                    //Mode 1 preheat ramp value is 140-150ºC
float soak_setoint = 150;                       //Mode 1 soak is 150ºC for a few seconds
float reflow_setpoint = 200;                    //Mode 1 reflow peak is 200ºC
float temp_setpoint = 0;                        //Used for PID control
float pwm_value = 255;                          //The SSR is OFF with HIGH, so 255 PWM would turn OFF the SSR
float MIN_PID_VALUE = 0;
float MAX_PID_VALUE = 180;                      //Max PID value. You can change this. 
float cooldown_temp = 40;                       //When is ok to touch the plate

/////////////////////PID VARIABLES///////////////////////
/////////////////////////////////////////////////////////
float Kp = 2;               //Mine was 2
float Ki = 0.0025;          //Mine was 0.0025
float Kd = 9;               //Mine was 9
float PID_Output = 0;
float PID_P, PID_I, PID_D;
float PID_ERROR, PREV_ERROR;
/////////////////////////////////////////////////////////

void setup() {
  //Define the pins as outputs or inputs
  pinMode(SSR, OUTPUT);
  digitalWrite(SSR, HIGH);        //Make sure we start with the SSR OFF (is off with HIGH)
  pinMode(buzzer, OUTPUT); 
  digitalWrite(buzzer, LOW);  
  pinMode(but_1, INPUT_PULLUP);
  pinMode(but_2, INPUT_PULLUP);
  pinMode(but_3, INPUT_PULLUP);
  pinMode(but_4, INPUT_PULLUP);
  pinMode(Thermistor_PIN, INPUT);

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  u8g.setFont(u8g_font_unifont); // set font / init OLED
  u8g.setRot180();

  u8g.firstPage();
  u8g.drawStr(0, 35, "Hotoscope");
  u8g.sendBuffer();

  delay(500);

  //Serial.begin(9600);
  tone(buzzer, 1800, 200);     
  millis_before = millis();
  millis_now = millis();
}

void loop() {
  millis_now = millis();
  if(millis_now - millis_before_2 > pid_refresh_rate){    //Refresh rate of the PID
    millis_before_2 = millis(); 
    
    temperature = therm1.analog2temp();
    
    if(running_mode == 1){   
      if(temperature < preheat_setoint){
        temp_setpoint = seconds*1.666;                    //Reach 150ºC till 90s (150/90=1.666)
      }  
        
      if(temperature > preheat_setoint && seconds < 90){
        temp_setpoint = soak_setoint;               
      }   
        
      else if(seconds > 90 && seconds < 110){
        temp_setpoint = reflow_setpoint;                 
      } 
       
      //Calculate PID
      PID_ERROR = temp_setpoint - temperature;
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
      
      analogWrite(SSR,pwm_value);           //We change the Duty Cycle applied to the SSR
      
      PREV_ERROR = PID_ERROR;
      
      if(seconds > 130){
        digitalWrite(SSR, HIGH);            //With HIGH the SSR is OFF
        temp_setpoint = 0;
        running_mode = 10;                  //Cooldown mode        
      }     
    }//End of running_mode = 1


    //Mode 10 is between reflow and cooldown
    if(running_mode == 10){
      u8g.firstPage();
      u8g.drawStr( 0, 22, "Complete");
      u8g.sendBuffer();
      
      tone(buzzer, 1800, 1000);    
      seconds = 0;              //Reset timer
      running_mode = 11;
      delay(3000);  
    }    
  }//End of > millis_before_2 (Refresh rate of the PID code)
  

  
  millis_now = millis();
  if(millis_now - millis_before > refresh_rate){          //Refresh rate of prntiong on the LCD
    millis_before = millis();   
    seconds = seconds + (refresh_rate/1000);              //We count time in seconds
    

    //Mode 0 is with SSR OFF (we can selcet mode with buttons)
    if(running_mode == 0){ 
      digitalWrite(SSR, HIGH);        //With HIGH the SSR is OFF
      u8g.firstPage();
      u8g.drawStr( 0, 15, "T: ");
      u8g.setPrintPos(75, 15);
      u8g.print(temperature, 0);   
      u8g.drawStr(0, 35, "SSR OFF");      
        
      if(selected_mode == 0){
        u8g.drawStr( 0, 15, "Select Mode");     
      }
      else if(selected_mode == 1){
        u8g.drawStr( 0, 15, "MODE 1");     
      }
      else if(selected_mode == 2){
        u8g.drawStr( 0, 15, "MODE 2");     
      }
      else if(selected_mode == 3){
        u8g.drawStr( 0, 15, "MODE 3");     
      }
      u8g.sendBuffer(); 
    }//End of running_mode = 0

     //Mode 11 is cooldown. SSR is OFF
     else if(running_mode == 11){ 
      if(temperature < cooldown_temp){
        running_mode = 0; 
        tone(buzzer, 1000, 100); 
      }
      digitalWrite(SSR, HIGH);        //With HIGH the SSR is OFF 
      u8g.firstPage();
      u8g.drawStr( 0, 15, "T: ");
      u8g.setPrintPos(75, 15);
      u8g.print(temperature, 0);   
      u8g.drawStr(0, 35, "SSR OFF");       
      u8g.drawStr( 0, 75, "Cooling down");
      u8g.sendBuffer();
    }//end of running_mode == 11

    //Mode 1 is the PID runnind with selected mode 1
    else if(running_mode == 1){ 
      u8g.firstPage();           
      u8g.drawStr( 0, 15, "T: ");
      u8g.setPrintPos(75, 15);
      u8g.print(temperature, 0);   
      u8g.drawStr(0, 35, "SSR ON"); 
        
      u8g.drawStr( 0, 15, "S"); u8g.setPrintPos(75, 15); u8g.print(temp_setpoint,1);     
      u8g.drawStr( 0, 35, "PWM"); u8g.setPrintPos(75, 35); u8g.print(pwm_value,1);
      u8g.setPrintPos(0, 55); u8g.print(seconds,0); u8g.print("s");
      u8g.sendBuffer(); 
    }//End of running_mode == 1
  }


  
  ///////////////////////Button detection////////////////////////////
  ///////////////////////////////////////////////////////////////////
  if(!digitalRead(but_3) && but_3_state){
    but_3_state = false;
    selected_mode ++;   
    tone(buzzer, 2300, 40);  
    if(selected_mode > max_modes){
      selected_mode = 0;
    }
    delay(150);
  }
  else if(digitalRead(but_3) && !but_3_state){
    but_3_state = true;
  }

  
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  if(!digitalRead(but_4) && but_4_state){
    if(running_mode == 1){
      digitalWrite(SSR, HIGH);        //With HIGH the SSR is OFF
      running_mode = 0;
      selected_mode = 0; 
      tone(buzzer, 2500, 150);
      delay(130);
      tone(buzzer, 2200, 150);
      delay(130);
      tone(buzzer, 2000, 150);
      delay(130);
    }
    
    but_4_state = false;
    if(selected_mode == 0){
      running_mode = 0;
    }
    else if(selected_mode == 1){
      running_mode = 1;
      tone(buzzer, 2000, 150);
      delay(130);
      tone(buzzer, 2200, 150);
      delay(130);
      tone(buzzer, 2400, 150);
      delay(130);
      seconds = 0;                    //Reset timer
    }
  }
  else if(digitalRead(but_4) && !but_4_state){
    but_4_state = true;
  }
}
