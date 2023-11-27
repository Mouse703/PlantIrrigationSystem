#include <Arduino.h>
#include <WiFi.h>

#define BLYNK_TEMPLATE_ID "TMPL5G6PoB9Pq"
#define BLYNK_TEMPLATE_NAME "IIoT Plant Irrigation System"
#define BLYNK_AUTH_TOKEN "aU_mUNOYi5Id8-NmwmvP4QECHetU-Yrk"
#define Pump 12
#define SNSR 34

#include <BlynkSimpleEsp32.h>

char ssid[] = "telenet-6DE6195";
char password[] = "b5xwp3Pjjedj";
char auth[] = "aU_mUNOYi5Id8-NmwmvP4QECHetU-Yrk";

int pumpState;
int oldPumpState;
int thresholdHigh;
int thresholdLow;
int moistureReading;
int moistureReadingOld;
int moistureReadingPercent;

unsigned long previousMillis;
unsigned long sampleRate = 2500;

void readSensor();

void setup() 
{
  Serial.begin(115200);
  pinMode(Pump, OUTPUT);
  while(! Serial);                          //Wait for Serial
  Blynk.begin(auth, ssid, password);        //Set parameters
  Blynk.syncVirtual(V0);                    //ThresholdHigh
  Blynk.syncVirtual(V1);                    //ThresholdLow
  Blynk.syncVirtual(V2);                    //MoistureSensor
  Blynk.syncVirtual(V3);                    //Pump
}

void loop() 
{
  Blynk.run();
  //Sending sensorvalue to AdafruitIO if changed
  if ((unsigned long)(millis() - previousMillis) > sampleRate)
  {
    readSensor();
    if (moistureReadingPercent < thresholdLow)  
    {
      Blynk.virtualWrite(V3, 1);
      Serial.print("IO Sending pump state-> ");
      Serial.println(pumpState);
      digitalWrite(Pump, HIGH);
      delay(1000); //1 second pump activation to increase soil moisture
      digitalWrite(Pump, LOW);
      Serial.print("IO Sending pump state-> ");
      Serial.println(pumpState); 
      Blynk.virtualWrite(V3, 0);
      for (int i = 0; i <= 30; i++) 
      {
        readSensor();
        Serial.print("Moisture increasing/stabilizing...");
        Serial.print("Current value: ");
        Serial.println(moistureReadingPercent);
        Blynk.virtualWrite(V2, moistureReadingPercent);
        delay(2000);
      }
    }
    else {Serial.println("Soil OK");}

    if (moistureReadingOld == moistureReadingPercent) 
    {
      Serial.print("Moisture unchanged, skipping IO upload, current value: ");
      Serial.print(moistureReadingPercent);
      Serial.println("%");
    }    
    else 
    {
      Serial.print("IO Sending moisture value-> ");
      Serial.print(moistureReadingPercent);
      Serial.println("%");
      Blynk.virtualWrite(V2, moistureReadingPercent);
    }
    moistureReadingOld = moistureReadingPercent;
    previousMillis = millis();
  }
}


// BLYNK_WRITE handler for thresholdHigh
BLYNK_WRITE(V0) 
{
  thresholdHigh = param.asInt();  // Get the value from the app
  Serial.println("Incoming IO data");
  Serial.print("New threshold high: ");
  Serial.println(thresholdHigh);
  Serial.println();
  // You can use thresholdHigh in your code for your specific application
}

// BLYNK_WRITE handler for thresholdLow
BLYNK_WRITE(V1) 
{
  thresholdLow = param.asInt();
  Serial.println("Incoming IO data");
  Serial.print("New threshold low: ");
  Serial.println(thresholdLow);
  Serial.println();
}

void readSensor()
{
  moistureReading = analogRead(SNSR);
  moistureReadingPercent = 100 - map(moistureReading, 0, 4095, 0, 100);
}