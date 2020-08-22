#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "structures.h"
#include "mqtt_handler.h"
#include "global_var_two.h"
#include "global_var_one.h"

void (*callback)(void);
void write_config(configuration config)
{
  File configFile = SPIFFS.open("/config.json", "w");
  StaticJsonDocument<500> jsonBuffer;
  jsonBuffer["setupFlag"] = config.setupFlag;
  jsonBuffer["updateFlag"] = config.updateFlag;
  jsonBuffer["led_enabled"] = config.led_enabled;
  jsonBuffer["pingTime"] = config.pingTime;
  jsonBuffer["http_username"] = config.http_username;
  jsonBuffer["http_password"] = config.http_password;
  String r;
  serializeJsonPretty(jsonBuffer, r);
  configFile.print(r); 
  configFile.close();
}

void write_device_config(StaticJsonDocument<1000> jsonBuffer)
{

  File configFile = SPIFFS.open("/device_config.json", "w");
  String r;
  serializeJsonPretty(jsonBuffer, r);
  configFile.print(r); 
  configFile.close();
  TickerForTimeOut.once(1,[](){
    ESP.reset();
  });
}

void write_mqtt_topics(String r)
{
  File configFile = SPIFFS.open("/mqtt_topics.json", "w");
  configFile.print(r); 
  configFile.close();
}


/*-------Meathod for displaying serial data in JSON---------*/
void serialDisplay(String head,String body)
{
  if(debugging)
  {
    StaticJsonDocument<200> doc;
    doc["action"] = "display";
    doc["head"] = head;
    doc["body"] = body;
    String c;
    serializeJson(doc, c);
    Serial.println(c);
    sendToMQTT(debugtopic, c);
  }
}
/*-------Meathod for displaying serial data in JSON---------*/
/*-----Blank function-----------------------------------------*/
void blank()
{
  
}
/*-----Blank function-----------------------------------------*/
/*-----Meathod to convert IP Address to String -------------*/
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}
/*-----Meathod to convert IP Address to String -------------*/