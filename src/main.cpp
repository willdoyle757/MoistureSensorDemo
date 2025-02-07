#include <Arduino.h>
#include <WiFi.h>
#include <config.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "website.h"

const int sensor1DataPin = 32;//32 is analog
const int sensor1PowerPin = 12; //Power pin so that the sensor is only powered when taking reading due to short sensor lifespan

const int sensor2DataPin = 35;
const int sensor2PowerPin = 14;

//Where new pins for sensors are added
int dataPins[] = {sensor1DataPin, sensor2DataPin};
int powerPins[] = {sensor1PowerPin, sensor2PowerPin};
String plantNames[] = {"plant1", "plant2"};

int size = sizeof(dataPins)/sizeof(dataPins[0]);


WebServer server(80);

//http request
String header;

int readSensor(int dataPin, int powerPin);
int mapReadings(int);
void sendWebsite();
void sendData();
void sendDataToFirebase(const String& path, const String& jsonData);
String packageData(int dataPins[], int powerPins[], String plantNames[], int size);


int moistureLevel = 0;

unsigned long previousTime = 0;
unsigned long currentTime = 0;

const long timeout = 2000;

WiFiClientSecure client;

void setup() {

  Serial.begin(9600);

  //goes through sensors to initialize pins
  
  for (int i = 0; i < size; i++){
    pinMode(dataPins[i], INPUT);
    pinMode(powerPins[i], OUTPUT);
    digitalWrite(powerPins[i], LOW);
  }
  

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
    int moistureLevel = mapReadings(readSensor(sensor1DataPin, sensor1PowerPin));
    String jsonData = packageData(dataPins, powerPins, plantNames, size);
      
    sendDataToFirebase("Sensors", jsonData);
    previousTime = millis();
  }

  //For handling local connection on LAN
  server.handleClient();
}

//Sends Json to the google firebase
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

//
int readSensor(int dataPin, int powerPin)
{
  digitalWrite(powerPin, HIGH);
  delay(15);
  int val = analogRead(dataPin);
  digitalWrite(powerPin, LOW);
  return val;
}

int mapReadings(int val){
  //Serial.print("Data: ");
  //Serial.println(val);
  if (val == -1){
    return -40;
  }
  return (int)(((float)val - 4095) * ((100.0) / (1000.0 - 4095.0)));
}

String packageData(int dataPins[], int powerPins[], String plantNames[], int size){
  
  int n = size;
  String jsonData = "{";
  for (int i = 0; i < n; i++){
    int moistureLevel = mapReadings(readSensor(dataPins[i], powerPins[i]));

    jsonData+="\"sensor" + String(i+1) + "\": {\"data\": "+ String(moistureLevel) + "," + "\"name\": " + "\"" + plantNames[i] + "\"}";

    if (i != n - 1){
      jsonData+=",";
    }
    
  }
  jsonData +="}";
  Serial.println(jsonData);
  return jsonData;
}

/*
{"
"\"sensor1\": {\"data\": 0, \"name\": \"plant1\"}, "
"\"sensor2\": {\"data\": 131, \"name\": \"plant2\"}"
"}";
*/

//Sends data to the local client on lan
void sendData(){
  int moistureLevel = mapReadings(readSensor(sensor1DataPin, sensor1PowerPin));
  String jsonData = "{\"sensor1\": " + String(moistureLevel) + "}";

  server.send(200, "application/json", jsonData);
}

//sends website to local client on lan
void sendWebsite() {

  Serial.println("sending web page");
  
  server.send(200, "text/html", PAGE_MAIN);
}

