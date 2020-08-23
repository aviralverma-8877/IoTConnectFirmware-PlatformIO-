#include <Arduino.h>
#include <ESP8266mDNS.h> 
#include "mqtt_handler.h"
#include "global_var_one.h"
#include "global_var_two.h"
#include "common_meathods.h"
#include "web_sockets_handler.h"

String read_device_config();
String read_mqtt_config();
void perform_action();

void relay_action(String relay, bool value, String by)
{
  String mqtt_data = read_mqtt_config();
  DynamicJsonDocument doc(2000);
  DeserializationError error = deserializeJson(doc, mqtt_data);
  if(error)
    return;
  doc["relay"][relay]["status"] = value;
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
  sendToMQTT(norttopic, r);
  TickerForTimeOut.once_ms(10,[]{
    perform_action();
    TickerForTimeOut.once_ms(10,[]{
     send_status();
    });
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
void switch_wifi()
{
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid,wifi_pass);
  callback = &blank;
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
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(1000);
  }
  SPIFFS.format();
  configuration newConf = {false,false,true,2000,"admin","admin"};
  newConf.setupFlag = true;
  write_config(newConf);
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
  StaticJsonDocument<200> doc;
  doc["d"] = chipid;
  if(strcmp(DEVICE_V, "v2") == 0)
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
  DynamicJsonDocument doc(2000);
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config);
  if(error)
    return;
  
  bool has_dht = doc["dht"]["INSTALLED"];
  bool has_light = doc["light"]["INSTALLED"];
  doc.clear();
  String mqtt_topics = read_mqtt_config();
  error = deserializeJson(doc, mqtt_topics);
  if(error)
  {
    return;
  }
  if(has_dht)
  {
    int t = temp;
    int h = humid;
  }
  if(has_light)
  {
    int l = light;
  }
  doc.clear();
  doc["d"] = chipid;
  doc["t"] = temp;
  doc["h"] = humid;
  doc["l"] = light;
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
    if(inSetup)
      reset();
    else
      callback = &reset;
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
      Wifi_ssid = WiFi.SSID();
      LocalIP = IpAddress2String(WiFi.localIP());
      serialDisplay("SSID",Wifi_ssid);
      serialDisplay("IP Address",IpAddress);
      send_status();
    }
  }
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
      String http_username = jsonBuffer["http_username"];
      String http_password = jsonBuffer["http_password"];
      conf.http_username = http_username;
      conf.http_password = http_password;
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
  DynamicJsonDocument topic_doc(1500);
  int relay_count = 1;
  bool has_shift_reg = doc["device_config"]["shift_out_reg"]["avail"];
  topic_doc["COMMAND"] = chipid+"/COMMAND";
  if(has_shift_reg)
  {
    for(int i=0; i<8; i++)
    {
      String key = "Relay "+String(relay_count);
      String value = chipid+"/shift_out_reg/pin_"+i;
      topic_doc["relay"][key]["comp"] = "shift_reg";
      topic_doc["relay"][key]["topic"] = value;
      topic_doc["relay"][key]["status"] = false;
      topic_doc["relay"][key]["pin"] = i;
      relay_count++;
    }
  }
  int relay_gpio_count = doc["device_config"]["relay"]["count"];
  if(relay_gpio_count > 0)
  {
    for(int i=0; i<relay_gpio_count; i++)
    {
      String key = "Relay-"+String(relay_count);
      int pin  = doc["device_config"]["relay"]["GPIO"][i];
      String value = chipid+"/gpio_relay/pin_"+pin;
      topic_doc["relay"][key]["comp"] = "gpio";
      topic_doc["relay"][key]["topic"] = value;
      topic_doc["relay"][key]["status"] = false;
      topic_doc["relay"][key]["pin"] = pin;
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
  String r = read_mqtt_config();
  DynamicJsonDocument doc(2000);
  DeserializationError error = deserializeJson(doc, r);
  if(error)
    return;
  for( const auto& kv : doc["relay"].as<JsonObject>() ) 
  {
    String com = kv.value()["comp"];
    if(comp(com.c_str(), "shift_reg"))
    {
      int pin = kv.value()["pin"];
      bool value = kv.value()["status"];
      sr.set(pin, value);
    }
    if(comp(com.c_str(), "gpio"))
    {
      int pin = kv.value()["pin"];
      bool value = kv.value()["status"];
      digitalWrite(pin, value);
    }
  }
  TickerForTimeOut.once_ms(10,[](){
    send_status();
  });
}

void enable_ap()
{
  const byte DNS_PORT = 53;
  IPAddress apIP(192, 168, 4, 1);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("IoT Connect");
  dnsServer.start(DNS_PORT, "*", apIP);
}

void disable_ap()
{
  WiFi.mode(WIFI_STA);
}