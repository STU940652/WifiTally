#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "ssid.h"

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

WiFiUDP Udp;
char incomingPacket[255];  // buffer for incoming packets


const int led = LED_BUILTIN;

#define CAMERA_COUNT 6

const int tally_inputs [CAMERA_COUNT] = {
  5,  // D1
  4,  // D2
  0,  // D3
  
  14, // D5
  12, // D6
  13 // D7
};

struct tally_client_t {
  IPAddress ip;
  unsigned long expire_time;
} tally_clients[CAMERA_COUNT];

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

  // Init connection database
  for (int i=0; i < (sizeof(tally_clients)/sizeof(struct tally_client_t)); i++) {
    tally_clients[i].expire_time = 0;
  }

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

  Udp.begin(80);

  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
}

#define EXPIRE_PERIOD (120 * 60 * 1000)

void loop(void) {
  MDNS.update();

  /* Check for incoming messages */
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    if (String(incomingPacket).startsWith("CAMERA")) {
      unsigned int num = incomingPacket[6] - '1';
      if (num < CAMERA_COUNT) {
        Serial.printf("Registered camera %i\n", num);
        tally_clients[num].ip = Udp.remoteIP();
        tally_clients[num].expire_time = millis() + EXPIRE_PERIOD;
      }
    }
  }

  /* Update Tally Lights */
  for (int i=0; i < CAMERA_COUNT; i++){
    if (tally_clients[i].expire_time > millis()) {
      // We have a valid client
      Udp.beginPacket(tally_clients[i].ip, 80);
      if (digitalRead(tally_inputs[i])) {
        Udp.write("0\n");
      } else {
        Udp.write("1\n");
      }
      Udp.endPacket();
    }
  }

  delay(100);
}
