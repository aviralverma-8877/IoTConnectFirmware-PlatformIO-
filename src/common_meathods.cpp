#include "common_meathods.h"

void write_config(configuration config)
{
  File configFile = SPIFFS.open("/config.json", FILE_WRITE);
  String r = config_to_json(config);
  Serial.println(r);
  configFile.print(r); 
  configFile.close();
}
String config_to_json(configuration config)
{
  StaticJsonDocument<500> jsonBuffer;
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
  jsonBuffer["fauxmo_relay_1"] = config.fauxmo_relay_1;
  jsonBuffer["fauxmo_relay_2"] = config.fauxmo_relay_2;
  jsonBuffer["fauxmo_relay_3"] = config.fauxmo_relay_3;
  String r;
  serializeJsonPretty(jsonBuffer, r);
  return r;
}
void write_device_config(DynamicJsonDocument jsonBuffer)
{

  File configFile = SPIFFS.open("/device_config.json", FILE_WRITE);
  String r;
  serializeJsonPretty(jsonBuffer, r);
  configFile.print(r); 
  configFile.close();
  TickerForTimeOut.once(1,[](){
    ESP.restart();
  });
}

void write_mqtt_topics(String r)
{
  File configFile = SPIFFS.open("/mqtt_topics.json", FILE_WRITE);
  configFile.print(r); 
  configFile.close();
}


/*-------Meathod for displaying serial data in JSON---------*/
debug_msg dbg_msg;
bool dbg_free = true;
void serialDisplay(String head,String body)
{
  if(debugging)
  {
    while(!dbg_free)
    {
      delay(10);
    }
    dbg_msg.head = head;
    dbg_msg.body = body;
    dbg_free = false;
    xTaskCreatePinnedToCore([](void *p){
      debug_msg dbg_msg = *(debug_msg*)p;
      String head = dbg_msg.head;
      String body = dbg_msg.body;
      StaticJsonDocument<200> doc;
      doc["action"] = "display";
      doc["head"] = head;
      doc["body"] = body;
      String c;
      serializeJson(doc, c);
      Serial.println(c);
      dbg_free = true;
      vTaskDelete(NULL);
    },"Debugger",10000,&dbg_msg,0,&dbg_task,1);
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