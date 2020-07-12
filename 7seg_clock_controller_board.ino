#include "Adafruit_Si7021.h"
#include <FastLED.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
//#include <MCP7940.h>
#include "RTClib.h"

#define BAUD_RATE 9600
#define yes 2
#define no 1
#define NUM_LEDS 30
#define DATA_PIN 4

#define degree 11
#define unit_f 12

#define r_time_loc 0
#define g_time_loc 1
#define b_time_loc 2

#define r_temp_loc 3
#define g_temp_loc 4
#define b_temp_loc 5

#define temp_int_loc 6
#define temp_per_loc 7

#define night_light_loc 8
#define night_bright_loc 9

#define ext_tx_pin 8 // connect to external UART rx pin
#define ext_rx_pin 9 // connect to external UART tx pin

//MCP7940_Class rtc;
RTC_PCF8523 rtc;
Adafruit_Si7021 sensor = Adafruit_Si7021();

CRGB leds[NUM_LEDS];

SoftwareSerial ext_serial(ext_rx_pin, ext_tx_pin);

int LED_ON = 0x00ff00; // Note that Red and Green are swapped
int LED_OFF = 0x000000;

int digit_set[13][7] = {
                          {yes, yes, yes, no, yes, yes, yes},
                          {yes, no, no, no, yes, no, no},
                          {yes, yes, no, yes, no, yes, yes},
                          {yes, yes, no, yes, yes, yes, no},
                          {yes, no, yes, yes, yes, no, no},
                          {no, yes, yes, yes, yes, yes, no},
                          {no, yes, yes, yes, yes, yes, yes},
                          {yes, yes, no, no, yes, no, no},
                          {yes, yes, yes, yes, yes, yes, yes},
                          {yes, yes, yes, yes, yes, no, no},
                          {no, no, no, no, no, no, no},
                          {yes, yes, yes, yes, no, no, no},
                          {no, yes, yes, yes, no, no, yes}
};

unsigned long time_1;
unsigned long time_2;
unsigned long time_3;

int dst_count = 0;
int dst_offset = 0;
bool dst_set = false;

int _month_ = 0;
int _day_ = 0;
int _year_ = 0;
int _hour_ = 0;
int _min_ = 0;
int _second_ = 0;
int _WeekDay_ = 0;

int hour_ones = 0;
int hour_tens = 0;
int min_ones = 0;
int min_tens = 0;
int sec_ones = 0;
int sec_tens = 0;

int temp_tens;
int temp_ones;
int humid_tens;
int humid_ones;

int ones = 0;
int tens = 0;
int hundreds = 0;

int humidity;
int temperature;

int red_time;
int green_time;
int blue_time;

int red_temp;
int green_temp;
int blue_temp;

int red_humid;
int green_humid;
int blue_humid;

bool temp_sensor = true;
int temp_interval = 15;
int temp_period = 1;
int brightness_set = 0;
int LightSample = 0;
int night_bright = 0;
int night_light = 0;

int color_palete = 0;

char buffer[64] = " ";

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

char *buffer_1[255];
const uint8_t SPRINTF_BUFFER_SIZE = 32; // Buffer size for sprintf()        //
char inputBuffer[SPRINTF_BUFFER_SIZE];  // Buffer for sprintf()/sscanf()



void setup()
{
  Serial.begin(BAUD_RATE);
  ext_serial.begin(9600);

  if (!rtc.begin())
  {
    Serial.println("Could not find RTC");
  }
  //rtc.setBattery(true);

   if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!sensor.begin())
  {
    Serial.println("Did not find Si7021 sensor!");
    temp_sensor = false;
  }

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  red_time = EEPROM.read(r_time_loc);
  green_time = EEPROM.read(g_time_loc);
  blue_time = EEPROM.read(b_time_loc);

  red_temp = EEPROM.read(r_temp_loc);
  green_temp = EEPROM.read(g_temp_loc);
  blue_temp = EEPROM.read(b_temp_loc);

  temp_interval = EEPROM.read(temp_int_loc);
  temp_period = EEPROM.read(temp_per_loc);

  night_bright = EEPROM.read(night_bright_loc);
  night_light = EEPROM.read(night_light_loc);  

  brightness_set = 255;
}

