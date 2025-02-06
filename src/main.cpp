#include <Arduino.h>
#include <WiFi.h>
#include <config.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>//old use
#include <HTTPClient.h>
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
void sendDataToFirebase(const String& path, const String& jsonData);


int moistureLevel = 0;

unsigned long previousTime = 0;
unsigned long currentTime = 0;

const long timeout = 2000;

WiFiClientSecure client;

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
  //server.on("/live", sendData);
  server.on("/live", sendData);


  client.setInsecure(); 

}

//todo figure out how to connect to google firebase
void loop() {


  currentTime = millis();

  if (currentTime - previousTime >= 10000){
    int moistureLevel = mapReadings(readSensor());
    String jsonData = "{\"sensor1\": " + String(moistureLevel) + "}";
    sendDataToFirebase("Sensors", jsonData);
    previousTime = millis();
  }

  //For handling local connection on LAN
  server.handleClient();
}


void sendDataToFirebase(const String& path, const String& jsonData)
{
  HTTPClient http;

  Serial.println("Connecting to Firebase...");

  String url = String(FIREBASE_URL) + "/" + path + ".json?auth=" + String(API_KEY);

  Serial.print("Requesting URL: ");
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.PUT(jsonData);
  
  if (httpResponseCode > 0){
    Serial.println("Response code: " + String(httpResponseCode));
    Serial.println(http.getString());
  }
  else{
    Serial.println("Error in sending request");
    Serial.println(httpResponseCode);
  }

  http.end();

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
