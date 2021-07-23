#include <Arduino.h>

String nodeName = "BHRIGU"; // 6 char EXACTLY

/*
Obtain brightness readings (lux) using TSL2591 sensor
and publish to MQTT broker.
*/
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

Adafruit_TSL2591 _tsl(2591);
/*
Display a message received by subscriber to OLED.
This section has the Adafruit libraries and setup instructions
in order to use the OLED display.
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
uint16_t brightness = 0;

/*
Obtain sensor readings to publish to MQTT broker.
This section has the instructions to use the the MCP3008 
10-bit analog-to-digital converter for adding upto 
8 analog sensors to a nodemcu, circumventng the limitation 
of a single analog pin. We have retained the potentiometer 
wired to A0 for test comparison.
*/
#define PIN_POT A0        // Input pin for the potentiometer
#define PIN_WATER_LEVEL 0 // Channel on MCP3008 for water level
int potValue = 0;         // Value read from the pot
int waterLevel = 0;       // Value read from water-level sensor

/*
Connect to MQTT broker over WiFi (Home Internet).
This section has the libraries to connect to WiFi and the broker.
Note that we will use a single nodemcu as publisher and subscriber,
with a single instance of the Mosquitto client object.
Adjust TIMER_INTERVAL (in milliseconds) for publishing frequency.
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/*
Obtain analog values from sensors via MCP3008 10-bit analog-to-digital
convertor (ADC). The connections must be as follows:
MCP <> ESP8266
15 <> 3.3V
14 <> 3.3V
13 <> GND
12 <> D5 (CLK)
11 <> D6 (MISO)
10 <> D7 (MOSI)
9  <> D8 (CS)
8  <> GND

Pin numbering on the MCP3008 is 0-15 with 2 rows of 8 pins on either side 
is as follows: 
a.) Pins numbered 0-7 in one row are analog input channels.
b.) Pins numbered 8-15 in the row across are power and communications bus.
c.) Pins 0 and 15 are at the notch on either side. 
*/
#include <Adafruit_MCP3008.h>
Adafruit_MCP3008 _mcp; // Constructor

#include <SECRETS.h>
#define TIMER_INTERVAL 120000
#define onboard_led 16

const char* SSID = SECRET_SSID;
const char* PASS = SECRET_PASS;
char broker_ip[] = SECRET_BROKER_IP;
unsigned long tic = millis();

WiFiClient WiFiClient;
PubSubClient MosquittoClient(WiFiClient);

void configureLuxMeter() {
  /*
  COnfigure the TSL2591 lux sensor for use.
  */
  _tsl.setGain(TSL2591_GAIN_MED);
  _tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
}

void displayMessage(String mess) {
  /*
  Use in the callback for MQTT subscriber.
  The callback is 'mosquittoDo()'.
  */
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(mess);
  display.display();
}

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
  displayMessage(message4OLED);
}

void setup() {
  // Start the serial bus
  Serial.begin(9600);

  // Start the lux sensor
  if (_tsl.begin()) {
    Serial.println("Sensor ready!");
  } else {
    Serial.println("Found no sensor.");
  }
  configureLuxMeter();

  // Start the ADC
  _mcp.begin();

  // Start the display at I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
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
  and handle the message with the callback in event-driven 
  pattern. 
  */
  MosquittoClient.setServer(broker_ip, 1883);
  MosquittoClient.setCallback(mosquittoDo);
}

void reconnect() {
  /*
  Connect to the MQTT broker in order to publish a message
  or listen in on a topic of interest. 
  The 'connect()' methods wants client credentials. 
  When the MQTT broker is not setup for authentication, 
  we have successfully connected to the MQTT broker 
  passing string literals for args 'id' and 'user' and NULL for 'pass'.
  Having connected successully, proceed to publish or listen.
  Use connection string as follows:
  1. "BHRIGU", "fire_up_your_neurons", NULL
  2. "VASISH", "whatsup_nerds", NULL
  */
  while (!MosquittoClient.connected()) {
    if (MosquittoClient.connect("BHRIGU", "whatsup_nerds", NULL)) {
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
  Read the pot on A0, 
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
  waterLevel = _mcp.readADC(PIN_WATER_LEVEL);
  float waterLevelReading = waterLevel * 100.0 / 1023.0;
  char waterLevelDisplay[7];
  dtostrf(waterLevelReading, 6, 2, waterLevelDisplay);
  /*
  Read the brightness level reported by TSL2591X on I2C bus.
  */
  brightness = _tsl.getLuminosity(TSL2591_VISIBLE);
  float brightnessReading = brightness * 100.0 / 65535.0;
  char brightnessDisplay[7];
  dtostrf(brightnessReading, 6, 2, brightnessDisplay);
  /*
  Make the message to publish to the MQTT broker as
  serialized JSON. 
  */
  char readOut[60];
  snprintf(readOut, 60, "{\"Name\":\"%6s\",\"Pot\":%6s,\"Level\":%6s,\"Lux\":%6s}", nodeName.c_str(), potDisplay, waterLevelDisplay, brightnessDisplay);
  displayMessage(readOut);
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