bool DST_check(int month, int dayOfWeek, int hour, int minute, int second)
{

  if (month == 3 && dayOfWeek == 0 && hour == 2 && minute == 00 && second == 00 && dst_count <= 2)
  {
    dst_count++;
    Serial.println("DST START");
  }
  else if (month == 11 && dayOfWeek == 0 && hour == 2 && minute == 00 && second == 00)
  {
    dst_count = 0;
    Serial.println("DST STOP");
  }

  if (dst_count == 2)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void get_temp_humid()
{

  humidity = int(sensor.readHumidity());
  temperature = int(sensor.readTemperature());
  temperature = int((temperature * 9 / 5) + 32.0);

  temp_tens = temperature / 10;
  temp_ones = temperature % 10;

  humid_tens = humidity / 10;
  humid_ones = humidity % 10;
}

void get_time()
{

  DateTime now = rtc.now();

  _month_ = now.month();
  _day_ = now.day();
  _year_ = now.year();
  _WeekDay_ = now.dayOfTheWeek();

  _hour_ = now.hour();
  _min_ = now.minute();
  _second_ = now.second();

  dst_set = DST_check(_month_, _WeekDay_, _hour_, _min_, _second_);

  if (dst_set == true)
  {
    dst_offset = 1;
  }
  else if (dst_set == false)
  {
    dst_offset = 0;
  }

  if (_hour_ == 0)
  {
    _hour_ = 12;
  }
  if (_hour_ > 12)
  {
    _hour_ = _hour_ - 12;
  }

  hour_tens = _hour_ / 10;
  hour_ones = _hour_ % 10;

  min_tens = _min_ / 10;
  min_ones = _min_ % 10;

  sec_tens = _second_ / 10;
  sec_ones = _second_ % 10;
}

void readCommand(){ 
  static uint8_t inputBytes = 0;                                                                            // Variable for buffer position     //
  while (Serial.available())
  { 
    inputBuffer[inputBytes] = Serial.read();                                                                // Get the next byte of data        //
    if (inputBuffer[inputBytes] != '\n' && inputBytes < SPRINTF_BUFFER_SIZE)                                // keep on reading until a newline  //
  inputBytes++;                                                                                             // shows up or the buffer is full   //
    else
    { 
      inputBuffer[inputBytes] = 0;                                                                          // Add the termination character    //
      for (uint8_t i = 0; i < inputBytes; i++)                                                              // Convert the whole input buffer   //
      inputBuffer[i] = toupper(inputBuffer[i]);                                                             // to uppercase characters          //
      Serial.print(F("\nCommand \""));            
      Serial.write(inputBuffer);                  
      Serial.print(F("\" received.\n"));          
      
      enum commands
      {
        SetTime,
        TimeColor,
        TempColor,
        TempInterval,
        BrightnessSet,
        NightMode,
        Save,
        Unknown_Command
      };                                         
      commands command;                          
      char workBuffer[10];                      
      sscanf(inputBuffer, "%s %*s", workBuffer); 
      if (!strcmp(workBuffer, "SET_DATE"))
        command = SetTime; // Set command number when found    //
      else if (!strcmp(workBuffer, "TIME_COLOR"))
        command = TimeColor;
      else if (!strcmp(workBuffer, "TEMP_COLOR"))
        command = TempColor;
      else if (!strcmp(workBuffer, "TEMP_INTERVAL"))
        command = TempInterval;
      else if (!strcmp(workBuffer, "BRIGHTNESS"))
        command = BrightnessSet;
      else if (!strcmp(workBuffer, "NIGHT_MODE"))
        command = NightMode;
      else if (!strcmp(workBuffer, "SAVE"))
        command = Save;
      else
        command = Unknown_Command;                                                                      
      uint16_t tokens, year, month, day, hour, minute, second, red, green, blue, set, duration, period, palete=0; 
      switch (command)
      { // Action depending upon command    //

        case SetTime:                                                        
          tokens = sscanf(inputBuffer, " %*s %hu-%hu-%hu %hu:%hu:%hu;", 
                          &year, &month, &day, &hour, &minute, &second);            
          if (tokens != 6)                                                           
            Serial.print(F("Unable to parse date / time\n"));                      
          else
          { 
            rtc.adjust(DateTime(year, month, day, hour, minute, second));
            Serial.print(F("Date has been set.")); 
          }                                        
          break;                                   

        case TimeColor:                                             
          tokens = sscanf(inputBuffer, " %*s %hu-%hu-%hu-%hu;", 
                          &palete, &red, &green, &blue);                      
          if (tokens != 4)                                          
            Serial.print(F("Unable to set color\n"));                  
          else
          { 
            red_time = red;
            green_time = green;
            blue_time = blue;
            color_palete = palete;
            digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);
            Serial.println("Color Set\n"); 
          }                             
          break;

        case TempColor:                                             
          tokens = sscanf(inputBuffer, " %*s %hu-%hu-%hu-%hu;", 
                          &palete, &red, &green, &blue);                      
          if (tokens != 4)                                          
            Serial.print(F("Unable to set color\n"));                  
          else
          { 
            red_temp = red;
            green_temp = green;
            blue_temp = blue;
            color_palete = palete;
            digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_temp, green_temp, blue_temp);
            Serial.println("Color Set\n"); 
          }                                                             
          break;

        case TempInterval:                                  
          tokens = sscanf(inputBuffer, " %*s %hu-%hu;",  
                          &period, &duration);              
          if (tokens != 2)                                  
            Serial.print(F("Unable to set interval time")); 
          else
          { 
            temp_interval = duration;
            temp_period = period;
            Serial.println("Interval Set\n"); 
          }                                 
          break;

        case BrightnessSet:                                  
          tokens = sscanf(inputBuffer, " %*s %hu;",  
                          &set);              
          if (tokens != 1)                                  
            Serial.print(F("Unable to set brightness\n"));
          else
          { 
            brightness_set = set;
            FastLED.setBrightness(brightness_set);
            Serial.println("Brightness Set\n"); 
          }                                 
          break;

        case NightMode:                                  
          tokens = sscanf(inputBuffer, " %*s %hu-%hu;",  
                          &period, &duration);              
          if (tokens != 2)                                 
            Serial.print(F("Unable to set night settings")); 
          else
          {                                  
            night_bright = duration;
            night_light = period;
            Serial.println("Night settings set\n"); 
          }                                 
          break;

        case Save:                                   
          tokens = sscanf(inputBuffer, " %*s %hu;", 
                          &set);                    
          if (tokens != 1)                           
            Serial.print(F("Unable to save\n"));      
          else
          { 
            EEPROM.write(r_time_loc, red_time);
            EEPROM.write(g_time_loc, green_time);
            EEPROM.write(b_time_loc, blue_time);

            EEPROM.write(r_temp_loc, red_temp);
            EEPROM.write(g_temp_loc, green_temp);
            EEPROM.write(b_temp_loc, blue_temp); 

            EEPROM.write(temp_int_loc, temp_interval);
            EEPROM.write(temp_per_loc, temp_period);

            EEPROM.write(night_bright_loc, night_bright);
            EEPROM.write(night_light_loc, night_light);

            Serial.println("Colors saved\n"); 
          }                                 
          break;
          
        case Unknown_Command:                                         
        default:                                                     
          Serial.println(F("Unknown command. Valid commands are: ")); 
          Serial.println(F("SET_DATE yyyy-mm-dd hh:mm:ss"));     
          Serial.println(F("TIME_COLOR R-G-B"));
          Serial.println(F("TEMP_COLOR R-G-B"));
          Serial.println(F("TEMP_INTERVAL Period-Duration"));
          Serial.println(F("BRIGHTNESS Value"));   
          Serial.println(F("NIGHT_MODE Brightness-Threshold"));  
          Serial.println(F("SAVE 1"));
      }                                                            
      inputBytes = 0;                                              
    }                                                               
  }                                                                 

  ext_serial.listen();
  while (ext_serial.available())
  { // Loop while incoming serial data  //
    inputBuffer[inputBytes] = ext_serial.read();                             // Get the next byte of data        //
    if (inputBuffer[inputBytes] != '\n' && inputBytes < SPRINTF_BUFFER_SIZE) // keep on reading until a newline  //
      inputBytes++;                                                          // shows up or the buffer is full   //
    else
    { //                                  //
      inputBuffer[inputBytes] = 0;                // Add the termination character    //
      for (uint8_t i = 0; i < inputBytes; i++)    // Convert the whole input buffer   //
        inputBuffer[i] = toupper(inputBuffer[i]); // to uppercase characters          //
      Serial.print(F("\nCommand \""));            //                                  //
      Serial.write(inputBuffer);                  //                                  //
      Serial.print(F("\" received.\n"));          //                                  //
       enum commands
      {
        SetTime,
        TimeColor,
        TempColor,
        TempInterval,
        BrightnessSet,
        NightMode,
        Save,
        Unknown_Command
      };                                         
      commands command;                          
      char workBuffer[10];                      
      sscanf(inputBuffer, "%s %*s", workBuffer); 
      if (!strcmp(workBuffer, "SET_DATE"))
        command = SetTime; // Set command number when found    //
      else if (!strcmp(workBuffer, "TIME_COLOR"))
        command = TimeColor;
      else if (!strcmp(workBuffer, "TEMP_COLOR"))
        command = TempColor;
      else if (!strcmp(workBuffer, "TEMP_INTERVAL"))
        command = TempInterval;
      else if (!strcmp(workBuffer, "BRIGHTNESS"))
        command = BrightnessSet;
      else if (!strcmp(workBuffer, "NIGHT_MODE"))
        command = NightMode;
      else if (!strcmp(workBuffer, "SAVE"))
        command = Save;
      else
        command = Unknown_Command;                                                                      
      uint16_t tokens, year, month, day, hour, minute, second, red, green, blue, set, duration, period, palete=0; 
      switch (command)
      { // Action depending upon command    //

        case SetTime:                                                        
          tokens = sscanf(inputBuffer, " %*s %hu-%hu-%hu %hu:%hu:%hu;", 
                          &year, &month, &day, &hour, &minute, &second);            
          if (tokens != 6)                                                           
            Serial.print(F("Unable to parse date / time\n"));                      
          else
          { 
            rtc.adjust(DateTime(year, month, day, hour, minute, second));
            Serial.print(F("Date has been set.")); 
          }                                        
          break;                                   

        case TimeColor:                                             
          tokens = sscanf(inputBuffer, " %*s %hu-%hu-%hu-%hu;", 
                          &palete, &red, &green, &blue);                      
          if (tokens != 4)                                          
            Serial.print(F("Unable to set color\n"));                  
          else
          { 
            red_time = red;
            green_time = green;
            blue_time = blue;
            color_palete = palete;
            digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);
            Serial.println("Color Set\n"); 
          }                             
          break;

        case TempColor:                                             
          tokens = sscanf(inputBuffer, " %*s %hu-%hu-%hu-%hu;", 
                          &palete, &red, &green, &blue);                      
          if (tokens != 4)                                          
            Serial.print(F("Unable to set color\n"));                  
          else
          { 
            red_temp = red;
            green_temp = green;
            blue_temp = blue;
            color_palete = palete;
            digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_temp, green_temp, blue_temp);
            Serial.println("Color Set\n"); 
          }                                                             
          break;

        case TempInterval:                                  
          tokens = sscanf(inputBuffer, " %*s %hu-%hu;",  
                          &period, &duration);              
          if (tokens != 2)                                  
            Serial.print(F("Unable to set interval time")); 
          else
          { 
            temp_interval = duration;
            temp_period = period;
            Serial.println("Interval Set\n"); 
          }                                 
          break;

        case BrightnessSet:                                  
          tokens = sscanf(inputBuffer, " %*s %hu;",  
                          &set);              
          if (tokens != 1)                                  
            Serial.print(F("Unable to set brightness\n"));
          else
          { 
            brightness_set = set;
            FastLED.setBrightness(brightness_set);
            Serial.println("Brightness Set\n"); 
          }                                 
          break;

        case NightMode:                                  
          tokens = sscanf(inputBuffer, " %*s %hu-%hu;",  
                          &period, &duration);              
          if (tokens != 2)                                 
            Serial.print(F("Unable to set night settings")); 
          else
          {                                  
            night_bright = duration;
            night_light = period;
            Serial.println("Night settings set\n"); 
          }  

        case Save:                                   
          tokens = sscanf(inputBuffer, " %*s %hu;", 
                          &set);                    
          if (tokens != 1)                           
            Serial.print(F("Unable to save\n"));      
          else
          { 
            EEPROM.write(r_time_loc, red_time);
            EEPROM.write(g_time_loc, green_time);
            EEPROM.write(b_time_loc, blue_time);

            EEPROM.write(r_temp_loc, red_temp);
            EEPROM.write(g_temp_loc, green_temp);
            EEPROM.write(b_temp_loc, blue_temp); 

            EEPROM.write(temp_int_loc, temp_interval);
            EEPROM.write(temp_per_loc, temp_period);

            EEPROM.write(night_bright_loc, night_bright);
            EEPROM.write(night_light_loc, night_light);

            Serial.println("Colors saved\n"); 
          }                                 
          break;
          
        case Unknown_Command:                                         
        default:                                                     
          Serial.println(F("Unknown command. Valid commands are: ")); 
          Serial.println(F("SET_DATE yyyy-mm-dd hh:mm:ss"));     
          Serial.println(F("TIME_COLOR R-G-B"));
          Serial.println(F("TEMP_COLOR R-G-B"));
          Serial.println(F("TEMP_INTERVAL Period-Duration"));
          Serial.println(F("BRIGHTNESS Value"));   
          Serial.println(F("NIGHT_MODE Brightness-Threshold"));  
          Serial.println(F("SAVE 1"));
      }                                                            
      inputBytes = 0;   
    }                                                               
  }                                                                 

} 

