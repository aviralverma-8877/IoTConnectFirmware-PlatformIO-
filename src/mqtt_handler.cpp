#include "mqtt_handler.h"

/*-------Meathod called when connected to MQTT--------------*/
void sendWebSocketStatus()
{
  send_data_to_webSocket(device_status());
}
void onMqttConnect(bool sessionPresent) {
  serialDisplay("MQTT","MQTT is Connected");
  TickerForFeedbackLED.detach();
  read_config();
  if(conf.led_enabled)
  {
    digitalWrite(indicator_led, def_led_value);
  }
  else
  {
    digitalWrite(indicator_led, !def_led_value);
  }
  MQTTStatus = true;
  reconnect_mqtt = false;
  TickerForconnectToMqtt.detach();
  if (SPIFFS.exists("/mqtt_topics.json")) 
  {
    StaticJsonDocument<1000> doc;
    StaticJsonDocument<100> filter;
    filter["mqtt"]["service"] = true;
    String device_config = read_device_config();
    DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
    if(error)
      return;
    String service = doc["mqtt"]["service"];
    if(!comp(service.c_str(), "N/A"))
      subscribe_mqtt_input();
  }
}
/*-------Meathod called when connected to MQTT--------------*/
/*-------Meathod called when subscribed to MQTT Topic-------*/
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  serialDisplay("MQTT","Subscribed to MQTT");
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
  if(WiFi.status() == WL_CONNECTED)
  {
    reconnect_mqtt = true;
    mqtt.disconnect();
    mqtt.setCleanSession(true);
    TickerForconnectToMqtt.detach();
    TickerForconnectToMqtt.attach(5, setup_mqtt);
  }
  serialDisplay("MQTT","MQTT is disconnected.");
}
/*-------Meathod called when disconnected from MQTT Topic---*/

void MqttBlank(AsyncMqttClientDisconnectReason reason) {
}


