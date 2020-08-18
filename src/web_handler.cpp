#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>          //Async WiFi Manager
#include <Ticker.h>                       //Ticker for running multithread
#include <ArduinoJson.h>                  //Encoading and Decoding JSON
#include <ESP8266HTTPClient.h>            //HTTP Client library.
#include "global_var_one.h"
#include "global_var_two.h"
#include "device_handler.h"
#include "common_meathods.h"
#include "mqtt_handler.h"

extern "C" uint32_t _FS_start;
extern "C" uint32_t _FS_end;


/*-------Web Server Controller------------------------------*/
void handleWebControl(AsyncWebServerRequest *request)
{
  String message;
  StaticJsonDocument<200> doc;
  int params = request->params();
  if(request->hasParam("command"))
  {
    String command = request->arg("command");
    DeserializationError error = deserializeJson(doc, command);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = 0;
      return_doc["error"] = "error in parsing";
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg);
      return;
    }
    int relay = doc["relay"];
    bool action = doc["action"];
    relay_action(relay, action, "");
    doc["done"] = 1;
    serializeJson(doc, message);
  }
  if(request->hasParam("device"))
  {
    String command = request->arg("device");
    DeserializationError error = deserializeJson(doc, command);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = 0;
      return_doc["error"] = "Error in parsing";
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg); 
      return;
    }
    String action = doc["action"];
    doc["done"] = 1;
    serializeJson(doc, message);
    if(comp(action.c_str(),"reset"))
      callback = &reset;
    if(comp(action.c_str(),"reboot"))
      ESP.reset();
    if(comp(action.c_str(),"toggle_onb"))
    {
      bool status = doc["status"];
      conf.led_enabled = status;
      write_config(conf);
      send_status();
    }
  }
  request->send(200, "application/json", message);
}
void handleWebStatus(AsyncWebServerRequest *request)
{
  String return_msg = "";
  StaticJsonDocument<500> return_doc;
  JsonArray relay_status = return_doc.createNestedArray("v");
  for(int t=0; t<8; t++)
  {
    relay_status.add(sr.get(t));
  }
  return_doc["uname"] = conf.http_username;
  return_doc["type"] = DEVICE_V;
  return_doc["wifi_ssid"] = Wifi_ssid;
  return_doc["wifi_rssi"] = WiFi.RSSI();
  return_doc["onb_led"] = conf.led_enabled;
  return_doc["firmware_version"] = FIRMWARE_V;
  String device_config = read_device_config();
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, device_config);
  bool init_setup = doc["init_setup_done"];
  return_doc["init_setup"] = init_setup;
  if(strcmp(DEVICE_V, "v2") == 0)
  {
    return_doc["t"] = temp;
    return_doc["h"] = humid;
    return_doc["l"] = light;
  }
  serializeJson(return_doc, return_msg);
  request->send(200, "application/json", return_msg); 
}
void handleDeviceConfig(AsyncWebServerRequest *request)
{
  String return_msg;
  int params = request->params();
  if(request->hasParam("options"))
  {
    String device_config = request->arg("options");
    StaticJsonDocument<600> doc;
    DeserializationError error = deserializeJson(doc, device_config);
    if (error) 
    {
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = false;
      return_doc["error"] = "Failed to parse config";
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg);
      return;
    }
    StaticJsonDocument<200> return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);
    TickerForTimeOut.once(1,[doc]{
      write_device_config(doc);
      generate_mqtt_topics();
    });
  }
  else
  {
    StaticJsonDocument<200> return_doc;
    return_doc["done"] = false;
    return_doc["error"] = "No config available";
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);
  }
  

}
void web_scan_wifi(AsyncWebServerRequest *request)
{
  WiFi.scanNetworksAsync([request](int networksFound){
    StaticJsonDocument<500> wifi_ssid;
    JsonArray ssid = wifi_ssid.createNestedArray("ssid");
    for(int i=0; i<networksFound; i++)
    {
      ssid.add(String(WiFi.SSID(i)));
    }
    String return_msg;
    serializeJson(wifi_ssid, return_msg);
    request->send(200, "application/json", return_msg);
  });
}
void web_update_login(AsyncWebServerRequest *request)
{
  if(request->hasParam("options"))
  {
    StaticJsonDocument<200> login_option;
    String option = request->arg("options");
    DeserializationError error = deserializeJson(login_option, option);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = false;
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg); 
      return;
    }
    String http_username = login_option["uname"];
    String http_password = login_option["password"];
    conf.http_username = http_username;
    conf.http_password = http_password;
    write_config(conf);

    String return_msg = "";
    StaticJsonDocument<200> return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg); 
  }
}
void web_set_wifi(AsyncWebServerRequest *request)
{
  if(request->hasParam("options"))
  {
    StaticJsonDocument<200> wifi_option;
    String option = request->arg("options");
    DeserializationError error = deserializeJson(wifi_option, option);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = false;
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg); 
      return;
    }
    String ssid = wifi_option["ssid"];
    String pass = wifi_option["pass"];
    wifi_ssid = ssid;
    wifi_pass = pass;
    callback = &switch_wifi;
    String return_msg = "";
    StaticJsonDocument<200> return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);
    TickerForTimeOut.once(15,[](){
      if(WiFi.status() != WL_CONNECTED)
      {
        callback = &reset;
      }
      else
      {
        callback = &fetchIP; 
      }
    });
  }
  else
  {
    request->send(404);
  }
  
}
/*-------Web Server Controller------------------------------*/
/*---------Firmware Update---------------------------------*/
// Simple Firmware Update Form
void firmware_web_updater()
{
  webServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "\
    <form method='POST' action='/update_flash' enctype='multipart/form-data'>\
      <input type='file' placeholder='firmware.bin' name='update'><input type='submit' value='Update'> firmware.bin\
    </form></ br>\
    <form method='POST' action='/update_spiffs' enctype='multipart/form-data'>\
      <input type='file' placeholder='spiffs.bin' name='update'><input type='submit' value='Update'> spiffs.bin\
    </form>");
  });

  webServer.on("/update_flash", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    if(shouldReboot)
    {
      request->send_P(200, "text/html", "Upload successfull, Rebooting....<br /><a href='/'>Home Page</a>\
      <script>\
        setTimeout(\
          function()\
            {\
              window.location.href = \"/\"\
            },10000);\
      </script>");
    }
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      if(debugging)
        Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000, U_FLASH)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
      if(debugging)
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        if(debugging)
          Serial.printf("Update Success: %uB\n", index+len);
        TickerForTimeOut.once(1,[](){
          ESP.reset();
        });
      } else {
        if(debugging)
          Update.printError(Serial);
      }
    }
  });

  webServer.on("/update_spiffs", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    if(shouldReboot)
    {
      request->send_P(200, "text/html", "Upload successfull, Rebooting....<br /><a href='/'>Home Page</a>\
      <script>\
        setTimeout(\
          function()\
            {\
              window.location.href = \"/\"\
            },10000);\
      </script>");
      read_config();
      /*--------Reading data from SPIFFS for last relay status ----*/
      for(int t=0; t<8; t++)
      {
        sr.set(t,conf.pinValues[t]);           //Overwriting saved values on default values.
        delay(100);
      }
      /*--------Reading data from SPIFFS for last relay status ----*/
    }
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      if(debugging)
        Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
      close_all_fs();
      if (!Update.begin(fsSize, U_FS)){//start with max available size
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
      if(debugging)
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        if(debugging)
          Serial.printf("Update Success: %uB\n", index+len);
        TickerForTimeOut.once(1,[](){
          ESP.reset();
        });
      } else {
        if(debugging)
          Update.printError(Serial);
      }
    }
  });
}
/*---------Firmware Update---------------------------------*/
