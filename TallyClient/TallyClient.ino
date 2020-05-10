/**
   reuseConnection.ino

    Created on: 22.11.2015

*/


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>

#include <ESP8266HTTPClient.h>
#include "ssid.h"

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

#define CAMERA 1
#define HOSTNAME "CAMERA1"
#define TARGET_HOSTNAME "WiFiTally.local"
// #define DEBUG

ESP8266WiFiMulti WiFiMulti;

HTTPClient http;

IPAddress server_ip;
uint16_t server_port=80;
bool server_ready = false;

bool resolve_mdns_service(char* service_name, char* protocol, char* desired_host, IPAddress* ip_addr, uint16_t *port_number) {
  Serial.println("Sending mDNS query");
  int n = MDNS.queryService(service_name, protocol);
  Serial.printf("mDNS query got %d results\n", n);

  if(n == 0) {
    Serial.println("no services found");
  } else {
    for (int i = 0; i < n; ++i) {
#ifdef DEBUG
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
#endif

      if(MDNS.hostname(i) == String(desired_host)) {
        *ip_addr = MDNS.IP(i);
        *port_number = MDNS.port(i);
        return true;
      }
    }
  }

  return false;
}

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
  
  // Start mDNS
  if(!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }
}

WiFiClient client;
String server_url;

void loop() {
  if (!server_ready) {
    if(resolve_mdns_service("http", "tcp", TARGET_HOSTNAME, &server_ip, &server_port)) {
      Serial.printf("got an answer for %s.local!\n", TARGET_HOSTNAME);
      Serial.println(server_ip);
      Serial.println(server_port);
      server_ready = true;
      server_url = String("http://") + server_ip.toString() + String(":") + String(server_port) + String("/tally");
      Serial.println(server_url);
    } else {
      Serial.printf("Sorry, %s.local not found\n", TARGET_HOSTNAME);
    }    
  }
  
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED) && server_ready) {
    http.begin(client, server_url);

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
