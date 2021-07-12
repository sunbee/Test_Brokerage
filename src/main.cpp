#include <Arduino.h>


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int sensorPin = A0;   // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <SECRETS.h>
#define TIMER_INTERVAL 12000
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
  display.setCursor(0,14);
  display.println(mess);
  display.display();
  delay(2001);
  display.clearDisplay();
}

void mosquittoDo(char* topic, byte* payload, unsigned int length) {
  Serial.print("Got a message in topic ");
  Serial.println(topic);
  Serial.print("Received data: ");
  char message2display[length];
  for (unsigned int i = 0; i < length; i++) {
    Serial.print(char(payload[i]));
    message2display[i] = payload[i];
  }
  char message4OLED[length+6]; // 'Got: ' and null terminator.
  snprintf(message4OLED, length+6, "Got: %s", message2display); 
  Serial.println();
  displayMessage(message4OLED);
}

void setup() {
  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.clearDisplay();
  pinMode(onboard_led, OUTPUT);

  // put your setup code here, to run once:
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

  MosquittoClient.setServer(broker_ip, 1883);
  MosquittoClient.setCallback(mosquittoDo);
}

void reconnect() {
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

void publish_message() {
  String msg_payload = "Namaste from India";
  char char_buffer[999];
  msg_payload.toCharArray(char_buffer, 999);
  MosquittoClient.publish("Test", char_buffer);
}

void loop() {
  unsigned long toc = millis();

  // put your main code here, to run repeatedly:
  digitalWrite(onboard_led, LOW);
  if (toc - tic > TIMER_INTERVAL) {
    tic = toc;
    if (!MosquittoClient.connected()) {
      Serial.println("Made no MQTT connection.");
      reconnect();
    } else {
      digitalWrite(onboard_led, HIGH);
      publish_message();
    }
  }
  MosquittoClient.loop();
}

