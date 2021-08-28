#ifndef mqtt_handler
    #define mqtt_handler
    #include <Arduino.h>
    #include <AsyncMqttClient.h>              //Async MQTT Library
    #include <Ticker.h>                       //Ticker for running multithread
    #include <ArduinoJson.h>                  //Encoading and Decoding JSON
    #include "global_var_one.h"
    #include "global_var_two.h"
    #include "device_handler.h"
    #include "mqtt_handler.h"
    #include "common_meathods.h"
    #include "web_sockets_handler.h"


    void sendWebSocketStatus();
    void onMqttConnect(bool sessionPresent);
    void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void MqttBlank(AsyncMqttClientDisconnectReason reason);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);
    void send_device_template();
    void sendToMQTT(String topic, String msg);
    void send_status(String relay, bool value);
    void send_status();
    void connectToMqtt();
    void subscribe_mqtt_input();
    void connect_to_mqtt();
    void setup_mqtt();
#endif