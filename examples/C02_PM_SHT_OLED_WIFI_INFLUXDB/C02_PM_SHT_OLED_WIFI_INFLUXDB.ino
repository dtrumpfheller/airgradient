/*
This is the code for the AirGradient DIY Air Quality Sensor with an ESP8266 Microcontroller.

It is a high quality sensor showing PM2.5, CO2, Temperature and Humidity on a small display and can send data over Wifi.

For build instructions please visit https://www.airgradient.com/diy/

Compatible with the following sensors:
Plantower PMS5003 (Fine Particle Sensor)
SenseAir S8 (CO2 Sensor)
SHT30/31 (Temperature/Humidity Sensor)

Please install ESP8266 board manager (tested with version 3.0.0)

The codes needs the following libraries installed:
"WifiManager by tzapu, tablatronix" tested with Version 2.0.3-alpha
"ESP8266 and ESP32 OLED driver for SSD1306 displays by ThingPulse, Fabrice Weinberg" tested with Version 4.1.0

Configuration:
Please set in the code below which sensor you are using and if you want to connect it to WiFi.

If you are a school or university contact us for a free trial on the AirGradient platform.
https://www.airgradient.com/schools/

MIT License
*/

#include <AirGradient.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include "SSD1306Wire.h"

AirGradient ag = AirGradient();

SSD1306Wire display(0x3c, SDA, SCL);

// set sensors that you do not use to false
const boolean hasPM=true;
const boolean hasCO2=true;
const boolean hasSHT=true;

// set to true if you want to connect to wifi. The display will show values only when the sensor has wifi connection
const boolean connectWIFI = true;
const char* WIFI_SSID = "SSID";
const char* WIFI_PASSWORD = "PASSWORD";

// change if you want to send the data to another server
const String APIROOT = "http://192.168.1.1:9086";
const String ORGANIZATION = "home";
const String BUCKET = "airgradient";
const String TOKEN = "TOKEN";
const String MEASUREMENT = "airgradient";
const String TAGS = "location=Living\\ Room";

void setup(){
  Serial.begin(9600);

  display.init();
  display.flipScreenVertically();
  showTextRectangle("Init", String(ESP.getChipId(),HEX),2);

  if (hasPM) ag.PMS_Init();
  if (hasCO2) ag.CO2_Init();
  if (hasSHT) ag.TMP_RH_Init(0x44);

  if (connectWIFI) connectToWifi();
  delay(2000);
}

void loop(){

  // create payload
  String payload = MEASUREMENT + ",id=" + String(ESP.getChipId(), HEX) + "," + TAGS + " ";

  if (hasPM) {
    AirGradient::DATA pm = ag.getPM_Raw();

    int PM1 = pm.PM_AE_UG_1_0;
    payload += "pm1=" + String(PM1) + ",";
    showTextRectangle("PM1", String(PM1), 3);
    delay(3000);

    int PM2 = pm.PM_AE_UG_2_5;
    payload += "pm2.5=" + String(PM2) + ",";
    showTextRectangle("PM2", String(PM2), 3);
    delay(3000);

    int PM10 = pm.PM_AE_UG_10_0;
    payload += "pm10=" + String(PM10) + ",";
    showTextRectangle("PM10", String(PM10), 3);
    delay(3000);
  }

  if (hasCO2) {
    int CO2 = ag.getCO2_Raw();
    payload += "co2=" + String(CO2) + ",";
    showTextRectangle("CO2", String(CO2), 3);
    delay(3000);
  }

  if (hasSHT) {
    TMP_RH result = ag.periodicFetchData();
    payload += "temperature=" + String(result.t) + ",";
    payload += "humidity=" + String(result.rh);
    showTextRectangle(String(result.t), String(result.rh) + "%", 3);
    delay(3000);
  }

  if (connectWIFI){
    // send payload
    String POSTURL = APIROOT + "/api/v2/write?org=" + ORGANIZATION + "&bucket=" + BUCKET + "&precision=s";

    Serial.print("Request: ");
    Serial.print(POSTURL);
    Serial.print('\n');
    Serial.print("Payload: ");
    Serial.print(payload);
    Serial.print('\n');

    WiFiClient client;
    HTTPClient http;
    http.begin(client, POSTURL);
    http.addHeader("Authorization", "Token " + TOKEN);
    int httpCode = http.POST(payload);
    String response = http.getString();
    http.end();

    Serial.print("HTTP Status: ");
    Serial.print(httpCode);
    Serial.print('\n');
    Serial.print("Response: ");
    Serial.print(response);
    Serial.println('\n');
  }
}

void connectToWifi(){
  WiFi.hostname("AirGradient");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print(" ");
  showTextRectangle("Connecting to", WIFI_SSID, 1);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnection established!");
  Serial.print("IP address: \t");
  Serial.println(WiFi.localIP());
  Serial.print('\n');
}

void showTextRectangle(String ln1, String ln2, int size) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  if (size == 1) {
    display.setFont(ArialMT_Plain_10);
  } else if (size == 2) {
    display.setFont(ArialMT_Plain_16);
  } else {
    display.setFont(ArialMT_Plain_24);
  }
  display.drawString(32, 16, ln1);
  display.drawString(32, 36, ln2);
  display.display();
}
