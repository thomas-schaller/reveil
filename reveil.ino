// Program to demonstrate the MD_Parola library
//
// Display the time in one zone and other information scrolling through in
// another zone.
// - Time is shown in a user defined fixed width font
// - Scrolling text uses the default font
// - Temperature display uses user defined characters
// - Optional use of DS1307 module for time and DHT11 sensor for temp and humidity
// - DS1307 library (MD_DS1307) found at https://github.com/MajicDesigns/DS1307
// - MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//

// Use the DS3231 clock module
#define USE_DS3231 1

// Header file includes
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#include "Font_Data.h"

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8

#define CLK_PIN   11
#define DATA_PIN  12
#define CS_PIN    10


#if USE_DS3231
#include <RTC.h>
#include <Wire.h>
static DS3231 RTC;
#endif

// Arbitrary output pins
 MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define SPEED_TIME  75
#define PAUSE_TIME  0

#define MAX_MESG  20

// Turn on debug statements to the serial output
#define DEBUG 0

// Global variables
char szTime[9];    // mm:ss\0
char szMesg[MAX_MESG+1] = "";


void getTime(char *psz, bool f = true)
// Code for reading clock time
{
#if  USE_DS3231

  sprintf(psz, "%02d%c%02d", RTC.getHours(), (f ? ':' : ' '), RTC.getMinutes());
#else
  uint16_t  h, m, s;

  s = millis()/1000;
  m = s/60;
  h = m/60;
  m %= 60;
  s %= 60;
  sprintf(psz, "%02d%c%02d", h, (f ? ':' : ' '), m);
#endif
}

void getDate(char *psz)
// Code for reading clock date
{
#if USE_DS3231
  char  szBuf[10];
  sprintf(psz, "%02d/%02d", RTC.getDay(), RTC.getMonth());
#else
  strcpy(szMesg, "29/02/2016");
#endif
}

void setup(void)
{
  P.begin(2);

  P.setZone(0, 0, MAX_DEVICES-5);
  P.setZone(1, MAX_DEVICES-4, MAX_DEVICES-1);
  P.setFont(1, numeric7Seg);

  P.setInvert(false);
  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, 0, PA_PRINT, PA_NO_EFFECT);

 

#if USE_DS3231
  RTC.begin();
  RTC.stopClock();
  RTC.setHourMode(CLOCK_H24);
  RTC.setDateTime(__DATE__, __TIME__);
  RTC.startClock();
#endif

  getTime(szTime);
}

void loop(void)
{
  static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher

  P.displayAnimate();
  P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
  getDate(szMesg);
  P.displayReset(0);
  

  // Finally, adjust the time string if we have to
  if (millis() - lastTime >= 1000)
  {
    lastTime = millis();
    getTime(szTime, flasher);
    flasher = !flasher;

    P.displayReset(1);
  }
}
