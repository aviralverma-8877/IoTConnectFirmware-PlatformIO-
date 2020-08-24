#include <Arduino.h>
#include <AsyncMqttClient.h>              //Async MQTT Library
#include <Ticker.h>                       //Ticker for running multithread
#include <ArduinoJson.h>                  //Encoading and Decoding JSON
#include <ESP8266httpUpdate.h>            //ESP Update Library.
#include "global_var_one.h"
#include "global_var_two.h"
#include "device_handler.h"
#include "mqtt_handler.h"
#include "common_meathods.h"
#include "web_sockets_handler.h"

/*-------Meathod called when connected to MQTT--------------*/
void onMqttConnect(bool sessionPresent) {
  serialDisplay("MQTT","MQTT is Connected");
  MQTTStatus = true;
  send_status();
  first_connect = true;
  if (SPIFFS.exists("/mqtt_topics.json")) {
    StaticJsonDocument<1000> doc;
    String device_config = read_device_config();
    DeserializationError error = deserializeJson(doc, device_config);
    if(error)
      return;
    String service = doc["mqtt"]["service"];
    if(service != "N/A")
      subscribe_mqtt_input();
  }
  TickerForPinging.attach_ms(10000, pinging);
  if(hasSensor)
    TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
}
/*-------Meathod called when connected to MQTT--------------*/
/*-------Meathod called when subscribed to MQTT Topic-------*/
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  serialDisplay("MQTT","Subscribed to MQTT");
  TickerForFeedbackLED.detach();
  digitalWrite(indicator_led, LOW);     // set pin to the opposite state
}
/*-------Meathod called when subscribed to MQTT Topic-------*/
/*-------Meathod called when unsubscribed from MQTT Topic---*/
void onMqttUnsubscribe(uint16_t packetId) {
  serialDisplay("MQTT","Unsubscribed from "+packetId);
}
/*-------Meathod called when unsubscribed from MQTT Topic---*/

/*-------Meathod called when disconnected from MQTT Topic---*/
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  MQTTStatus = false;
  send_status();
  serialDisplay("MQTT","MQTT is disconnected.");
  TickerForFeedbackLED.attach(0.6, feedbackLED);
  if (WiFi.status() != WL_CONNECTED) {
    enable_ap();
    if(debugging)
      Serial.print(".");
  }
  else
    disable_ap();
    connectToMqtt();
}
/*-------Meathod called when disconnected from MQTT Topic---*/

