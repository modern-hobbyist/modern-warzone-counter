#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdint.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <HTTPClient.h>
#include "ascii.h"
#include "credentials.h"

#define SERIAL_BAUDRATE 115200
#define FREQUENCY 300000

int warzone_wins = 0;

unsigned long lastMillis;

WiFiMulti WiFiMulti;
Adafruit_7segment matrix;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  Serial.println();
  Serial.println();
  Serial.println();

  //For some reason this can't be called in the constructor, so I created this begin function
  matrix = Adafruit_7segment();
  matrix.begin(0x70);
  //  //  matrix.drawColon(true);
  matrix.setBrightness(5);

  printName(WARZONE_USERNAME);

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
  matrix.writeDigitNum(0, (warzone_wins / 1000));
  matrix.writeDigitNum(1, (warzone_wins / 100) % 10);
  matrix.drawColon(false);
  matrix.writeDigitNum(3, (warzone_wins / 10) % 10);
  matrix.writeDigitNum(4, warzone_wins % 10);
  matrix.writeDisplay();
  delay(1000);
}

void printName(String name){
  for(int i = 0; i <= name.length() + 3; i++){
    if(i > 14){
      matrix.writeDigitRaw(4, 0b00000000);
    }else{
      matrix.writeDigitRaw(4,  get_ascii(name.charAt(i) - ' '));
    }

    if(i-1 >= 0 && i-1 < name.length()){
      matrix.writeDigitRaw(3,  get_ascii(name.charAt(i-1) - ' '));
    }else{
      matrix.writeDigitRaw(3, 0b00000000);
    }

    if(i-2 >= 0 && i-2 < name.length()){
      matrix.writeDigitRaw(1,  get_ascii(name.charAt(i-2) - ' '));
    }else{
      matrix.writeDigitRaw(1, 0b00000000);
    }

    if(i-3 >= 0 && i-3 < name.length()){
      matrix.writeDigitRaw(0, get_ascii(name.charAt(i-3) - ' '));
    }else{
      matrix.writeDigitRaw(0, 0b00000000);
    }
    
    matrix.writeDisplay();
    delay(250);
  }
  delay(1000);
}
