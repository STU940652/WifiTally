/**
   reuseConnection.ino

    Created on: 22.11.2015

*/


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#define HOSTNAME "CAMERA1"
#define TARGET_HOSTNAME "WiFiTally.local"
// #define DEBUG
#define UDP_PORT 80

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

IPAddress server_ip;
uint16_t server_port=80;
bool server_ready = false;

WiFiUDP Udp;
unsigned long refresh_time = 0;
// Refresh every 5 minutes
// Expires after 2 hours
#define REFRESH_PERIOD (5 * 60 * 1000)

char incomingPacket[255];  // buffer for incoming packets


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
  for (int i=0; i < sizeof(stassid_list)/sizeof(stassid_t); i++) {
    WiFiMulti.addAP(stassid_list[i].ssid, stassid_list[i].pk);
  }
  
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

   Udp.begin(UDP_PORT);
}


void loop() {
  // Connect WiFi
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    server_ready = false;
    digitalWrite(LED_BUILTIN, 0);
    delay(200);
    digitalWrite(LED_BUILTIN, 1);
    delay(200);    
  }  

  if (!server_ready) {
    if(resolve_mdns_service("tally", "udp", TARGET_HOSTNAME, &server_ip, &server_port)) {
      Serial.printf("got an answer for %s.local!\n", TARGET_HOSTNAME);
      Serial.println(server_ip);
      Serial.println(server_ip);
      refresh_time = 0;
      server_ready = true;
    } else {
      digitalWrite(LED_BUILTIN, 0);
      delay(100);
      digitalWrite(LED_BUILTIN, 1);
      delay(100);  
      digitalWrite(LED_BUILTIN, 0);
      delay(100);
      digitalWrite(LED_BUILTIN, 1);
      delay(100);    
      Serial.printf("Sorry, %s not found\n", TARGET_HOSTNAME);
      return;
    }    
  }
    
  if ((WiFiMulti.run() == WL_CONNECTED) && server_ready) {
    if (millis() > refresh_time) {
      Serial.printf("Refreshing server registration\n"); 
      Udp.beginPacket(server_ip, server_port);
      Udp.write(String(String(HOSTNAME)+String("\n")).c_str());
      Udp.endPacket();
      // Next Refresh
      refresh_time = millis() + REFRESH_PERIOD;
    }
  }

  /* Check for incoming messages */
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    //Serial.printf("UDP packet contents: %s\n", incomingPacket);
    if (incomingPacket[0] == '1'){
        digitalWrite(LED_BUILTIN, 0);
      } else {
        digitalWrite(LED_BUILTIN, 1);    
    }
  }
 
  delay(10);
}
