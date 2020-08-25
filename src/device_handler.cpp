#include <Arduino.h>
#include <ESP8266mDNS.h> 
#include "DHTesp.h"
#include "mqtt_handler.h"
#include "global_var_one.h"
#include "global_var_two.h"
#include "common_meathods.h"
#include "web_sockets_handler.h"
#include "mqtt_handler.h"

String read_device_config();
String read_mqtt_config();
void perform_action();
bool comp(const char *val1,const char *val2);

void relay_action(String relay, bool value, String by)
{
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(1500);
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

//sending nortification
  if(by != "")
  {
    doc["by"] = by;
  }

  doc["relay"] = relay;
  doc["esp_chip_id"] = chipid;
  doc["value"] = value;

  String r = "";
  serializeJson(doc, r);
  TickerForTimeOut.once_ms(100,[r](){
    sendToMQTT(norttopic, r);
    perform_action();
  });
}

/*-------Meathod to update ESP------------------------------*/
void updateESP()
{
  StaticJsonDocument<200> doc;
  doc["action"] = "UpdateStatus";
  doc["stat"] = "Updating Device...";
  String r;
  serializeJson(doc, r);
  sendToMQTT(outtopic, r);
  delay(100);
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateAddress.c_str());
  serialDisplay("Update Address",updateAddress);
  String stat = "";
  switch(ret) 
  {
    case HTTP_UPDATE_FAILED:
        stat = "Failed to update.";
        break;
    case HTTP_UPDATE_NO_UPDATES:
        stat = "No Update Available...";
        break;
    case HTTP_UPDATE_OK:
        stat = "Update Successfull...";
        break;
  }
  StaticJsonDocument<200> doc_1;
  doc_1["action"] = "UpdateStatus";
  doc_1["stat"] = stat;
  r = "";
  serializeJson(doc_1, r);
  sendToMQTT(outtopic, r);
  delay(100);
  serialDisplay("Update",stat);
  callback = &blank;
}
/*-------Meathod to update ESP------------------------------*/
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
    digitalWrite(indicator_led, LOW);
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
  configuration newConf = {false,false,true,2000,"admin","admin"};
  newConf.setupFlag = true;
  serialDisplay("Writing","Writing Config");
  write_config(newConf);
  serialDisplay("Writing","Writing Completes");
  ESP.restart();

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
  int temp, humid, light;
  if(has_dht)
  {
    DHTesp dht;
    if(comp(DHTType.c_str(), "DHT11"))
      dht.setup(dht_pin, DHTesp::DHT11);
    else if(comp(DHTType.c_str(), "DHT22"))
      dht.setup(dht_pin, DHTesp::DHT22);
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    doc["t"] = temperature;
    doc["h"] = humidity;
  }
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
  if(digitalRead(reset_btn) == HIGH)
  {
      reset();
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
      conf.pingTime = jsonBuffer["pingTime"];
      conf.setupFlag = jsonBuffer["setupFlag"];
      conf.updateFlag = jsonBuffer["updateFlag"];
      conf.wifi_setup_done = jsonBuffer["wifi_setup_done"];
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
  ESP.restart();
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