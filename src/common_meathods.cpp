#include "common_meathods.h"

void write_config(configuration config)
{
  String r = config_to_json(config);
  config_queue = r;
  xTaskCreate([](void *p){
    String r = config_queue;
    File configFile = SPIFFS.open("/config.json", FILE_WRITE);
    configFile.print(r); 
    configFile.close();
    vTaskDelete(NULL);
  },"Config write",10000,NULL,10,NULL);
}
String config_to_json(configuration config)
{
  JsonDocument jsonBuffer;
  jsonBuffer["setupFlag"] = config.setupFlag;
  jsonBuffer["updateFlag"] = config.updateFlag;
  jsonBuffer["led_enabled"] = config.led_enabled;
  jsonBuffer["save_eeprom"] = config.save_eeprom;
  jsonBuffer["pingTime"] = config.pingTime;
  jsonBuffer["btn_relay_act"] = config.btn_relay_act;
  jsonBuffer["http_username"] = config.http_username;
  jsonBuffer["http_password"] = config.http_password;
  jsonBuffer["WiFi_SSID"] = config.WiFi_SSID;
  jsonBuffer["WiFi_PASS"] = config.WiFi_PASS;
  jsonBuffer["wifi_setup_done"] = config.wifi_setup_done;
  jsonBuffer["fauxmo_relay"] = config.fauxmo_relay;
  jsonBuffer.shrinkToFit();
  String r;
  serializeJsonPretty(jsonBuffer, r);
  return r;
}
void write_device_config(JsonDocument jsonBuffer)
{
  String r;
  serializeJsonPretty(jsonBuffer, r);
  device_config_queue = r;
  xTaskCreate([](void *p){
    String r = read_device_config();
    File configFile = SPIFFS.open("/device_config.json", FILE_WRITE);
    configFile.print(r); 
    configFile.close();
    vTaskDelete(NULL);
  },"Device Config Write",20000,NULL,11,NULL);
}

void write_mqtt_topics(String r)
{
  mqtt_topic_queue = r;
  xTaskCreate([](void *p){
    String r = mqtt_topic_queue;
    File configFile = SPIFFS.open("/mqtt_topics.json", FILE_WRITE);
    configFile.print(r); 
    configFile.close();
    vTaskDelete(NULL);
  },"MQTT Topic write",10000,NULL,12,NULL);
}


/*-------Meathod for displaying serial data in JSON---------*/
void serialDisplay(String head,String body)
{
  if(debugging)
  {
    JsonDocument doc;
    if(comp(debug_meathod.c_str(),""))
    {
      doc["action"] = "display";
      doc["head"] = head;
      doc["body"] = body;
      String c;
      serializeJson(doc, c);
      Serial.println(c);
    }
    else
    {
      if(comp(head.c_str(),debug_meathod.c_str()))
      {
        doc["action"] = "display";
        doc["head"] = head;
        doc["body"] = body;
        String c;
        serializeJson(doc, c);
        Serial.println(c);
      }
    }
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