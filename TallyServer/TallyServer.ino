#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
char incomingPacket[255];  // buffer for incoming packets

const int led = LED_BUILTIN;

#define CAMERA_COUNT 6
#define EXPIRE_PERIOD (120 * 60 * 1000)
#define UDP_PORT 80

const int tally_inputs [CAMERA_COUNT] = {
  5,  // D1
  4,  // D2
  0,  // D3
  
  14, // D5
  12, // D6
  13 // D7
};

struct stassid_t {
  char* ssid;
  char* pk;
};

#include "ssid.h"

/* ssid.h has a list like this:
struct stassid_t stassid_list[] = 
{
  {"SSID1", "PASSWORD2"},
  {"SSID2", "PASSWORD2"},
};
*/

ESP8266WiFiMulti WiFiMulti;

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
  for (int i=0; i < sizeof(stassid_list)/sizeof(stassid_t); i++) {
    WiFiMulti.addAP(stassid_list[i].ssid, stassid_list[i].pk);
  }  
  Serial.println("");

  // Init connection database
  for (int i=0; i < (sizeof(tally_clients)/sizeof(struct tally_client_t)); i++) {
    tally_clients[i].expire_time = 0;
  }

  // Connect WiFi
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);    
  }
  
  Serial.println("");
  Serial.print("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("WiFiTally")) {
    Serial.println("MDNS responder started");
  }

  Udp.begin(UDP_PORT);

  MDNS.addService("tally", "udp", 80);
  Serial.println("Tally UDP server started");
}

void loop(void) {
  MDNS.update();
  
  // Connect WiFi
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);    
  } 
  
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
