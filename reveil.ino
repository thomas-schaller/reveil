// Program to demonstrate the MD_Parola library
//
// Display the time in one zone and other information scrolling through in
// another zone.
// - Time is shown in a user defined fixed width font
// - Scrolling text uses the default font
// - Optional use of DS3231 module for time
// - MD_MAX72XX library can be found at https://github.com/MajicDesigns/MD_MAX72XX
//

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

// potentiometre KY-040
#define KY_DT_PIN 5 
#define KY_CLK_PIN 3

#define HOR_BU_PIN 7 
#define ALA_BU_PIN 8 

#define LED_ALA_PIN A3

//info alarme
#define LED_PIN 9
#define SWITCH_PIN 6

#include <RTC.h>
#include <Wire.h>
static DS3231 RTC;

// Arbitrary output pins
 MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define SPEED_TIME  75
#define PAUSE_TIME  0

#define MAX_MESG  20

// Turn on debug statements to the serial output
#define DEBUG 0

// Global variables
char szTime[11];    // mm:ss\0
char szMesg[MAX_MESG+1] = "";
int mode = 0;
const uint32_t PAUSE_BOUTON = 1000;
static uint32_t lastTime2 = 0; // millis() memory
DateTime Alarm1;


void getTime(char *psz, bool f = true)
// Code for reading clock time
{
  char secondes[3]; // pour affichage des secondes
  sprintf(secondes,"%02d",RTC.getSeconds());
  secondes[0]=secondes[0]+23; // permutation codes AScci 71 à 80
  secondes[1]=secondes[1]+23;

  sprintf(psz, "%02d%c%02d %c%c", RTC.getHours(), (f ? ':' : ' '), RTC.getMinutes(),secondes[0],secondes[1]);

}

void getDate(char *psz)
// Code for reading clock date
{
  char  szBuf[10];
  sprintf(psz, "%02d/%02d", RTC.getDay(), RTC.getMonth());
}

void setup(void)
{

  //potentiometre
   pinMode (KY_CLK_PIN,INPUT);
   pinMode (KY_DT_PIN,INPUT);
   
   pinMode (HOR_BU_PIN,INPUT);
   pinMode (ALA_BU_PIN,INPUT);
  
  
   digitalWrite(KY_CLK_PIN,true);
   digitalWrite(KY_DT_PIN,true);
   digitalWrite(HOR_BU_PIN,true);

   pinMode (SWITCH_PIN,INPUT);
  
   pinMode (LED_PIN,OUTPUT);
   pinMode(LED_ALA_PIN,OUTPUT);
   
  P.begin(2);

  P.setZone(0, 0, MAX_DEVICES-5);
  P.setZone(1, MAX_DEVICES-4, MAX_DEVICES-1);
  P.setFont(1, numeric7Seg_Byfeel);

  P.setInvert(false);
  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(0, szMesg, PA_CENTER, SPEED_TIME, 0, PA_PRINT, PA_NO_EFFECT);

 

  RTC.begin();
  RTC.stopClock();
  RTC.setHourMode(CLOCK_H24);
  RTC.setDateTime(__DATE__, __TIME__);
  RTC.startClock();
  getTime(szTime);
  Alarm1 = RTC.getAlarm1();

  

   Serial.begin (115200);

    Serial.print("INT/SQW Pin Mode : ");
  if (RTC.getINTPinMode())
    Serial.println("Alarm");
  else
    Serial.println("SQW");
}

void loop(void)
{
  
  if (mode == 0){
    affichage();
  }
  else if ( mode == 1) 
  {
    reglageHorloge();
    mode = 0;
  }
  else
  {
    reglageAlarme();
    mode = 0;
  }
  if (digitalRead(SWITCH_PIN) == HIGH && !RTC.isAlarm1Enabled())
    {
      RTC.enableAlarm1();
    }
    else if ( digitalRead(SWITCH_PIN) == LOW && RTC.isAlarm1Enabled() )
    {
      RTC.disableAlarm1();
    }

  if ( RTC.isAlarm1Enabled() )
  {
    digitalWrite(LED_PIN,HIGH);
  }
  else
  {
    digitalWrite(LED_PIN,LOW);
  }
  if (digitalRead(HOR_BU_PIN) == HIGH && millis() - lastTime2 >= PAUSE_BOUTON )
  {
    mode =1;
    Serial.print("boutton pousse mode");
    Serial.println(mode);
    lastTime2 = millis();
  }
  if (digitalRead(ALA_BU_PIN) == HIGH && millis() - lastTime2 >= PAUSE_BOUTON )
  {
    mode =2;
    lastTime2 = millis();
  }
  if (RTC.isAlarm1Tiggered())
  {
    digitalWrite(LED_ALA_PIN,HIGH);
    Serial.println("Alarme declenchée");
  }
  else
  {
    digitalWrite(LED_ALA_PIN,LOW);
  }
}

