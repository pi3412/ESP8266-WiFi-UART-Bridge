// ESP8266 WiFi <-> UART Bridge
// by RoboRemo
// www.roboremo.com

// Disclaimer: Don't use RoboRemo for life support systems
// or any other situations where system failure may affect
// user or environmental safety.

#include <ESP8266WiFi.h>


// config: ////////////////////////////////////////////////////////////

#define UART_BAUD 9600
#define packTimeout 5 // ms (if nothing more on UART, then send packet)
#define bufferSize 8192

// For STATION mode:
const char *ssid = "myrouter";  // Your ROUTER SSID
const char *pw = "password"; // and WiFi PASSWORD
const int port = 9876;

//////////////////////////////////////////////////////////////////////////


#include <WiFiUdp.h>
WiFiUDP udp;
IPAddress remoteIp;


uint8_t buf1[bufferSize];
uint8_t i1=0;

uint8_t buf2[bufferSize];
uint8_t i2=0;



void setup() {

  delay(500);
  
  Serial.begin(UART_BAUD);


  // STATION mode (ESP connects to router and gets an IP)

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }

  Serial.println("Starting UDP Server");
  udp.begin(port); // start UDP server 

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
