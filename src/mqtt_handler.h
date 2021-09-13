#ifndef mqtt_handler
    #define mqtt_handler
    #include <Arduino.h>
    #include <AsyncMqttClient.h>              //Async MQTT Library
    #include <Ticker.h>                       //Ticker for running multithread
    #include <ArduinoJson.h>                  //Encoading and Decoding JSON
    #include "FS.h"
    #include "SPIFFS.h"
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
    String send_device_template(bool send_on_mqtt);
    void sendToMQTT(String topic, String msg);
    void sendToMQTT(void *parameter);
    void send_status(String relay, bool value);
    void connectToMqtt();
    void send_to_web_mqtt(String msg);
    void subscribe_mqtt_input();
    void connect_to_mqtt();
    void setup_mqtt();
#endif