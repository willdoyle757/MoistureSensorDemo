#include <Arduino.h>
#include <WiFi.h>
#include <config.h>
#include <WebServer.h>
#include "website.h"

const int sensorDataPin1 = 32;//32 is analog
const int sensorPowerPin1 = 12; //Power pin so that the sensor is only powered when taking reading due to short sensor lifespan



WebServer server(80);

//http request
String header;

int readSensor();
int mapReadings(int);
void sendWebsite();
void sendData();

unsigned long currentTime = millis();

int moistureLevel = 0;

unsigned long previousTime = 0;
unsigned long sensorLastOn = 0;

const long timeout = 2000;


void setup() {

  Serial.begin(9600);

  pinMode(sensorDataPin1, INPUT);
  pinMode(sensorPowerPin1, OUTPUT); //Set pin for power

  digitalWrite(sensorPowerPin1, LOW);

  //Wifi username and password define in seperate config.h file
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  
  server.on("/", sendWebsite);
  server.on("/live", sendData);
}

void loop() {
 server.handleClient();
}

int readSensor()
{
  digitalWrite(sensorPowerPin1, HIGH);
  delay(15);
  int val = analogRead(sensorDataPin1);
  digitalWrite(sensorPowerPin1, LOW);
  return val;
}

int mapReadings(int val){
  //Serial.print("Data: ");
  //Serial.println(val);
  if (val == -1){
    return -40;
  }
  return (int)(((float)val - 4095) * ((100.0)/ (1000.0 - 4095.0)));
}

void sendData(){
  int moistureLevel = mapReadings(readSensor());
  String jsonData = "{\"sensor1\": " + String(moistureLevel) + "}";

  server.send(200, "application/json", jsonData);
}

void sendWebsite() {

  Serial.println("sending web page");
  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/html", PAGE_MAIN);
}

// No moisture is higher analog value, high moisture is low analog value
