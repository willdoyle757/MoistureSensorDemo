#include <Arduino.h>
#include <WiFi.h>
#include <config.h>

const int analogPin = 32;
const int powerPin = 12; //Power pin so that the sensor is only powered when taking reading due to short sensor lifespan

WiFiServer server(80);

//http request
String header;

int readSensor();
int mapReadings(int);

unsigned long currentTime = millis();

unsigned long previousTime = 0;
unsigned long sensorLastOn = 0;

const long timeout = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(powerPin, OUTPUT); //Set pin for power

  digitalWrite(powerPin, LOW);

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
  
}

void loop() {
  
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

            //if conditional to only take a reading every 2 seconds without blocking the html webpage loop
            if (sensorLastOn - millis() >= 5000){
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
            client.println("body { margin-top: 50px; } h1 { color: #0f3376; } </style>");
            //Script for fetching live data
            client.println("<script>");
            client.println("function fetchData() {");
            client.println("  fetch('/live')"); // AJAX request to fetch live data
            client.println("    .then(response => response.text())");
            client.println("    .then(data => {");
            client.println("      document.getElementById('moisture').innerText = data + '%';");
            client.println("    });");
            client.println("}");
            client.println("setInterval(fetchData, 2000);"); // Update every 2 seconds
            client.println("</script>");
            client.println("</head>");
            //HTML Body
            client.println("<body>");
            client.println("<h1>ESP32 Moisture Sensor</h1>");
            client.println("<p>Moisture Level: <span id=\"moisture\">Loading...</span></p>");
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
}

int readSensor()
{
  digitalWrite(powerPin, HIGH);
  delay(15);
  int val = analogRead(analogPin);
  digitalWrite(powerPin, LOW);
  
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

// No moisture is higher analog value, high moisture is low analog value