/*-------Meathod called on reciving message from MQTT-------*/
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
  MQTTStatus = true;
  String p = "";
  for(int i=index; (unsigned)i<len;i++)
  {
    p += payload[i];
  }
  serialDisplay("MQTT Topic",topic);
  serialDisplay("MQTT Message",p);
  DynamicJsonDocument device_doc(1000);
  device_doc["action"] = "mqtt_in";
  device_doc["topic"] = topic;
  device_doc["payload"] = p;
  String pl;
  serializeJson(device_doc, pl);
  serialDisplay("Sending Websocket","MQTT Input");
  send_data_to_webSocket(pl);
  device_doc.clear();
  String device_config = read_device_config();
  DynamicJsonDocument device_filter(100);
  device_filter["mqtt"]["prefix"] = true;
  device_filter["mqtt"]["suffix"] = true;
  DeserializationError error_1 = deserializeJson(device_doc, device_config, DeserializationOption::Filter(device_filter));
  if(error_1)
    return;
  String prefix = device_doc["mqtt"]["prefix"];
  String suffix = device_doc["mqtt"]["suffix"];
  if(strcmp(topic, (prefix+intopic+suffix).c_str())==0)
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
        if(hasSensor)
        {
          TickerForsendSensorData.detach();
          TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
        }
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
        TickerForTimeOut.once(2,[](){
          ESP.reset();
        });
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
  /*-------Get MQTT Topic list--------------------------------*/
      else if(comp(action,"GET_MQTT_TOPICS"))
      {
        String mqtt_topics = read_mqtt_config();
        StaticJsonDocument<100> filter;
        filter["relay"][0]["name"] = true;
        filter["relay"][0]["topic"] = true;
        DynamicJsonDocument doc(1000);
        DeserializationError error = deserializeJson(doc, mqtt_topics, DeserializationOption::Filter(filter));
        if(error)
        {
          return;
        }
        mqtt_topics = "";
        serializeJson(doc,mqtt_topics);
        sendToMQTT(outtopic, mqtt_topics);
      }
  /*-------Get MQTT Topic list--------------------------------*/
    }
  }
  else
  {
    const char* relay = "";
    bool action = false;
    String mqtt_data = read_mqtt_config();
    DynamicJsonDocument doc(1500);
    DeserializationError error_1 = deserializeJson(doc, mqtt_data);
    if(error_1)
      return;
    StaticJsonDocument<200> msg;
    DeserializationError error_2 = deserializeJson(msg, p);
    if(error_2)
      return;
    if(!msg.containsKey("action"))
      return;
    for( JsonObject kv : doc["relay"].as<JsonArray>() ) 
    {
      String config_topic = kv["topic"];
      serialDisplay("MQTT Topics",(prefix+config_topic+suffix));
      if(comp(topic, (prefix+config_topic+suffix).c_str()))
      {
        relay = (const char*)kv["name"];
        action = msg["action"];
        TickerForTimeOut.once_ms(10,[relay, action](){
          perform_action(relay, action);
        });
        kv["status"] = action;
      }
    }
    mqtt_data = "";
    serializeJsonPretty(doc, mqtt_data);
    read_config();
    if(conf.save_eeprom)
    {
      write_mqtt_topics(mqtt_data);
    }
  }
}
/*-------Meathod called on reciving message from MQTT-------*/
/*----Meathod for sending MQTT Data-------------------------*/
void sendToMQTT(String topic, String msg)
{
  DynamicJsonDocument doc(500);
  StaticJsonDocument<200> filter;
  filter["mqtt"]["prefix"] = true;
  filter["mqtt"]["suffix"] = true;
  filter["mqtt"]["service"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
  {
    return;
  }
  String service = doc["mqtt"]["service"];
  if(!comp(service.c_str(),"N/A"))
  {
    String prefix = doc["mqtt"]["prefix"];
    String suffix = doc["mqtt"]["suffix"];
    serialDisplay("MQTT","Published to "+prefix+topic+suffix);
    serialDisplay("MQTT Data",msg.c_str());
    mqtt.publish((prefix+topic+suffix).c_str(), 0, false, msg.c_str(), msg.length());
    DynamicJsonDocument web_payload(msg.length()+200);
    web_payload["action"] = "mqtt_out";
    web_payload["topic"] = prefix+topic+suffix;
    web_payload["payload"] = msg;
    String r;
    serializeJson(web_payload, r);
    send_data_to_webSocket(r);
    web_payload.clear();
  }
}
/*----Meathod for sending MQTT Data-------------------------*/
/*----Meathod for sending relay status----------------------*/
void send_status()
{
  serialDisplay("Sending Status","START");
  read_config();
  if(SPIFFS.exists("/mqtt_topics.json"))
  {
    DynamicJsonDocument doc(1500);
    StaticJsonDocument<200> filter;
    filter["mqtt"]["prefix"] = true;
    filter["mqtt"]["suffix"] = true;
    String device_config = read_device_config();
    DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
    if(error)
    {
      return;
    }
    String prefix = doc["mqtt"]["prefix"];
    String suffix = doc["mqtt"]["suffix"];
    serialDisplay("MQTT Prefix",prefix);
    serialDisplay("MQTT Suffix",suffix);
    doc.clear();
    filter.clear();
    String mqtt_data = read_mqtt_config();
    filter["relay"][0]["name"] = true;
    filter["relay"][0]["pin"] = true;
    filter["relay"][0]["comp"] = true;
    filter["relay"][0]["topic"] = true;
    error = deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));
    if(error)
      return;
    for( JsonObject kv : doc["relay"].as<JsonArray>() ) 
    {
      String com = kv["comp"];
      if(comp(com.c_str(), "shift_reg"))
      {
        int pin = kv["pin"];
        bool status = bool(sr.get(pin));
        kv["status"] = status;
        String topic = kv["topic"];
        String full_topic = prefix+topic+suffix;
        kv["full_topic"] = full_topic;
        serialDisplay("full_topic",full_topic);
        serialDisplay("Pin", String(pin));
        serialDisplay("Status", String(status));
      }
      else if(comp(com.c_str(), "gpio"))
      {
        int pin = kv["pin"];
        bool status = bool(digitalRead(pin));
        kv["status"] = status;
        String topic = kv["topic"];
        String full_topic = prefix+topic+suffix;
        kv["full_topic"] = full_topic;
        serialDisplay("full_topic",full_topic);
        serialDisplay("Pin", String(pin));
        serialDisplay("Status", String(status));
      }
    }
    doc["esp_clip_id"] = chipid;
    doc["action"] = "status";
    doc["hasSensor"] = hasSensor;
    String r;
    serializeJson(doc, r);
    serialDisplay("Sending Status","WebSocket");
    send_data_to_webSocket(r);
    if(MQTTStatus)
    {
      TickerForTimeOut.once_ms(100,[r](){
        serialDisplay("Sending Status","MQTT");
        sendToMQTT(outtopic, r);
        serialDisplay("Sending Status","MQTT Sent");
      });
    }
  }  
}
/*----Meathod for sending relay status----------------------*/
/*----Meathod called on sending/publishing message on MQTT--*/
void onMqttPublish(uint16_t packetId) {
  serialDisplay("MQTT","Message sent on "+packetId);
}
/*----Meathod called on sending/publishing message on MQTT--*/


