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
  /*
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;

    Serial.println("New Client.");          // print a message out in the serial port
    String request = "";    
    
    int moistureLevel = 0;
    //&& currentTime - previousTime <= timeout
    while (client.connected() ) {  // loop while the client's connected
      currentTime = millis();

      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read()   ;  
        request += c; // Read the HTTP request
             
        if (c=='\n')
        {
          //checks if the http request is for the live data or the HTML Page
          if (request.indexOf("/live") >= 0) {
            //Sensor Data reading
            //if conditional to only take a reading every 3 seconds without blocking the html webpage loop
            if (millis() -sensorLastOn>= 3000){
              sensorLastOn = millis();
              moistureLevel = mapReadings(readSensor());
            }

            //header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            //data
            client.print(moistureLevel); // Send live data as plain text
            break;
          }
          else
          {
            //Header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            //Main HTML page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("body { font-family: Arial, sans-serif; text-align: center; } h1 { color: #0f3376; } ");
            client.println(".sensor {margin: 20px;padding: 10px; border: 1px solid #ccc; display: inline-block;</style>");
            //Script for fetching live data
            client.println("<script>");
            client.println("function fetchData() {");
            client.println("  fetch('/live')"); // AJAX request to fetch live data
            client.println("    .then(response => response.text())");
            client.println("    .then(data => {");
            client.println("      document.getElementById('sensor1').innerText = data + '%';");
            client.println("    });");
            client.println("}");
            client.println("setInterval(fetchData, 2000);"); // Update every 2 seconds
            client.println("</script>");
            client.println("</head>");
            //HTML Body
            client.println("<body>");
            client.println("<h1>ESP32 Moisture Sensor</h1>");
            client.println("<div class=\"sensor\">");
            client.println("  <h2>Sensor 1</h2>");
            client.println("  <p>Moisture Level: <span id=\"sensor1\">Loading...</span></p>");
            client.println("  <button id=\"button-sensor1\" class=\"button\" onclick=\toggleSensor('sensor1')\">Turn Off</button> </div>");
            client.println("</body></html>");
            break;
          }
        }
      } 
    } 
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  if (millis() - sensorLastOn >= 3000){
    sensorLastOn = millis();
    moistureLevel = mapReadings(readSensor());
  }*/

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
