#include "device_handler.h"
void setup_tickers()
{
  xTaskCreate([](void *p){
    for(;;)
    {
      pinging();
      delay(5000);
    }
  },"Mqtt Ping",10000,NULL,1,NULL);
  if(hasSensor)
    TickerForsendSensorData.attach(delayMS/1000, sendSensorData);
}

void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
  serialDisplay("onWifiConnect","WiFi Connected");
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
  TickerForWebSocketStatus.attach(1,sendWebSocketStatus);
  if(reconnect_mqtt)
  {
    serialDisplay("onWifiConnect","setup_mqtt");
    setup_mqtt();
  }
}

void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
  TickerForFeedbackLED.detach();
  TickerForFeedbackLED.attach(0.6, feedbackLED);
  serialDisplay("onWifiDisconnect","WiFi Disconnected");
  TickerForWebSocketStatus.detach();
  reconnect_mqtt = true;
  mqtt.disconnect();
  mqtt.setCleanSession(true);
  connectToWiFi();
}

String device_status()
{
  String return_msg = "";
  DynamicJsonDocument return_doc(700);
  read_config();
  return_doc["action"] = "device_status";
  return_doc["uname"] = conf.http_username;
  return_doc["wifi_ssid"] = Wifi_ssid;
  return_doc["wifi_rssi"] = WiFi.RSSI();
  return_doc["onb_led"] = conf.led_enabled;
  return_doc["btn_relay_act"] = conf.btn_relay_act;
  return_doc["save_eeprom"] = conf.save_eeprom;
  return_doc["firmware_version"] = FIRMWARE_V;
  return_doc["mqtt_status"] = MQTTStatus;
  return_doc["fauxmo_relay"] = conf.fauxmo_relay;
  return_doc["mqtt_status"] = MQTTStatus;
  bool WiFi_status = (WiFi.status() == WL_CONNECTED);
  return_doc["wifi_status"] = WiFi_status;
  return_doc["chip_id"] = chipid;
  return_doc["ram"] = ESP.getFreeHeap();
  String device_config = read_device_config();
  StaticJsonDocument<100> filter;
  filter["init_setup_done"] = true;
  StaticJsonDocument<100> doc;
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
  {}
  else
  {
    bool init_setup = doc["init_setup_done"];
    return_doc["init_setup"] = init_setup;
  }
  doc.clear();
  return_doc.shrinkToFit();
  serializeJson(return_doc, return_msg);
  return_doc.clear();
  return return_msg;
}
void relay_action(String relay, bool value, String by)
{
  DynamicJsonDocument doc(1500);
  read_config();
  bool save_eeprom = conf.save_eeprom;
  serialDisplay("relay_action","SAVE EEPROM "+String(save_eeprom));
  if(save_eeprom)
  {
    String mqtt_data = read_mqtt_config();
    DeserializationError error = deserializeJson(doc, mqtt_data);
    if(error)
    {
      return;
    }
    doc.shrinkToFit();
    JsonArray array = doc["relay"].as<JsonArray>();
    for (JsonObject ele : array) {
      String name = ele["name"];
      if(comp(name.c_str(), relay.c_str()))
      {
        ele["status"] = value;
        break;
      }
    }
    mqtt_data = "";
    serializeJsonPretty(doc, mqtt_data);
    doc.clear();
    write_mqtt_topics(mqtt_data);    
  }
  perform_action(relay, value);  
//sending nortification
  if(!comp(by.c_str(),""))
  {
    doc["by"] = by;
  }

  doc["relay"] = relay;
  doc["esp_chip_id"] = chipid;
  doc["value"] = value;

  String r = "";
  serializeJson(doc, r);
  sendToMQTT(norttopic, r);
}
/*-------feedbackLED----------------------------------------*/
void feedbackLED()
{
  if(conf.led_enabled)
  {
    int state = digitalRead(indicator_led);  // get the current state of GPIO1 pin
    digitalWrite(indicator_led, !state);     // set pin to the opposite state
  }
  else
  {
    digitalWrite(indicator_led, !def_led_value);
  }
  
}
/*-------feedbackLED----------------------------------------*/



