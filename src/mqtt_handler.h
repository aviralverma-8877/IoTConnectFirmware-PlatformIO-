#include <Arduino.h>
#include <AsyncMqttClient.h>              //Async MQTT Library
#include <Ticker.h>                       //Ticker for running multithread
#include <ArduinoJson.h>                  //Encoading and Decoding JSON
#include <ESP8266httpUpdate.h>            //ESP Update Library.

void onMqttConnect(bool sessionPresent);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);
void sendToMQTT(String topic, String msg);
void send_status();
void connectToMqtt();
void subscribe_mqtt_input();
void connect_to_mqtt();
void send_mqtt_status();