void affichage(void )
{
static uint32_t lastTime = 0; // millis() memory
  static uint8_t  display = 0;  // current display mode
  static bool flasher = false;  // seconds passing flasher

  P.displayAnimate();
  getDate(szMesg);
  P.displayReset(0);
  

  // Finally, adjust the time string if we have to
  if (millis() - lastTime >= 500)
  {
    lastTime = millis();
    getTime(szTime, flasher);
    flasher = !flasher;

    P.displayReset(1);
  }
}
void reglageHorloge(void)
{
  int Pin_clk_Aktuell;
  int Pin_clk_Letzter= digitalRead(KY_CLK_PIN);
  byte reglage =0;
  byte valeur = RTC.getHours();
  byte h = valeur;
  byte m = RTC.getMinutes();
  RTC.stopClock();
  byte modulo = 24;
  while (reglage <2)
  {
      P.displayAnimate();
    strcpy(szMesg, "Horloge");
    P.displayReset(0);
    
    if (reglage == 0)
    {
      sprintf(szTime, "%02d%:", valeur);
    }else
    {
     sprintf(szTime, "%02d:%02d", h , valeur);
    }
    P.displayReset(1);
    
    if (digitalRead(HOR_BU_PIN) == HIGH && millis() - lastTime2 >= PAUSE_BOUTON)
    {
      Serial.print("Reglage Horloge reglage ");
      Serial.print(reglage);
      Serial.print(" valeur ");
      Serial.println(valeur);
      if (reglage == 0)
      {
        h = valeur;
        Serial.print("Valeur minutes ");
        Serial.println(m);
        valeur = m;
        modulo = 60;
      }
      else
      {
        Serial.print("mise a jour heure ");
        Serial.print(h);
        RTC.setHours(h);
        RTC.setMinutes(valeur);
        Serial.print("mise a jour minutes ");
        Serial.println(valeur);
        RTC.startClock();
      }
      reglage ++;
      lastTime2 = millis();
    }
 
    // Lecture des statuts actuels
   Pin_clk_Aktuell = digitalRead(KY_CLK_PIN);
    
   // Vérification de changement
   if (Pin_clk_Aktuell != Pin_clk_Letzter)
   { 
          
        if (digitalRead(KY_DT_PIN) != Pin_clk_Aktuell) 
        {  
            // Pin_CLK a changé en premier
            valeur= (valeur+1+modulo)%modulo;
            
        } 
          
        else
        {       // Sinon Pin_DT achangé en premier
            valeur= (valeur-1+modulo)%modulo;
        }
      Serial.print("Horloge mode ");
      Serial.print(reglage);        
      Serial.print(" valeur ");
      Serial.println(valeur);
          
   } 
    
   // Préparation de la prochaine exécution:
   Pin_clk_Letzter = Pin_clk_Aktuell;
  }  
}

void reglageAlarme(void)
{
  int Pin_clk_Aktuell;
  int Pin_clk_Letzter= digitalRead(KY_CLK_PIN);
  byte reglage =0;
  byte valeur = Alarm1.hours;
  byte modulo = 24;
  while (reglage <2)
  {
      P.displayAnimate();
    strcpy(szMesg, "Alarme");
    P.displayReset(0);
    
    if (reglage == 0)
    {
      sprintf(szTime, "%02d%:", valeur);
    }else
    {
     sprintf(szTime, "%02d:%02d", Alarm1.hours , valeur);
    }
    P.displayReset(1);
    
    if (digitalRead(ALA_BU_PIN) == HIGH && millis() - lastTime2 > PAUSE_BOUTON)
    {
      Serial.print("Alarme reglage ");
      Serial.print(reglage);
      Serial.print(" valeur ");
      Serial.println(valeur);
      if (reglage == 0)
      {
        Alarm1.hours = valeur;
        Serial.print("Valeur minutes ");
        Serial.println(Alarm1.minutes);
        valeur = Alarm1.minutes;
        modulo = 60;
      }
      else
      {
        Serial.print("mise a jour heure ");
        Serial.print(Alarm1.hours);
        
        Serial.print("mise a jour minutes ");
        Serial.println(valeur);
        Alarm1.minutes = valeur;
        RTC.setAlarm1(0, Alarm1.hours, Alarm1.minutes, 0);
      }
      reglage ++;
      lastTime2 = millis();
    }
 
    // Lecture des statuts actuels
   Pin_clk_Aktuell = digitalRead(KY_CLK_PIN);
    
   // Vérification de changement
   if (Pin_clk_Aktuell != Pin_clk_Letzter)
   { 
          
        if (digitalRead(KY_DT_PIN) != Pin_clk_Aktuell) 
        {  
            // Pin_CLK a changé en premier
            valeur= (valeur+1+modulo)%modulo;
            
        } 
          
        else
        {       // Sinon Pin_DT achangé en premier
            valeur= (valeur-1+modulo)%modulo;
        }
      Serial.print("Alarme mode ");
      Serial.print(reglage);        
      Serial.print(" valeur ");
      Serial.println(valeur);
          
   } 
    
   // Préparation de la prochaine exécution:
   Pin_clk_Letzter = Pin_clk_Aktuell;
  }
  Alarm1 = RTC.getAlarm1();  
}
