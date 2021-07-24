#include <Arduino.h>

String nodeName = "BHRIGU"; // 6 char EXACTLY


/*
Obtain brightness readings (lux) using TSL2591 sensor
and publish to MQTT broker.
*/
#include "SensorArray.h"

SensorArray _sensorArray;

#define PIN_WATER_LEVEL 0 // Channel on MCP3008 for water level

uint16_t brightness = 0;  // Value read from lux sensor
int waterLevel = 0;       // Value read from water-level sensor

/*
Obtain sensor readings to publish to MQTT broker.
This section has the instructions to use the the MCP3008 
10-bit analog-to-digital converter for adding upto 
8 analog sensors to a nodemcu, circumventng the limitation 
of a single analog pin. We have retained the potentiometer 
wired to A0 for test comparison.
*/
#define PIN_POT A0        // Input pin for the potentiometer
int potValue;         // Value read from the pot


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
#define TIMER_INTERVAL 6000
#define onboard_led 16

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
    if (MosquittoClient.connect("nodemcu1", "fire_up_your_neurons", NULL)) {
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
  Read the pot on A0, 6000
  then normalize the value
  and prepare for publishing
  as serialized JSON. 
  */
  potValue = analogRead(PIN_POT);
  Serial.print("Reading: " + potValue);
  float potReading = potValue * 100.0 / 1023.0;
  char potDisplay[7];
  dtostrf(potReading, 6, 2, potDisplay);
  /*
  Read the water level on analog channel no. 0
  of the MCP3008 10-bit ADC using the readADC()
  method of the _mcp object and passing the 
  channel no. (ref. MCP).
  */
  waterLevel = _sensorArray.get_mcp_waterLevel(PIN_WATER_LEVEL, true);
  float waterLevelReading = waterLevel * 100.0 / 1023.0;
  char waterLevelDisplay[7];
  dtostrf(waterLevelReading, 6, 2, waterLevelDisplay);
  /*
  Read the brightness level reported by TSL2591X on I2C bus.
  */
  brightness = _sensorArray.get_tsl_visibleLight();
  Serial.println(brightness);
  float brightnessReading = brightness * 100.0 / 65535.0;
  char brightnessDisplay[7];
  dtostrf(brightnessReading, 6, 2, brightnessDisplay);
  /*
  Make the message to publish to the MQTT broker as
  serialized JSON. 
  */
  char readOut[60];
  snprintf(readOut, 60, "{\"Name\":\"%6s\",\"Pot\":%6s,\"Level\":%6s,\"Lux\":%6s}", nodeName.c_str(), potDisplay, waterLevelDisplay, brightnessDisplay);
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
  if (toc - tic > TIMER_INTERVAL) {
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