/*----Meathod for reconfiguring WiFi settings---------------*/
void reset()
{
  if (WiFi.status() == WL_CONNECTED) {
    mqtt.onDisconnect(MqttBlank);
    mqtt.disconnect();
    WiFi.disconnect();
  }
  serialDisplay("reset","Formatting SPIFFS");
  bool formatted = SPIFFS.format();
  if(formatted){
    serialDisplay("reset","Success formatting");
  }else{
    serialDisplay("reset","Error formatting");
  }
  serialDisplay("reset","Formatting Completed");
  configuration newConf = {false,false,true,false,2000,"N/A","admin","admin","","",false,"{}"};
  newConf.setupFlag = true;
  serialDisplay("reset","Writing Config");
  write_config(newConf);
  serialDisplay("reset","Writing Completes");
  delay(100);
  ESP.restart();
}
/*----Meathod for reconfiguring WiFi settings---------------*/
/*---Meathod for pinging MQTT Server for active connection--*/
void pinging()
{
  if(MQTTStatus)
  {
    serialDisplay("pinging","Pinged now");
    StaticJsonDocument<200> doc;
    doc["d"] = chipid;
    if(hasSensor)
      doc["s"] = true;
    else
      doc["s"] = false;
    doc["i"] = IpAddress;
    doc["l"] = LocalIP;
    String r;
    serializeJson(doc, r);
    sendToMQTT(espraw, r);
  }
}
/*---Meathod for pinging MQTT Server for active connection--*/
/*-----Meathod for checking reset button--------------------*/
void checkReset()
{
  if(digitalRead(reset_btn) == def_btn_value)
  {
    if(!reset_btn_status)
    {
      reset_btn_status = true;
      reset_btn_press_time = millis();
      serialDisplay("checkReset","Button Pressed");
    }
    else
    {
      if(reset_btn_press_time != 0)
      {
        if((millis()-reset_btn_press_time)>(10*1000))
        {
          read_config();
          if(conf.led_enabled)
          {
            digitalWrite(indicator_led, !def_led_value);
          }
          else
          {
            digitalWrite(indicator_led, def_led_value);
          }
          reset();
        }
      }
    }
  }
  if(digitalRead(reset_btn) != def_btn_value)
  {
    if(reset_btn_status)
    {
      serialDisplay("checkReset","Button Released");
      reset_btn_status = false;
      reset_btn_press_count++;
      StaticJsonDocument<200> doc;
      doc["action"] = "reset_btn";
      doc["espid"] = chipid;
      doc["count"] = reset_btn_press_count;
      String r;
      serializeJson(doc,r);
      if(MQTTStatus)
        sendToMQTT(espaction,r);
      read_config();
      String relay = conf.btn_relay_act;
      if(!comp(relay.c_str(), "N/A"))
      {
        DynamicJsonDocument new_doc(1500);
        String mqtt_data = read_mqtt_config();
        DeserializationError error = deserializeJson(new_doc, mqtt_data);
        if(error)
        {
            return;
        }
        new_doc.shrinkToFit();
        JsonArray array = new_doc["relay"].as<JsonArray>();
        for (JsonObject ele : array) {
            String topic = ele["topic"];
            if(comp(topic.c_str(), relay.c_str()))
            {
              toggle_relay(ele["name"]);
              break;
            }
        }
      }
    }
  }
}
/*-----Meathod for checking reset button--------------------*/
/*-----Meathod for comparing string-------------------------*/
bool comp(const char *val1,const char *val2)
{
  bool res = true;
  if(strlen(val1) != strlen(val2))
    res = false;
  for(int y=0;(unsigned)y<strlen(val1);y++)
  {
    if(val1[y] != val2[y])
      res = false;
  }
  return res;
}
/*-----Meathod for comparing string-------------------------*/
/*-----Meathod for feyching IP Address----------------------*/
void fetchIP()
{
  serialDisplay("fetchIP", "Fetching IP");
  if (WiFi.status() == WL_CONNECTED)
  {
    httpAPI.begin(wifiClient, "http://api.ipify.org/?format=json");
    int HttpCode = httpAPI.GET();
    serialDisplay("fetchIP", "Get IP HTTP response code"+String(HttpCode));
    if(HttpCode > 0)
    {  
      String payload = httpAPI.getString();
      httpAPI.end();
      StaticJsonDocument<200> doc;
      deserializeJson(doc, payload);
      IpAddress = "";
      const char* s = doc["ip"];
      IpAddress = s;
      LocalIP = IpAddress2String(WiFi.localIP());
      WiFi_gateway = WiFi.gatewayIP().toString();
      serialDisplay("fetchIP","IP Address"+IpAddress);
      serialDisplay("fetchIP","LocalIP"+LocalIP);
      serialDisplay("fetchIP","Gateway"+WiFi_gateway);
    }
    else
    {
      fetchIP();
    }
  }
  Wifi_ssid = WiFi.SSID();
  serialDisplay("fetchIP","SSID"+Wifi_ssid);
}
/*-----Meathod for fetching IP Address----------------------*/
void connectToWiFi()
{
  read_config();
  bool setup_flag = bool(conf.setupFlag);
  serialDisplay("connectToWiFi","Setup Flag : "+String(setup_flag));
  bool wifi_setup_done = bool(conf.wifi_setup_done);
  serialDisplay("connectToWiFi","WiFI setup done : "+String(wifi_setup_done));
  if(setup_flag)
  {
    serialDisplay("connectToWiFi","Setup Flag is true.");
    enable_ap();
  }
  else if(wifi_setup_done)
  {
    serialDisplay("connectToWiFi","Connecting to "+conf.WiFi_SSID+".");
    enable_sta();
  }
  else
  {
    serialDisplay("connectToWiFi","Setup Flag is true.");
    enable_ap();      
  }
}
void print_config(){
  if(debugging)
  {
    if (SPIFFS.exists("/config.json")) 
    {
      File configFile = SPIFFS.open("/config.json");
      if (configFile) 
      {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        configFile.close();
        Serial.print(buf.get());
      }
    }
  }
}
void read_config()
{
  if(!comp(config_queue.c_str(),""))
  {
    StaticJsonDocument<500> jsonBuffer;
    DeserializationError error = deserializeJson(jsonBuffer, config_queue);
    if(error)
      return;
    conf.led_enabled = jsonBuffer["led_enabled"];
    conf.save_eeprom = jsonBuffer["save_eeprom"];
    conf.pingTime = jsonBuffer["pingTime"];
    conf.setupFlag = jsonBuffer["setupFlag"];
    conf.updateFlag = jsonBuffer["updateFlag"];
    conf.wifi_setup_done = jsonBuffer["wifi_setup_done"];
    String relay = jsonBuffer["btn_relay_act"];
    conf.btn_relay_act = relay;
    String http_username = jsonBuffer["http_username"];      
    conf.http_username = http_username;
    String http_password = jsonBuffer["http_password"];
    conf.http_password = http_password;
    String ssid = jsonBuffer["WiFi_SSID"];
    conf.WiFi_SSID = ssid;
    String pass = jsonBuffer["WiFi_PASS"];
    conf.WiFi_PASS = pass; 
    String fauxmo_relay = jsonBuffer["fauxmo_relay"];
    conf.fauxmo_relay = fauxmo_relay;
  }
  else if (SPIFFS.exists("/config.json")) {
    File configFile = SPIFFS.open("/config.json");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      configFile.close();
      StaticJsonDocument<500> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, buf.get());
      if(error)
        return;
      conf.led_enabled = jsonBuffer["led_enabled"];
      conf.save_eeprom = jsonBuffer["save_eeprom"];
      conf.pingTime = jsonBuffer["pingTime"];
      conf.setupFlag = jsonBuffer["setupFlag"];
      conf.updateFlag = jsonBuffer["updateFlag"];
      conf.wifi_setup_done = jsonBuffer["wifi_setup_done"];
      String relay = jsonBuffer["btn_relay_act"];
      conf.btn_relay_act = relay;
      String http_username = jsonBuffer["http_username"];      
      conf.http_username = http_username;
      String http_password = jsonBuffer["http_password"];
      conf.http_password = http_password;
      String ssid = jsonBuffer["WiFi_SSID"];
      conf.WiFi_SSID = ssid;
      String pass = jsonBuffer["WiFi_PASS"];
      conf.WiFi_PASS = pass; 
      String fauxmo_relay = jsonBuffer["fauxmo_relay"];
      conf.fauxmo_relay = fauxmo_relay;
    }
  }
}

