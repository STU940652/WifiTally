#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "ssid.h"

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = LED_BUILTIN;

const int tally_inputs [] = {
  5,  // D1
  4,  // D2
  0,  // D3
  
  14, // D5
  12, // D6
  13 // D7
};

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  // Setup GPIO
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  for (int i=0; i < sizeof(tally_inputs)/sizeof(int); i++) {
    pinMode(tally_inputs[i], INPUT_PULLUP);
  }
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, 0);
    delay(500);
    digitalWrite(led, 1);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("WiFiTally")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", []() {
  digitalWrite(led, 0);
  server.send(200, "text/plain", "WiFi Tally");
  delay(100);
  digitalWrite(led, 1);
  });

  server.on("/tally", []() {
    String message = "";
    for (int i=0; i < sizeof(tally_inputs)/sizeof(int); i++) {
      if (digitalRead(tally_inputs[i])) {
        message += "0";
      } else {
        message += "1";
      }
    }
    message += "\n";
    server.send(200, "text/plain", message);
  });

  server.onNotFound(handleNotFound);

  server.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}
