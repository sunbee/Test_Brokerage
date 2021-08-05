#include <Arduino.h>

String nodeName = "BHRIGU"; // 6 char EXACTLY

/*
Obtain sensor readings using the following:
1. TSL2591 sensor for brightness
2. MCP3008 for analogue sensors - water level
3. SSD1306 OLED display at 0x39
and publish to MQTT broker.
*/
#include "PinNumbers.h"
#include "SensorArray.h"

SensorArray _sensorArray; // Array of sensors

uint16_t brightness = 0;    // Value read from lux sensor
int waterLevel = 0;         // Value read from water-level sensor
int potValue;               // Value read from the pot
float tempInC;              // Temperature read from ds18b20 "Dallas"
float RH;                   // Relative Humidity read from DHT22
float TempF;                // Temperature Fahrenheit reported from DHT22
long unsigned pubInterval;  // Time interval for recurring writes to broker

/*
Connect to MQTT broker over WiFi (Home Internet).
This section has the libraries to connect to WiFi and the broker.
Note that we will use a single nodemcu as publisher and subscriber,
with a single instance of the Mosquitto client object.
Adjust TIMER_INTERVAL (in milliseconds) for publishing frequency.
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <SECRETS.h>
#define TIMER_INTERVAL  6000     // 6 seconds
#define TIMER_MIN       2000     // 2 seconds MIN
#define TIMER_MAX       180000   // 3 minutes MAX

const char* SSID = SECRET_SSID;
const char* PASS = SECRET_PASS;
char broker_ip[] = SECRET_BROKER_IP;
unsigned long tic = millis();

WiFiClient WiFiClient;
PubSubClient MosquittoClient(WiFiClient);

void mosquittoDo(char* topic, byte* payload, unsigned int length) {
  /*
  Handle a new message published to the subscribed topic on the 
  MQTT broker and show in the OLED display.
  This is the heart of the subscriber, making it so the nodemcu
  can act upon information, say, to operate a solenoid valve when
  the mositure sensor indicates dry soil.
  */
  Serial.print("Got a message in topic ");
  Serial.println(topic);
  Serial.print("Received data: ");
  char message2display[length];
  for (unsigned int i = 0; i < length; i++) {
    Serial.print(char(payload[i]));
    message2display[i] = payload[i];
  }
  Serial.println();
  char message4OLED[length+6]; // 'Got: ' and null terminator.
  snprintf(message4OLED, length+6, "Got: %s", message2display); 
  _sensorArray.displayMessage(message4OLED);
}

void setup() {
  // Start the serial bus
  Serial.begin(9600);
  while (!Serial) {
    // Stabilize the serial commuincations bus.
  }
  delay(999);
  // Start the humidity/temperature sensor (DHT22)
  _sensorArray.start_dht22();
  // Start the temperature sensor (Dallas)
  _sensorArray.start_ds18b20(DALLAS_TEMPERATURE);
  // Start the lux sensor
  _sensorArray.start_tsl();
  // Start the ADC
  _sensorArray.start_mcp();
  // Start the display at I2C addr 0x3C
  _sensorArray.start_display();

  pinMode(onboard_led, OUTPUT);

  // Connect to WiFi:
  WiFi.mode(WIFI_OFF);
  delay(1500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(699);
    Serial.print(".");
  }
  Serial.print("Connected: ");
  Serial.println(SSID);

  /* 
  Configure the Mosquitto client with the particulars 
  of the MQTT broker. The client is dual-use as publisher
  and subscriber.
  In order to publish, use the 'publish()' method of the 
  client object. The message will be serialized JSON 
  containing sensor readings. 
  In order to listen, subscribe to the topic of interest
  and handle the me  unsigned long timerInterval = TIMER_INTERVAL;ssage with the callback in event-driven 
  pattern. 
  */
  MosquittoClient.setServer(broker_ip, 1883);
  MosquittoClient.setCallback(mosquittoDo);
}