void digit_show(int palete, int h_t, int h_o, int m_t, int m_o, int r, int g, int b)
{

  //Serial.println(m_o);
  for (int a = 0; a < 7; a++)
  {
    if (digit_set[m_o][a] == yes)
    {
      if(palete == 1){
        leds[a] = CHSV(r, g, b);
      }
      else{
        leds[a] = CRGB(r, g, b);
      }
      
    }

    else if (digit_set[m_o][a] == no)
    {
      if(palete == 1){
        leds[a] = CHSV(0, 0, 0);
      }
      else{
        leds[a] = CRGB(0, 0, 0);
      }
    }
  }

  for (int a = 7; a < 14; a++)
  {
    if (digit_set[m_t][a - 7] == yes)
    {
      if(palete == 1){
        leds[a] = CHSV(r, g, b);
      }
      else{
        leds[a] = CRGB(r, g, b);
      }
    }

    else if (digit_set[m_t][a - 7] == no)
    {
      if(palete == 1){
        leds[a] = CHSV(0, 0, 0);
      }
      else{
        leds[a] = CRGB(0, 0, 0);
      }
    }
  }

  for (int a = 14; a < 21; a++)
  {
    if (digit_set[h_o][a - 14] == yes)
    {
      if(palete == 1){
        leds[a] = CHSV(r, g, b);
      }
      else{
        leds[a] = CRGB(r, g, b);
      }
    }

    else if (digit_set[h_o][a - 14] == no)
    {
      if(palete == 1){
        leds[a] = CHSV(0, 0, 0);
      }
      else{
        leds[a] = CRGB(0, 0, 0);
      }
    }
  }

  for (int a = 21; a < 28; a++)
  {
    if (h_t == 0)
    {
      if(palete == 1){
        leds[a] = CHSV(0, 0, 0);
      }
      else{
        leds[a] = CRGB(0, 0, 0);
      }
    }
    else
    {
      if (digit_set[h_t][a - 21] == yes)
      {
        if(palete == 1){
        leds[a] = CHSV(r, g, b);
      }
      else{
        leds[a] = CRGB(r, g, b);
      }
      }

      else if (digit_set[h_t][a - 21] == no)
      {
        if(palete == 1){
        leds[a] = CHSV(0, 0, 0);
      }
      else{
        leds[a] = CRGB(0, 0, 0);
      }
      }
    }
  }

  FastLED.show();
}
void loop()
{
  if (millis() > time_1 + (1e3))
  {

    time_1 = millis();
    get_time();
    get_temp_humid();

    Serial.print(hour_tens);
    Serial.print(hour_ones);
    Serial.print(": ");
    Serial.print(min_tens);
    Serial.print(min_ones);
    Serial.print(": ");
    Serial.print(sec_tens);
    Serial.print(sec_ones);
    Serial.print(",");
    Serial.print(LightSample);

    ext_serial.print(hour_tens);
    ext_serial.print(hour_ones);
    ext_serial.print(": ");
    ext_serial.print(min_tens);
    ext_serial.print(min_ones);
    ext_serial.print(": ");
    ext_serial.print(sec_tens);
    ext_serial.print(sec_ones);
    ext_serial.print(",");
    ext_serial.print(LightSample);

    if (temp_sensor == true)
    {
      Serial.print(", ");
      Serial.print(temp_tens);
      Serial.print(temp_ones);
      Serial.print(", ");
      Serial.print(humid_tens);
      Serial.print(humid_ones);
      Serial.print("\n");

      ext_serial.print(", ");
      ext_serial.print(temp_tens);
      ext_serial.print(temp_ones);
      ext_serial.print(", ");
      ext_serial.print(humid_tens);
      ext_serial.print(humid_ones);
      ext_serial.print("\n");
    }
    else
    {
      Serial.print("\n");
      ext_serial.print("\n");
    }

    if (_min_ % temp_period == 0) {
      if (_second_ >= 0 && _second_ < temp_interval) {
        if (temp_sensor == true)
        {
          digit_show(color_palete, temp_tens, temp_ones, degree, unit_f, red_temp, green_temp, blue_temp);
        }
        else
        {
          digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);
        }
      }
      else if (_second_ >= temp_interval) {
        digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);
      }
    }
    else {
      digit_show(color_palete, hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);
    }
  }

  if (millis() > time_3 + 1e3)
  {
    time_3 = millis();
    LightSample = analogRead(A6);
    if (LightSample < night_light)
    {
      
      FastLED.setBrightness(night_bright);
    }
    else
    {
      FastLED.setBrightness(brightness_set);
    }
  }

  readCommand();

}
