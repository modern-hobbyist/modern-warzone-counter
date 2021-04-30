#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ezButton.h>
#include <stdint.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <HTTPClient.h>
#include "credentials.h"

#define SERIAL_BAUDRATE 115200
#define FREQUENCY 300000

int warzone_wins = 0;

unsigned long lastMillis;

WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println(" connected");

  setClock();
  Serial.println(makeGetURL());
  updateFromServer();
}


void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

String makeGetURL() {
  return WARZONE_GET_URL + WARZONE_USERNAME + "/" + WARZONE_PLATFORM;
}

void updateFromServer() {
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
//    client -> setCACert(rootCACertificate);
    client->setInsecure();

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\n");
      if (https.begin(*client, makeGetURL())) {  // HTTPS
        Serial.print("[HTTPS] GET...\n");
        // start connection and send HTTP header
        https.addHeader("x-rapidapi-key", RAPID_API_KEY);
        https.addHeader("x-rapidapi-host", "call-of-duty-modern-warfare.p.rapidapi.com");
        https.addHeader("useQueryString", "true");
        
        int httpCode = https.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_FOUND) {
            parseResponse(https.getString());
          }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
  
        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\n");
      }

      // End extra scoping block
    }
  
    delete client;
  } else {
    Serial.println("Unable to create client");
  }
}


void parseResponse(String httpsResponse){
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, httpsResponse);
  
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  JsonObject br = doc["br"];
  warzone_wins = br["wins"]; // 0
  Serial.print("Warzone Wins: ");
  Serial.println(warzone_wins);
}

void loop() {
  if (millis() - lastMillis >= FREQUENCY) 
  {
    lastMillis = millis();  //get ready for the next iteration
    updateFromServer();
  }
  updateDisplay();
}

void updateDisplay(){
  //TODO set display digits to warzone_wins
}
