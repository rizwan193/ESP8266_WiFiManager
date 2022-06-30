#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <FS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

WiFiServer server(80);

const char* ssid = "Enter your router ssid";   
const char* password = "Enter the router password";
char API[33] = "";                  //API to be entered 
char savedAPI[33] = "1234567";     //Default API