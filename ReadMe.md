Tally Light system using Arduio ESP8266. This consists of a server unit that reads the tally relays from a video switcher, and client units that recieve the tally information over the network connection and illuminate the on-board LED when the camera is live.

# Hardware
- KeeYees ESP8266 NodeMCU
  - [Amazon Link](https://www.amazon.com/gp/product/B07S5Z3VYZ)
  - Need one for the master and one each per camera
- Battery pack (optional)
  - Makes camera unit totally wireless
  - Use 3 AAA batteries
  - [Amazon Link](https://www.amazon.com/gp/product/B07D26XFJW)
 
# Prerequisites
Install ESP8266 library. Instructions from the [Amazon Link](https://www.amazon.com/gp/product/B07S5Z3VYZ)

1. Set up your Arduino IDE as: 
   - Go to File->Preferences and copy the URL below to get the ESP board manager extensions
   - http://arduino.esp8266.com/stable/package_esp8266com_index.json
   - Place the “http://” before the URL and let the Arduino IDE use it...otherwise it gives you a protocol error.
2. Go to Tools > Board > Board Manager> Type "esp8266" and download the Community esp8266 and install. 
3. Set up your chip:
   - Tools -> Board -> NodeMCU 1.0 (ESP-12E Module)
   - Tools -> Flash Size -> 4M (3M SPIFFS)
   - Tools -> CPU Frequency -> 80 Mhz
   - Tools -> Upload Speed -> 921600
   - Tools ->Port -> (whatever it is)
   
# Usage
## ssid.h
For both the server and the clients, put a **ssid.h** file in the same directory as the sketch, with the SSID(s) and password(s) of the WiFi network(s) you would like to use. The contents of **ssid.h** is below:

```
struct stassid_t stassid_list[] = 
{
  {"SSID1", "PASSWORD2"},
  {"SSID2", "PASSWORD2"},
};
```
## Clients
Each of the clients must have a unique hostname consisting of 'CAMERA' and a number. Change the line below in the sketch prior to programming each client with the correct client number
```
#define HOSTNAME "CAMERA1"
```
## Server
The server is connected to the tally relays through GPIO. The relays should e connected such that the inputs are connected to ground when the relay closes. Channels are shown below:
| Camera | GPIO Number | Pin Label |
|--------|-------------|-----------|
|    1   |      5      |   D1      |
|    2   |      4      |   D2      |
|    3   |      0      |   D3      |
|    4   |     14      |   D5      |
|    5   |     12      |   D6      |
|    6   |     13      |   D7      |

# Operation Overview
The server uses mDNS to advertise a hostname of "WiFiTally", which the clients use to find the server's IP address. The client sends a UDP packet to port 80 of the server with it's hostname (e.g. "CAMERA1"). The server then registers the IP address of the client with that tally input.
Every 100ms, the server checks the tally GPIO, and sends an update to each of of the registered clients. A UDP packet is sent to port 80 of the client with a '1' to light the LED or a '0' to turn the LED off.
The server expires the registration every 2 hours. Clients re-register every 5 minutes.

# Known Issues
1. The expiration timer rolls over every 49.7 days. Strange things may happen around that time.