void reconnect() {
  /*
  Connect to the MQTT broker in order to publish a message
  or listen in on a topic of interest. 
  The 'connect()' m  unsigned long timerInterval = TIMER_INTERVAL;ethods wants client credentials. 
  When the MQTT broker is not setup for authentication, 
  we have successfully connected to the MQTT broker 
  passing string literals for args 'id' and 'user' and NULL for 'pass'.
  Having connected successully, proceed to publish or listen.
  Use connection string as follows:
  1. "BHRIGU", "fire_up_your_neurons", NULL
  2. "VASISH", "whatsup_nerds", NULL
  */
  while (!MosquittoClient.connected()) {
    if (MosquittoClient.connect("BHRIGU", "fire_up_your_neurons", NULL)) {
      Serial.println("Uh-Kay!");
      MosquittoClient.subscribe("Test"); // SUBSCRIBE TO TOPIC
    } else {
      Serial.print("Retrying ");
      Serial.println(broker_ip);
      delay(699);
    }
  }
}

String makeMessage() {
  /*
  Read the pot on A0. 
  */
  potValue = analogRead(PIN_POT);
  char potDisplay[7];
  dtostrf(potValue, 4, 0, potDisplay);
  /*
  Read the water level on analog channel no. 0 of the MCP3008 10-bit ADC. 
  */
  waterLevel = _sensorArray.get_mcp_waterLevel(PIN_WATER_LEVEL, true);
  char waterLevelDisplay[7];
  dtostrf(waterLevel, 4, 0, waterLevelDisplay);
  /*
  Read the brightness level reported by TSL2591X on I2C bus.
  */
  brightness = _sensorArray.get_tsl_lux();
  char brightnessDisplay[7];
  dtostrf(brightness, 6, 2, brightnessDisplay);
  /*
  Read the temperature reported by dsa8b20 "Dallas" on one-wire.
  */
  tempInC = _sensorArray.get_ds18b20_temperature(CELSIUS);
  char tempInCDisplay[7];
  dtostrf(tempInC, 6, 2, tempInCDisplay); 
  /*
  Read the relative humidity and temperature reported by DHT22.
  */
  RH = _sensorArray.get_dht_humidity();
  TempF = _sensorArray.get_dht_temperature(FAHRENHEIT);
  if (isnan(RH) || isnan(TempF)) {
    RH = -99.99;
    TempF = -99.99;
  }
  char RHDisplay[7];
  dtostrf(RH, 6, 2, RHDisplay);
  /*
  Make the message to publish to the MQTT broker as
  serialized JSON. 
  */
  char readOut[90];
  snprintf(readOut, 90, "{\"Name\":\"%6s\",\"Pot\":%6s,\"Level\":%6s,\"Lux\":%6s,\"TempC\":%6s,\"RH\":%6s}", nodeName.c_str(), potDisplay, waterLevelDisplay, brightnessDisplay, tempInCDisplay, RHDisplay);
  _sensorArray.displayMessage(readOut);
  return readOut; // Note 128 char limit on messages.
}

void publish_message() {
  String msg_payload = makeMessage(); // "Namaste from India";
  char char_buffer[128];
  msg_payload.toCharArray(char_buffer, 128);
  MosquittoClient.publish("Test", char_buffer);
}

void loop() {
  unsigned long toc = millis();
  /*
    Lights! Camera! Action!
    Here is where the action is for publisher and subscriber.
    Note the use of millis to scheduling the publication of
    sensor readings to the MQTT broker in a non-blocking way. 
    The use of 'delay()' would block the listener, 
    causing events to be missed.  
  */
  digitalWrite(onboard_led, LOW);  
  pubInterval = map(potValue, 0, 1023, TIMER_MIN, TIMER_MAX);
  if (toc - tic > pubInterval) {
    tic = toc;
    if (!MosquittoClient.connected()) {
      Serial.println("Made no MQTT connection.");
      reconnect();
    } else {
      digitalWrite(onboard_led, HIGH);
      publish_message(); // Publisher action
    }
  }
  MosquittoClient.loop(); // Subscriber action
}

