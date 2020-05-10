/**
   reuseConnection.ino

    Created on: 22.11.2015

*/


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include "ssid.h"

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

#define CAMERA 1

ESP8266WiFiMulti WiFiMulti;

HTTPClient http;

void setup() {

  // Setup GPIO
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  
  Serial.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(800);
    digitalWrite(LED_BUILTIN, 0);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(STASSID, STAPSK);

  // allow reuse (if server supports it)
  http.setReuse(true);

  // Connect WiFi
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);    
  }
}

WiFiClient client;

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    http.begin(client, "http://192.168.1.98/tally");

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      // http.writeToStream(&Serial);
      // Serial.printf("Tally: %c\n", http.getString()[CAMERA-1]);
      if (http.getString()[CAMERA-1] == '1') {
        digitalWrite(LED_BUILTIN, 0);
      } else {
        digitalWrite(LED_BUILTIN, 1);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      digitalWrite(LED_BUILTIN, 0);
      delay(200);
      digitalWrite(LED_BUILTIN, 1);
      delay(200);
      digitalWrite(LED_BUILTIN, 0);
      delay(200);
      digitalWrite(LED_BUILTIN, 1);
     }

    http.end();
  } else {
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
  }

  delay(100);
}
