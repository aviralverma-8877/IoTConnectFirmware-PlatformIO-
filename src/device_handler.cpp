#include <Arduino.h>
#include <ESP8266mDNS.h> 
#include <DHT.h>
#include "mqtt_handler.h"
#include "global_var_one.h"
#include "global_var_two.h"
#include "common_meathods.h"
#include "web_sockets_handler.h"
#include "mqtt_handler.h"

String read_device_config();
String read_mqtt_config();
void perform_action();
void perform_action(String relay, bool value);
bool comp(const char *val1,const char *val2);
void fetchIP();
void feedbackLED();
void checkReset();
void read_config();
void pinging();
void sendSensorData();
void toggle_relay(String relay);

void setup_tickers()
{
  TickerForconnectToMqtt.attach(1, connectToMqtt);
  TickerForfetchIP.attach(10, fetchIP);
  TickerForFeedbackLED.attach(0.6, feedbackLED);
  TickerForcheckReset.attach_ms(10, checkReset);
  TickerForWebSocketStatus.attach(1,sendWebSocketStatus);
  TickerForPinging.attach(10, pinging);
  if(hasSensor)
    TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
}

String device_status()
{
  String return_msg = "";
  DynamicJsonDocument return_doc(700);
  return_doc["action"] = "device_status";
  return_doc["uname"] = conf.http_username;
  return_doc["wifi_ssid"] = Wifi_ssid;
  return_doc["wifi_rssi"] = WiFi.RSSI();
  return_doc["onb_led"] = conf.led_enabled;
  return_doc["btn_relay_act"] = conf.btn_relay_act;
  return_doc["save_eeprom"] = conf.save_eeprom;
  return_doc["firmware_version"] = FIRMWARE_V;
  return_doc["mqtt_status"] = MQTTStatus;
  bool WiFi_status = (WiFi.status() == WL_CONNECTED);
  return_doc["wifi_status"] = WiFi_status;
  return_doc["chip_id"] = chipid;
  String device_config = read_device_config();
  StaticJsonDocument<100> filter;
  filter["init_setup_done"] = true;
  DynamicJsonDocument doc(100);
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
  {}
  else
  {
    bool init_setup = doc["init_setup_done"];
    return_doc["init_setup"] = init_setup;
  }
  doc.clear();
  serializeJson(return_doc, return_msg);
  return_doc.clear();
  return return_msg;
}
void relay_action(String relay, bool value, String by)
{
  DynamicJsonDocument doc(1500);
  read_config();
  bool save_eeprom = conf.save_eeprom;
  serialDisplay("SAVE EEPROM",String(save_eeprom));
  if(save_eeprom)
  {
    String mqtt_data = read_mqtt_config();
    DeserializationError error = deserializeJson(doc, mqtt_data);
    if(error)
    {
      return;
    }
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
    perform_action();
  }
  else
  {
    TickerForTimeOut.once_ms(50,[relay, value](){
      perform_action(relay, value);
    });
  }
  
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
  TickerForTimeOutTwo.once_ms(100,[r](){
    sendToMQTT(norttopic, r);
  });
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
void manage_dns_request()
{
  dnsServer.processNextRequest();
}


/*----Meathod for reconfiguring WiFi settings---------------*/
void reset()
{
  TickerForconnectToMqtt.detach();
  if (WiFi.status() == WL_CONNECTED) {
    mqtt.onDisconnect(MqttBlank);
    mqtt.disconnect();
    WiFi.disconnect();
  }
  serialDisplay("Format","Formatting SPIFFS");
  SPIFFS.format();
  serialDisplay("Format","Formatting Completed");
  configuration newConf = {false,false,true,false,2000,"N/A","admin","admin","","",false};
  newConf.setupFlag = true;
  serialDisplay("Writing","Writing Config");
  write_config(newConf);
  serialDisplay("Writing","Writing Completes");
  ESP.reset();

}
/*----Meathod for reconfiguring WiFi settings---------------*/
/*----Meathod for sending relay status on UART--------------*/
void send_status_uart()
{
  StaticJsonDocument<200> doc;
  doc["a"] = "RS";
  JsonArray data = doc.createNestedArray("n");
  for(int t=0; t<8; t++)
  {
    data.add(sr.get(t));
  }
  serializeJson(doc, Serial);
  Serial.println();
}
/*----Meathod for sending relay status on UART--------------*/
/*---Meathod for pinging MQTT Server for active connection--*/
void pinging()
{
  serialDisplay("Ping","Pinged now");
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
/*---Meathod for pinging MQTT Server for active connection--*/
/*-----Meathod for sending sensor data----------------------*/

void ICACHE_RAM_ATTR handleData(float h, float t) {
  DynamicJsonDocument doc(500);
  volatile float humidity = h;
  volatile float temperature = t;
  doc["d"] = chipid;
  doc["t"] = temperature;
  doc["h"] = humidity;
  String s;
  serializeJson(doc, s);
  sendToMQTT(espsensor, s);
}

void ICACHE_RAM_ATTR handleError(uint8_t e) {
}

void setup_sensor()
{
  DynamicJsonDocument doc(500);
  StaticJsonDocument<200> filter;
  filter["device_config"]["dht"]["INSTALLED"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
    return;
  bool has_dht = doc["device_config"]["dht"]["INSTALLED"];
  if(has_dht)
  {
    if(comp(DHTType.c_str(), "dht11"))
    {
      sensor_dht11.setup(dht_pin);
      sensor_dht11.onData(handleData);
      sensor_dht11.onError(handleError);
    }
    else if(comp(DHTType.c_str(), "dht22"))
    {
      sensor_dht22.setup(dht_pin);
      sensor_dht22.onData(handleData);
      sensor_dht22.onError(handleError);
    }  
  }
}

void sendSensorData()
{
  DynamicJsonDocument doc(500);
  StaticJsonDocument<200> filter;
  filter["device_config"]["dht"]["INSTALLED"] = true;
  filter["device_config"]["light"]["INSTALLED"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
    return;
  bool has_dht = doc["device_config"]["dht"]["INSTALLED"];
  bool has_light = doc["device_config"]["light"]["INSTALLED"];
  doc.clear();
  doc["d"] = chipid;  
  if(has_dht)
  {
    if(comp(DHTType.c_str(), "dht11"))
    {
      sensor_dht11.read();
    }
    else if(comp(DHTType.c_str(), "dht22"))
    {
      sensor_dht22.read();
    }  
  }
  int light;
  if(has_light)
  {
    light = map(analogRead(LDR_PIN), 0, 255, 0, 100);
    doc["l"] = light;
  }
  String s;
  serializeJson(doc, s);
  sendToMQTT(espsensor, s);
}
/*-----Meathod for sending sensor data----------------------*/
/*-----Meathod for checking reset button--------------------*/
void checkReset()
{
  if(digitalRead(reset_btn) == def_btn_value)
  {
    if(!reset_btn_status)
    {
      reset_btn_status = true;
      reset_btn_press_time = millis();
    }
    else
    {
      if(reset_btn_press_time != 0)
      {
        if((millis()-reset_btn_press_time)>(10*1000))
        {
          reset();
        }
      }
    }
  }
  if(digitalRead(reset_btn) == !def_btn_value)
  {
    if(reset_btn_status)
    {
      reset_btn_status = false;
      reset_btn_press_count++;
      StaticJsonDocument<200> doc;
      doc["action"] = "reset_btn";
      doc["espid"] = chipid;
      doc["count"] = reset_btn_press_count;
      String r;
      serializeJson(doc,r);
      sendToMQTT(espaction,r);
      read_config();
      String relay = conf.btn_relay_act;
      if(!comp(relay.c_str(), "N/A"))
      {
        toggle_relay(relay);
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
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient httpAPI;
    httpAPI.begin("http://api.ipify.org/?format=json");
    int HttpCode = httpAPI.GET();
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
      serialDisplay("IP Address",IpAddress);
      serialDisplay("LocalIP",LocalIP);
      serialDisplay("Gateway",WiFi_gateway);
    }
  }
  Wifi_ssid = WiFi.SSID();
  serialDisplay("SSID",Wifi_ssid);
  callback = &blank;
}
/*-----Meathod for fetching IP Address----------------------*/

void read_config()
{
  if (SPIFFS.exists("/config.json")) {
    File configFile = SPIFFS.open("/config.json", "r");
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
    }
  }
}

String read_mqtt_config()
{
  if (SPIFFS.exists("/mqtt_topics.json")) {
    File configFile = SPIFFS.open("/mqtt_topics.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      configFile.close();
      return buf.get();
    }
  }
}

String read_device_config()
{
  if (SPIFFS.exists("/device_config.json")) {
    File configFile = SPIFFS.open("/device_config.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      configFile.close();
      return buf.get();
    }
  }
}

void generate_mqtt_topics()
{
  DynamicJsonDocument doc(1000);
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config);
  if(error)
    return;

  int relay_count = 1;
  bool has_shift_reg = doc["device_config"]["shift_out_reg"]["avail"];

  DynamicJsonDocument topic_doc(1500);
  JsonArray relay = topic_doc.createNestedArray("relay");
  
  if(has_shift_reg)
  {
    for(int i=0; i<8; i++)
    {
      DynamicJsonDocument relay_object(200);
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
      DynamicJsonDocument relay_object(200);
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
  File topicFile = SPIFFS.open("/mqtt_topics.json", "w");
  String r;
  serializeJsonPretty(topic_doc, r);
  topicFile.print(r);
  topicFile.close();
  ESP.reset();
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
  send_status();
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
  JsonArray array = doc["relay"];
  for( int t=0; t< array.size(); t++) 
  {
    DynamicJsonDocument ele = array[t];
    String com = ele["comp"];
    String name = ele["name"];
    byte pin = ele["pin"];
    if(comp(com.c_str(), "shift_reg"))
    {
      if(comp(name.c_str(), relay.c_str()))
      {
        bool status = sr.get(pin);
        relay_action(relay,!status,"");
      }          
    }
    if(comp(com.c_str(), "gpio"))
    {
      if(comp(name.c_str(), relay.c_str()))
      {
        bool status = digitalRead(pin);
        relay_action(relay,!status,"");
      }
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
  send_status();
}

void enable_ap()
{
  const byte DNS_PORT = 53;
  IPAddress apIP(192, 168, 4, 1);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("IoT Connect");
  dnsServer.start(DNS_PORT, "*", apIP);
}

void disable_ap()
{
  WiFi.mode(WIFI_STA);
}