String read_mqtt_config()
{
  if(!comp(mqtt_topic_queue.c_str(),""))
  {
    return mqtt_topic_queue;
  }
  else if (SPIFFS.exists("/mqtt_topics.json")) {
    File configFile = SPIFFS.open("/mqtt_topics.json");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      configFile.close();
      return buf.get();
    }
  }
  return "";
}

String read_device_config()
{
  if(!comp(device_config_queue.c_str(),""))
  {
    return device_config_queue;
  }
  else if (SPIFFS.exists("/device_config.json")) {
    File configFile = SPIFFS.open("/device_config.json");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      configFile.close();
      return buf.get();
    }
  }
  return "";
}

void generate_mqtt_topics(DynamicJsonDocument doc)
{
  int relay_count = 1;
  bool has_shift_reg = doc["device_config"]["shift_out_reg"]["avail"];

  DynamicJsonDocument topic_doc(1500);
  JsonArray relay = topic_doc.createNestedArray("relay");
  
  if(has_shift_reg)
  {
    for(int i=0; i<8; i++)
    {
      StaticJsonDocument<200> relay_object;
      String key = "Relay "+String(relay_count);
      String value = chipid+"/shift_out_reg/pin_"+i;
      relay_object["name"] = key;
      relay_object["comp"] = "shift_reg";
      relay_object["topic"] = value;
      relay_object["status"] = false;
      relay_object["pin"] = i;
      relay.add(relay_object);
      relay_count++;
    }
  }
  int relay_gpio_count = doc["device_config"]["relay"]["count"];
  if(relay_gpio_count > 0)
  {
    for(int i=0; i<relay_gpio_count; i++)
    {
      StaticJsonDocument<200> relay_object;
      String key = "Relay "+String(relay_count);
      int pin  = doc["device_config"]["relay"]["GPIO"][i];
      String value = chipid+"/gpio_relay/pin_"+pin;
      relay_object["name"] = key;
      relay_object["comp"] = "gpio";
      relay_object["topic"] = value;
      relay_object["status"] = false;
      relay_object["pin"] = pin;
      relay.add(relay_object);
      relay_count++;
    }
  }
  String r;
  serializeJsonPretty(topic_doc, r);
  mqtt_topic_queue = r;
  xTaskCreate([](void *p){
    String r = mqtt_topic_queue;
    File topicFile = SPIFFS.open("/mqtt_topics.json", FILE_WRITE);
    topicFile.print(r);
    topicFile.close();
    vTaskDelete(NULL);
  },"Config write",10000,NULL,13,NULL);
}

