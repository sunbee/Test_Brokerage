#include <Arduino.h>

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

/*
Obtain a sensor reading to publish to MQTT broker.
This section has the instructions to use the sole analog pin
on the nodemuc. 
How will the nodemcu recieve inputs from sensors for 
temperature, moisture, brightness, etc.? We will need a
multiplexing solution e.g. MCP3008 so multiple sensors can
be configured to use a single analog pin.
*/
int sensorPin = A0;   // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

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
#define TIMER_INTERVAL 60000
#define onboard_led 16

const char* SSID = SECRET_SSID;
const char* PASS = SECRET_PASS;
char broker_ip[] = SECRET_BROKER_IP;
unsigned long tic = millis();

WiFiClient WiFiClient;
PubSubClient MosquittoClient(WiFiClient);

void displayMessage(String mess) {
  /*
  Use in the callback for MQTT subscriber.
  The callback is 'mosquittoDo()'.
  */
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,15);
  display.println(mess);
  display.display();
  // delay(2001);
  // display.clearDisplay();
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
  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
  pinMode(onboard_led, OUTPUT);

  // Connect to WiFi:
  Serial.begin(9600);
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
  The 'connect()' methods wants client credentials. When the
  MQTT broker is not set up for authentication, we have successfully
  connected to the MQTT broker using dummy data, passing string literals 
  for args 'id' and 'user' and NULL for 'pass'.
  Having connected successully, proceed to publish or listen.
  */
  while (!MosquittoClient.connected()) {
    if (MosquittoClient.connect("NodeMCU", "fire_up_your_neurons", NULL)) {
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
  then display to OLED
  using the helper function. 
  */
 sensorValue = analogRead(sensorPin);
 Serial.print("Reading: " + sensorValue);
 float sensorReading = sensorValue * 100.0 / 1023.0;
 char sensorDisplay[7];
 dtostrf(sensorReading, 6, 2, sensorDisplay);
 char readOut[15];
 snprintf(readOut, 15, "{\"Pot\":%6s}", sensorDisplay);
 displayMessage(readOut);
 return readOut;
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
    sensor readings to the MQTT broker in a non-blocking way
    for the listener. The use of 'delay()' would block the 
    listener, causing events to be missed.  
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

