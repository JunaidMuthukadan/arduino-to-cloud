#include <UbidotsESP8266.h>
#include <SoftwareSerial.h> 

#define SSID "Jund"
#define PASS "$7426274262"

#define TOKEN "wWcudFMmbtCK3QBoPwoQUlKYNMRMm7"
#define ID "575e73b076254201f7b46502"

Ubidots client(TOKEN);

void setup() {
  Serial.begin(9600);
  client.wifiConnection(SSID,PASS);
}

void loop() {
  float value = 10;
  client.add(ID,value);
  client.sendAll();
}
