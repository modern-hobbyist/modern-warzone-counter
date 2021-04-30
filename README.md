# Warzone Win Counter
This is an ESP32 project that will ping the Rapid API endpoint for Warzone information to print out how many warzone wins a given user has.

# Hardware
* ESP32
* Adafruit 7-segment LED with Backpack

# Setup
## Required Libraries
* <Adafruit_GFX.h>
* "Adafruit_LEDBackpack.h"
* <Arduino.h>
* <ArduinoJson.h>
* <WiFi.h>
* <WiFiMulti.h>
* <WiFiClient.h>
* <Wire.h>
* <HTTPClient.h>

## Credentials
* Create a rapid API account and sign up for an API key
	* https://rapidapi.com/elreco/api/call-of-duty-modern-warfare
* Copy `credentials_example.h` into `credentials.h` and enter your:
	* WiFi Network
	* WiFi Password
	* RAPID_API_KEY
	* Warzone Username
	* Warzone Platform

## Compile Project
* Plug in your ESP32 and upload the project to it!
