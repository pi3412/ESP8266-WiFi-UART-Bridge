// ESP8266 WiFi <-> UART Bridge
// by RoboRemo
// www.roboremo.com

// Disclaimer: Don't use this application for life support systems,
// navigation or any other situations where system failure may affect
// user or environmental safety.

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// config: ////////////////////////////////////////////////////////////

#define GPS_BAUD 9600
#define PLOTTER_BAUD 4800
#define packTimeout 5 // ms (if nothing more on UART, then send packet)
#define bufferSize 8192

//#define STATIC_IP_ADDR

// For STATION mode:
const char *ssid = "myrouter";  // Your ROUTER SSID
const char *pw = "password"; // and WiFi PASSWORD
const int port = 10120; // 10110 is official TCP and UDP NMEA 0183 Navigational Data Port

//////////////////////////////////////////////////////////////////////////

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

IPAddress remoteIp;

uint8_t buf1[bufferSize];
uint8_t i1=0;

uint8_t buf2[bufferSize];
uint8_t i2=0;

#ifdef STATIC_IP_ADDR
IPAddress staticIP(192,168,0,25);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);
#endif


void setup() {

  delay(500);
  
   // start 1st serial connection to dump WIFI status messages over USB
  Serial.begin(GPS_BAUD);
  
  // start 2nd serial connection to GPS Plotter via pin GPIO2 (no receive possible)
  Serial1.begin(PLOTTER_BAUD); 

  // STATION mode (ESP connects to router and gets an IP)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  #ifdef STATIC_IP_ADDR
  WiFi.config(staticIP, gateway, subnet);
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.printf("WiFi connected with IP address: ");  
  Serial.println(WiFi.localIP());
  udp.begin(port); // start UDP server 

  // swap serial port from USB to attached GPS: GPIO15 (TX) and GPIO13 (RX)
  Serial.swap();
}


void loop() {

  // if there’s data available, read a packet
  int packetSize = udp.parsePacket();
  if(packetSize>0) {
    remoteIp = udp.remoteIP(); // store the ip of the remote device
    udp.read(buf1, bufferSize);
    // now send to UART:
    Serial.write(buf1, packetSize);
  }

  if(Serial.available()) {

    // read the data until pause:
    //Serial.println("sa");
    
    while(1) {
      if(Serial.available()) {
        buf2[i2] = (char)Serial.read(); // read char from UART
        if(i2<bufferSize-1) {
          i2++;
        }
      } else {
        //delayMicroseconds(packTimeoutMicros);
        //Serial.println("dl");
        delay(packTimeout);
        if(!Serial.available()) {
          //Serial.println("bk");
          break;
        }
      }
    }

    // now send to WiFi:  
    udp.beginPacket(remoteIp, port); // remote IP and port
    udp.write(buf2, i2);
    udp.endPacket();
    i2 = 0;
  }
  
}
