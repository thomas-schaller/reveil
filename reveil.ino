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

#include <DS3231.h>
#include <Wire.h>

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


#define SPEED_TIME  75
#define PAUSE_TIME  0

#define MAX_MESG  20

// Turn on debug statements to the serial output
#define DEBUG 0

// Global variables
char szTime[11];    // mm:ss\0
char szMesg[MAX_MESG+1] = "";
boolean siecle =false;
int mode = 0;
  boolean h12Flag,pmFlag;
const uint32_t PAUSE_BOUTON = 1000;
static uint32_t lastTime2 = 0; // millis() memory
// Arbitrary output pins
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
DS3231 horloge;

void getTime(char *psz, bool f = true)
// Code for reading clock time
{
  char secondes[3]; // pour affichage des secondes
  sprintf(secondes,"%02d",horloge.getSecond());
  secondes[0]=secondes[0]+23; // permutation codes AScci 71 à 80
  secondes[1]=secondes[1]+23;

  sprintf(psz, "%02d%c%02d %c%c", horloge.getHour(h12Flag,pmFlag), (f ? ':' : ' '), horloge.getMinute(),secondes[0],secondes[1]);

}

void getDate(char *psz)
// Code for reading clock date
{
  sprintf(psz, "%02d/%02d", horloge.getDate(), horloge.getMonth(siecle));
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

  // Start the I2C interface
   Wire.begin();
   horloge.setClockMode(false);
   getTime(szTime);
   Serial.begin (115200);
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
  if (digitalRead(SWITCH_PIN) == HIGH && !horloge.checkAlarmEnabled(1))
    {
      horloge.turnOnAlarm(1);
    }
    else if ( digitalRead(SWITCH_PIN) == LOW && horloge.checkAlarmEnabled(1) )
    {
      horloge.turnOffAlarm(1);
    }

  if ( horloge.checkAlarmEnabled(1) )
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
    Serial.println("activation reglage Horloge");
    lastTime2 = millis();
  }
  if (digitalRead(ALA_BU_PIN) == HIGH && millis() - lastTime2 >= PAUSE_BOUTON )
  {
    mode =2;
    Serial.println("activation reglage Alarme");
    lastTime2 = millis();
  }
  if (horloge.checkIfAlarm(1))
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
  byte valeur = horloge.getHour(h12Flag,pmFlag);
  byte heure;
  byte minu=horloge.getMinute();
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
     sprintf(szTime, "%02d:%02d", heure , valeur);
    }
    P.displayReset(1);
    
    if (digitalRead(HOR_BU_PIN) == HIGH && millis() - lastTime2 >= PAUSE_BOUTON)
    {
      Serial.print("Reglage Horloge ");

      if (reglage == 0)
      {
        
        heure=valeur;
        
        
        Serial.print("Valeur minutes ");   
        valeur = minu;
        modulo = 60;
      }
      else
      {
        
        horloge.setHour(heure);
        horloge.setMinute(valeur);
        Serial.print("mise a jour Heure ");
        Serial.println(horloge.getHour(h12Flag,pmFlag));
        Serial.print("mise a jour minutes ");
        Serial.println(horloge.getMinute());
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
  byte modulo = 24;
  byte jourAlarme, heureAlarme, minuteAlarme, secondeAlarme, alarmBits;
  bool alarmDy, alarmH12Flag, alarmPmFlag;
  horloge.getA1Time(jourAlarme,heureAlarme,minuteAlarme,secondeAlarme,alarmBits, alarmDy, alarmH12Flag, alarmPmFlag);
  byte valeur = heureAlarme;
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
     sprintf(szTime, "%02d:%02d",heureAlarme , valeur);
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
        heureAlarme = valeur;
        Serial.print("Valeur minutes ");
        Serial.println(minuteAlarme);
        valeur = minuteAlarme;
        modulo = 60;
      }
      else
      {
        minuteAlarme = valeur;
        Serial.print("mise a jour heure ");
        Serial.print(heureAlarme);
        Serial.print("mise a jour minutes ");
        Serial.println(minuteAlarme);
        horloge.turnOffAlarm(1);
        horloge.setA1Time(0, heureAlarme, minuteAlarme, 0,0x8,false,false,false);
        horloge.turnOnAlarm(1);
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
}
