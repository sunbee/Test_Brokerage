#include <Arduino.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

void mosquittoDo(char* topic, byte* payload, unsigned int length) {
  Serial.print("Got a message in topic ");
  Serial.println(topic);
  Serial.print("Received data: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print(char(payload[i]));
  }
  Serial.println();
}

void setup() {
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

