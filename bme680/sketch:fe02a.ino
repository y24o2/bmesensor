#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SEALEVELPRESSURE_HPA (1013.25)

const char* ssid = "WIFI SSID";
const char* password = "WIFI PASSWORD";
unsigned long delayTime;

// Weather
float h, t, p;
uint32_t g;
char temperatureCString[6];
char humidityString[6];
char pressureString[7];
char gasString[7];
void getWeather();

// BME280 -- VIN 3,3V, G GND, D1 SCL, D2 SDA
Adafruit_BME680 bme;

// Relais -- IN D7
char state[4];
void setOn();
void setOff();
void getInfo();

// Webserver
ESP8266WebServer server(80);

void handleRoot();
void handleOn();
void handleOff();
void handleToggle();
void handleGet();
void handleNotFound();

void setup() {
  Serial.begin(115200);
  Wire.setClock(10000);
  delay(10);

  // Relais
  pinMode(D7, OUTPUT);

  Serial.println(F("BME680 json & Pin Set"));

  bool status;
  status = bme.begin(0x77);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Setup WebServer
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/toggle", handleToggle);
  server.on("/get", handleGet);
  server.onNotFound(handleNotFound);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}

void getWeather() {
  Serial.println();
  Serial.println("Weather request");
  h = bme.readHumidity();
  t = bme.readTemperature();
  p = bme.readPressure() / 100.0;
  g = bme.readGas();
  dtostrf(t, 5, 1, temperatureCString);
  dtostrf(h, 5, 1, humidityString);
  dtostrf(p, 6, 1, pressureString);
  dtostrf(g, 6, 1, gasString);
  delay(100);
}

void handleRoot() {
  getWeather();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"temperature\": " + String(temperatureCString) +", \"humidity\": " + String(humidityString) + ", \"pressure\": " + String(pressureString) + ", \"gas\": " + String(gasString) + "}");
}

void handleOn(){
  Serial.println();
  Serial.println("R_IN HIGH");
  digitalWrite(D7, HIGH);
  delay(100);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"state\": 1}");
}

void handleOff(){
  Serial.println();
  Serial.println("R_IN LOW");
  digitalWrite(D7, LOW);
  delay(100);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", "{\"state\": 0}");
}

void handleToggle(){
  if(digitalRead(D7) == HIGH)
    handleOff();
  else
    handleOn();
}

void handleGet(){
  server.sendHeader("Access-Control-Allow-Origin", "*");
  if(digitalRead(D7) == HIGH)
    server.send(200, "application/json", "{\"state\": 1}");
  else
    server.send(200, "application/json", "{\"state\": 0}");
}

void handleNotFound(){
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(404, "text/plain", "404: Not found");
}