/*-------Meathod called on reciving message from MQTT-------*/
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
  String p = "";
  for(int i=index; (unsigned)i<len;i++)
  {
    p += payload[i];
  }
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(2000);
  DeserializationError error_1 = deserializeJson(doc, mqtt_data);
  if(error_1)
    return;
  if(strcmp(topic, intopic.c_str())==0)
  {
    StaticJsonDocument<200> root;
    serialDisplay("MQTT",p);
    DeserializationError error = deserializeJson(root, p);
    if (!error) 
    {
      const char *action;
      action = root["action"];
  /*-------Action command for reconfigring WiFi---------------*/
      if(comp(action,"RESET_DEVICE"))
      {
        StaticJsonDocument<200> doc;
        doc["action"] = "ResetDevice";
        doc["stat"] = "Reset Success.";
        String r;
        serializeJson(doc, r);
        sendToMQTT(outtopic, r);
        callback = &reset;
      }
  /*-------Action command for reconfigring WiFi---------------*/
  /*-------Action command for fetching WiFi SSID--------------*/
      if(comp(action,"SSID"))
      {
        StaticJsonDocument<200> doc;
        doc["SSID"] = Wifi_ssid;
        doc["RSSI"] = String(WiFi.RSSI());
        String r;
        serializeJson(doc, r);
        sendToMQTT(outtopic, r);
      }
  /*-------Action command for fetching WiFi SSID--------------*/
  /*-------Action command for setting Sensor Frequency--------------*/
      if(comp(action,"FREQ"))
      {
        delayMS = root["f"];
        conf.pingTime = delayMS;      //Saving the delay time in config
        write_config(conf);
        TickerForsendSensorData.detach();
        TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
      }
  /*-------Action command for setting Sensor Frequency--------------*/
  /*-------Action command for getting Sensor Frequency--------------*/
      if(comp(action,"GETFREQ"))
      {
        StaticJsonDocument<200> doc;
        doc["FREQ"] = delayMS;
        String r;
        serializeJson(doc, r);
        sendToMQTT(outtopic, r);
      }
  /*-------Action command for getting Sensor Frequency--------*/
  /*-------Action command for fetching ESP Chip ID------------*/
      if(comp(action,"ESPID"))
      {
        StaticJsonDocument<200> doc;
        doc["CHIPID"] = chipid;
        String r;
        serializeJson(doc, r);
        sendToMQTT(outtopic, r);
      }
  /*-------Action command for fetching ESP Chip ID------------*/

  /*-------Action command for resetting ESP-------------------*/
      if(comp(action,"RESET"))
      {
        StaticJsonDocument<200> doc;
        doc["action"] = "ResetStatus";
        doc["stat"] = "Resetting Device...";
        String r;
        serializeJson(doc, r);
        sendToMQTT(outtopic, r);
        ESP.reset();
      }
  /*-------Action command for resetting ESP-------------------*/

  /*-------Action command for controlling Relays--------------*/
      else if(comp(action,"RELAY"))
      {
        String no = root["no"];
        bool value = root["value"];
        String by = root["by"];
        relay_action(no, value, by);
      }
  /*-------Action command for controlling Relays--------------*/
  /*-------Action command for getting Relays status-----------*/
      else if(comp(action,"STATUS"))
      {
        send_status();
      }
  /*-------Action command for getting Relays status-----------*/
  /*-----Action command for getting getting firmware version--*/
      else if(comp(action,"GET_VERSION"))
      {
        StaticJsonDocument<200> doc;
        doc["action"] = "Firmware Version";
        doc["value"] = FIRMWARE_V;
        String r;
        serializeJson(doc, r);
        sendToMQTT(outtopic, r);
      }
  /*-----Action command for getting getting firmware version--*/
  /*-------Action command for updating firmware---------------*/
      else if(comp(action,"UPDATE"))
      {
        const char * c = root["url"];
        updateAddress = c;
        callback = &updateESP;
      }
  /*-------Action command for updating firmware---------------*/
    }
  }
  else
  {
    StaticJsonDocument<200> msg;
    DeserializationError error_2 = deserializeJson(msg, p);
    if(error_2)
      return;
    if(!msg.containsKey("action"))
      return;
    for( const auto& kv : doc["relay"].as<JsonObject>() ) 
    {

      String config_topic = kv.value()["topic"];      
      if(comp(topic, config_topic.c_str()))
      {
        bool action = msg["action"];
        const char *key = kv.key().c_str();
        doc["relay"][key]["status"] = action;
      }
    }
    mqtt_data = "";
    serializeJsonPretty(doc, mqtt_data);
    write_mqtt_topics(mqtt_data);
    TickerForTimeOut.once_ms(10,[](){
      perform_action();
    });
  }
}
/*-------Meathod called on reciving message from MQTT-------*/
/*----Meathod for sending MQTT Data-------------------------*/
void sendToMQTT(String topic, String msg)
{
  mqtt.publish(topic.c_str(), 2, false, msg.c_str(), msg.length());
}
/*----Meathod for sending MQTT Data-------------------------*/
/*----Meathod for sending relay status----------------------*/
void send_status()
{
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(1000);
  StaticJsonDocument<200> filter;
  filter["relay"][0]["name"] = true;
  filter["relay"][0]["status"] = true;
  filter["relay"][0]["pin"] = true;
  filter["relay"][0]["comp"] = true;
  DeserializationError error = deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));
  if(error)
    return;
  doc["esp_clip_id"] = chipid;
  doc["action"] = "status";
  String r;
  serializeJson(doc, r);
  send_data_to_webSocket(r);
  sendToMQTT(espstatus, r);
//  send_status_uart();
}
/*----Meathod for sending relay status----------------------*/
/*----Meathod called on sending/publishing message on MQTT--*/
void onMqttPublish(uint16_t packetId) {
  serialDisplay("MQTT","Message sent on "+packetId);
}
/*----Meathod called on sending/publishing message on MQTT--*/
void connectToMqtt() 
{
  if(WiFi.status() != WL_CONNECTED) {
    if(debugging)
      Serial.print(".");
  }
  else
  {
    mqtt.connect();
  }
}

void subscribe_mqtt_input()
{
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(2000);
  DeserializationError error = deserializeJson(doc, mqtt_data);
  if(error)
    return;
  String command = doc["COMMAND"];
  mqtt.subscribe(command.c_str(), 2);
  for( const auto& kv : doc["relay"].as<JsonObject>() ) 
  {
    String topic = kv.value()["topic"];
    if(debugging)
      Serial.println(topic);
    mqtt.subscribe(topic.c_str(), 2);
  }
}

void connect_to_mqtt()
{
  StaticJsonDocument<1000> doc;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config);
  if(error)
    return;
  const char* host = doc["mqtt"]["host"];
  uint16_t port = doc["mqtt"]["port"];
  bool auth = doc["mqtt"]["auth"];
  if(auth)
  {
    const char* uname = doc["mqtt"]["uname"];
    const char* pass = doc["mqtt"]["pass"];
    mqtt.setCredentials(uname, pass);
  }
  mqtt.setServer(host, port);
  connectToMqtt();
}