#include <WiFi.h>
#include "esp_wifi.h"


//Libraries
#include <WiFi.h>



//Parameters
String nom = "Master";
const char* ssid = "NORDLI-ESP32";
const char* password = "nordli-esp32";
int i = 0; 

//Variables
bool sendCmd = false;
String slaveCmd = "0";
String slaveState = "0";

//Objects

WiFiClient browser;
IPAddress ip(192, 168, 0, 1);
//IPAddress gateway(192, 168, 1, 254);
//IPAddress subnet(255, 255, 255, 0);
 
uint8_t packet[4096];


void setup() {
 	//Init Serial USB
 	Serial.begin(9600);
 	Serial.println("Initialize System");
  //Serial.print(packet); 
 	//Init ESP32 Wifi
 	WiFi.mode(WIFI_STA);
 	WiFi.begin(ssid, password);

 	while (WiFi.status() != WL_CONNECTED) {
 		delay(500);
 		Serial.print(F("."));
 	}
}
void loop() {
 	clientRequest();
}
void clientRequest( ) { /* function clientRequest */
 	////Check if client connected
 	WiFiClient client ;
  	client.connect(ip,4242);
 	client.setTimeout(50);
 	if (client) {
 			while (client.connected()) {
 					
 					client.write(packet, 4096);
 					client.flush();
          
 			}
 	}
}

void check_protocol(){
  uint8_t value; 
  esp_wifi_get_protocol(WIFI_IF_STA, &value);

  Serial.print(value); 
  Serial.print(" ");
  Serial.print(value == WIFI_PROTOCOL_11G);
  Serial.println();
}