void toggle_relay(String relay)
{
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(1000);
  StaticJsonDocument<200> filter;
  filter["relay"][0]["name"] = true;
  filter["relay"][0]["pin"] = true;
  filter["relay"][0]["comp"] = true;
  DeserializationError error = deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));
  if(error)
    return;
  doc.shrinkToFit();
  JsonArray array = doc["relay"];
  for( int t=0; t< array.size(); t++) 
  {
    DynamicJsonDocument ele = array[t];
    String com = ele["comp"];
    String name = ele["name"];
    byte pin = ele["pin"];
    bool status;
    if(comp(com.c_str(), "shift_reg"))
    {
      if(comp(name.c_str(), relay.c_str()))
      {
        status = sr.get(pin);
        serialDisplay("toggle_relay","Performing action");
        relay_action(relay,!status,"");
        return;
      }          
    }
    if(comp(com.c_str(), "gpio"))
    {
      if(comp(name.c_str(), relay.c_str()))
      {
        status = digitalRead(pin);
        serialDisplay("toggle_relay","Performing action");
        relay_action(relay,!status,"");
        return;
      }
    }
  }
}

void perform_action()
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
  doc.shrinkToFit();
  JsonArray array = doc["relay"];
  for( int t=0; t< array.size(); t++) 
  {
    DynamicJsonDocument ele = array[t];
    String com = ele["comp"];
    if(comp(com.c_str(), "shift_reg"))
    {
      int pin = ele["pin"];
      bool value = ele["status"];
      sr.set(pin, value);
    }
    if(comp(com.c_str(), "gpio"))
    {
      int pin = ele["pin"];
      bool value = ele["status"];
      digitalWrite(pin, value);
    }
  }
}

void perform_action(String relay, bool value)
{
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(1000);
  StaticJsonDocument<200> filter;
  filter["relay"][0]["name"] = true;
  filter["relay"][0]["pin"] = true;
  filter["relay"][0]["comp"] = true;
  DeserializationError error = deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));
  if(error)
    return;
  doc.shrinkToFit();
  JsonArray array = doc["relay"];
  for( int t=0; t< array.size(); t++) 
  {
    DynamicJsonDocument ele = array[t];
    String com = ele["comp"];
    String name = ele["name"];
    if(comp(com.c_str(), "shift_reg"))
    {
      if(comp(name.c_str(), relay.c_str()))
      {
        int pin = ele["pin"];
        sr.set(pin, value);
      }
    }
    if(comp(com.c_str(), "gpio"))
    {
      if(comp(name.c_str(), relay.c_str()))
      {
        int pin = ele["pin"];
        digitalWrite(pin, value);
      }
    }
  }
  send_status(relay, value);
}

