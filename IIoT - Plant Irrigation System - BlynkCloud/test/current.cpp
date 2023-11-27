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

const char *host = "192.168.0.228";  // Replace with your PC's IP address
const int port = 2121;

unsigned long previousMillis;
unsigned long sampleRate = 2500;

WiFiClient client;

void readSensor();
void sendToPC(int pumpState, int moistureReadingPercent);

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
    sendToPC(pumpState, moistureReadingPercent);
    if (moistureReadingPercent < thresholdLow)  
    {
      digitalWrite(Pump, HIGH);
      pumpState = 1;
      Blynk.virtualWrite(V3, 1);
      Serial.print("IO Sending pump state-> ");
      Serial.println(pumpState);
      sendToPC(pumpState, moistureReadingPercent);
      delay(1000); //1 second pump activation to increase soil moisture
      digitalWrite(Pump, LOW);
      pumpState = 0;
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

void sendToPC(int pumpState, int moistureReadingPercent) {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect(host, port)) {
      // Format your data as an HTTP POST request
      String data = "PumpState: " + String(pumpState) + ", MoistureReading: " + String(moistureReadingPercent) + "%";

      // Send the HTTP POST request
      client.println("POST / HTTP/1.1");
      client.println("Host: " + String(host) + ":" + String(port));
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(data.length());
      client.println();
      client.print(data);

      // Close the connection
      client.stop();
    }
  }
}