void connectToMqtt()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if(!MQTTStatus)
    {
      serialDisplay("MQTT","Trying MQTT Connect");
      mqtt.connect();
    }
  }
}

void subscribe_mqtt_input()
{
  serialDisplay("MQTT","Subscribing to topics");
  DynamicJsonDocument doc(1500);
  StaticJsonDocument<200> filter;
  filter["mqtt"]["prefix"] = true;
  filter["mqtt"]["suffix"] = true;
  filter["mqtt"]["qos"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
  {
    return;
  }
  MQTT_QoS = doc["mqtt"]["qos"];
  String prefix = doc["mqtt"]["prefix"];
  String suffix = doc["mqtt"]["suffix"];
  mqtt.subscribe((prefix+intopic+suffix).c_str(), MQTT_QoS);
  doc.clear();
  filter.clear();
  filter["relay"][0]["topic"] = true;
  String mqtt_data = read_mqtt_config();
  error = deserializeJson(doc, mqtt_data, DeserializationOption::Filter(filter));
  if(error)
  {
    return;
  }
  for( JsonObject kv : doc["relay"].as<JsonArray>() ) 
  {
    String topic = kv["topic"];
    serialDisplay("MQTT Topic", prefix+topic+suffix);
    mqtt.subscribe((prefix+topic+suffix).c_str(), MQTT_QoS);
  }
}

void connect_to_mqtt()
{
  serialDisplay("MQTT Setup","Setting up MQTT Properties");
  StaticJsonDocument<500> doc;
  StaticJsonDocument<100> filter;
  filter["mqtt"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config,DeserializationOption::Filter(filter));
  if(error)
  {
    return;
  }
  const char* host = doc["mqtt"]["host"];
  uint16_t port = doc["mqtt"]["port"];
  serialDisplay("MQTT Host",String(host));
  serialDisplay("MQTT Port",String(port));
  bool auth = doc["mqtt"]["auth"];
  if(auth)
  {
    const char* uname = doc["mqtt"]["uname"];
    const char* pass = doc["mqtt"]["pass"];
    serialDisplay("MQTT Username",String(uname));
    serialDisplay("MQTT Password",String(pass));
    mqtt.setCredentials(uname, pass);
  }
  mqtt.setServer(host, port);
  connectToMqtt();
}

void setup_mqtt()
{
  String device_config = read_device_config();
  StaticJsonDocument<100> filter;
  filter["mqtt"]["service"] = true;
  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
    return;
  String service = doc["mqtt"]["service"];
  if(!comp(service.c_str(),"N/A"))
  {
    serialDisplay("MQTT Setup","Setting up MQTT Actions");
    mqtt.onConnect(onMqttConnect);
    mqtt.onDisconnect(onMqttDisconnect);
    mqtt.onSubscribe(onMqttSubscribe);
    mqtt.onUnsubscribe(onMqttUnsubscribe);
    mqtt.onMessage(onMqttMessage);
    mqtt.onPublish(onMqttPublish);
    connect_to_mqtt();
  }
}
