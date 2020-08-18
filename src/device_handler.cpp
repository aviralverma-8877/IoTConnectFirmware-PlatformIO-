#include <Arduino.h>
#include "mqtt_handler.h"
#include "global_var_one.h"
#include "global_var_two.h"
#include "common_meathods.h"

void relay_action(int no, bool value, String by)
{
  sr.set(no, value);
//sending nortification
  StaticJsonDocument<200> doc;
  if(by != "")
  {
    doc["by"] = by;
    doc["no"] = no;
    doc["CHIPID"] = chipid;
    doc["input"] = value;
  }
  String r = "";
  serializeJson(doc, r);
  sendToMQTT(norttopic, r);
//saving to config
  TickerForTimeOut.once_ms(10,[](){  
    for(int t=0; t<8; t++)
    {
      conf.pinValues[t] = sr.get(t);
    }
    write_config(conf);
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
/*-------Serial Listener Setup-----------------------------------*/
void SerialListner()
{
  String message = "";
  StaticJsonDocument<200> doc;
  if(Serial.available())
  {
    char n;
    while(Serial.available())
    {
      n = Serial.read();
      message += n;
    }
    message.trim();
    DeserializationError error = deserializeJson(doc, message);
    if(error)
    {
      return;
    }
    String action = doc["a"];
    int relay = doc["n"];
    bool val = doc["v"];
    if(action == "R")
    {
      relay_action(relay, val, "");
    }
  }

}
/*-------Serial Listener Setup-----------------------------------*/

void switch_wifi()
{
  WiFi.disconnect();
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




/*----Meathod for reconfiguring WiFi settings---------------*/
void reset()
{
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    delay(1000);
  }
  SPIFFS.format();
  configuration newConf = {{ 0,0,0,0,0,0,0,0 },false,false,true,2000,"admin","admin"};
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
  StaticJsonDocument<200> doc;
  doc["d"] = chipid;
  doc["t"] = temp;
  doc["h"] = humid;
  doc["l"] = light;
  String s;
  serializeJson(doc, s);
  sendToMQTT(sensortopic, s);
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
      for(int t=0; t<8; t++)
      {
        bool pinVal = jsonBuffer["relay_status"][t]; 
        conf.pinValues[t] = pinVal;
      }
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
      return buf.get();
    }
  }
}

void generate_mqtt_topics()
{
  StaticJsonDocument<500> doc;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config);
  if(error)
    return;
  StaticJsonDocument<1000> topic_doc;
  int relay_count = 0;
  bool has_shift_reg = doc["device_config"]["shift_out_reg"]["avail"];
  topic_doc["input"]["COMMAND"] = chipid+"/COMMAND";
  if(has_shift_reg)
  {
    for(int i=0; i<8; i++)
    {
      String key = "relay_"+String(relay_count);
      relay_count++;
      String value = chipid+"/shift_out_reg/pin_"+i;
      topic_doc["input"][key]["topic"] = value;
      topic_doc["input"][key]["status"] = false;
    }
  }
  int relay_gpio_count = doc["device_config"]["relay"]["count"];
  if(relay_gpio_count > 0)
  {
    for(int i=0; i<relay_gpio_count; i++)
    {
      String key = "relay_"+String(relay_count);
      relay_count++;
      int pin  = doc["device_config"]["relay"]["GPIO"][i];
      String value = chipid+"/gpio_relay/pin_"+pin;
      topic_doc["input"][key]["topic"] = value;
      topic_doc["input"][key]["status"] = false;
    }
  }
  bool has_dht_sensor = doc["device_config"]["dht"]["INSTALLED"];
  if(has_dht_sensor)
  {
    String key = "dht";
    int pin  = doc["device_config"]["dht"]["GPIO"];
    String value = chipid+"/dht/pin_"+pin;
    topic_doc["output"][key] = value;
  }

  topic_doc["output"]["ESPSTATUS"] = chipid+"/ESPSTATUS";
  topic_doc["output"]["NORT"] = chipid+"/NORT";
  topic_doc["output"]["ESPRAW"] = chipid+"/ESPRAW";
  
  bool has_light_sensor = doc["device_config"]["light"]["INSTALLED"];
  if(has_dht_sensor)
  {
    String key = "light";
    String pin  = doc["device_config"]["light"]["GPIO"];
    String value = chipid+"/light/pin_"+pin;
    topic_doc["output"][key] = value;
  }
  File topicFile = SPIFFS.open("/mqtt_topics.json", "w");
  String r;
  serializeJsonPretty(topic_doc, r);
  topicFile.print(r);
}