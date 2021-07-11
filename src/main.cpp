#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <SECRETS.h>
#define onboard_led 16

const char* SSID = SECRET_SSID;
const char* PASS = SECRET_PASS;
char broker_ip[] = SECRET_BROKER_IP;

WiFiClient WiFiClient;
PubSubClient Broker(WiFiClient);

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

  Broker.setServer(broker_ip, 1883);
}

void reconnect() {
  while (!Broker.connected()) {
    if (Broker.connect("NodeMCU", "fire_up_your_neurons", NULL)) {
      Serial.println("Uh-Kay!");
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
  Broker.publish("Test", char_buffer);
  delay(1200);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(onboard_led, LOW);
  if (!Broker.connected()) {
    Serial.println("Made no MQTT connection.");
    reconnect();
  } else {
    digitalWrite(onboard_led, HIGH);
    publish_message();
  }
}