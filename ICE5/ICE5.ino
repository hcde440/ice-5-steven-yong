//I worked with Wesley.
//https://github.com/hcde440/ice-5-LuffyWesley

//Include all the libraries for the microcontroller, sensors, and MQTT.
#include "config.h"
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MPL115A2.h>
#include <Adafruit_Sensor.h>

//Create instances of the sensors and execute the clients.
#define DATA_PIN 12
Adafruit_MPL115A2 mpl115a2;
DHT_Unified dht(DATA_PIN, DHT22);
WiFiClient espClient;
PubSubClient mqtt(espClient);

//Use the unique MAC address of the microcontroller as an identifier when connecting to MQTT.
char mac[6];

//Create a character array to store the JSON data to be sent to MQTT.
char message[201];

//Initialize serial and keep looping until it connects, then execute the setup_wifi function. Connect to MQTT and initialize the sensors.
void setup() {
  Serial.begin(115200);
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));
  
  while(! Serial);
  
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  
  dht.begin();
  mpl115a2.begin();
}

//Connect to the internet and display messages to help debug issues.
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println(WiFi.macAddress());
}

//Reconnect to MQTT if it gets disconnected; subscribe to a topic.
void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      mqtt.subscribe("Treasure/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//Maintain connection to MQTT and get values from the sensors. Ttore the sensor values in character arrays, convert to string, and publish to MQTT in JSON.
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();
  
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  dht.temperature().getEvent(&event);
  float celcius = event.temperature;
  float fahrenheit = (event.temperature * 1.8) + 32;
  float pressure = mpl115a2.getPressure();
  float humidity = event.relative_humidity;
  
  char fah[6];
  char cel[6];
  char pres[7];
  char hum[6];
  
  dtostrf(pressure, 6, 2, pres);
  dtostrf(humidity, 5, 2, hum);
  dtostrf(fahrenheit, 5, 2, fah);
  dtostrf(celcius, 5, 2, cel);
  sprintf(message, "{\"Temperature (F)\":\"%s\", \"Pressure\":\"%s\", \"Humidity\":\"%s\"}", fah, pres, hum);
  mqtt.publish("Treasure/Fluke", message);
  
  delay(5000);
}
