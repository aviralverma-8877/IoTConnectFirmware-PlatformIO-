#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <Ticker.h>                       //Ticker for running multithread
#include <ArduinoJson.h>                  //Encoading and Decoding JSON
#include <ESP8266HTTPClient.h>            //HTTP Client library.
#include "global_var_one.h"
#include "global_var_two.h"
#include "device_handler.h"
#include "common_meathods.h"
#include "mqtt_handler.h"
#include "web_sockets_handler.h"

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

    String relay = doc["relay"];
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
      reset();
    if(comp(action.c_str(),"reboot"))
      ESP.reset();
    if(comp(action.c_str(),"toggle_onb"))
    {
      bool status = doc["status"];
      conf.led_enabled = status;
      write_config(conf);
      if(conf.led_enabled)
      {
        digitalWrite(indicator_led, def_led_value);
      }
      else
      {
        digitalWrite(indicator_led, !def_led_value);
      }
    }
    if(comp(action.c_str(),"save_status"))
    {
      bool status = doc["status"];
      conf.save_eeprom = status;
      write_config(conf);
    }
    if(comp(action.c_str(),"change_relay_btn_action"))
    {
      String relay = doc["status"];
      conf.btn_relay_act = relay;
      write_config(conf);
    }
  }
  request->send(200, "application/json", message);
}
void handleWebStatus(AsyncWebServerRequest *request)
{
  String return_msg = device_status();
  request->send(200, "application/json", return_msg);
  send_status();
}
void handleDeviceConfig(AsyncWebServerRequest *request)
{
  String return_msg;
  int params = request->params();
  if(request->hasParam("options"))
  {
    String device_config = request->arg("options");
    StaticJsonDocument<1000> doc;
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
    String service = doc["mqtt"]["service"];
    if(comp(service.c_str(),"IoT Connect"))
    {
      doc["mqtt"]["host"] = MQTT_HOST;
      doc["mqtt"]["port"] = MQTT_PORT;
      doc["mqtt"]["uname"] = MQTT_UNAME;
      doc["mqtt"]["pass"] = MQTT_PASS;
      doc["mqtt"]["auth"] = true;
    }
    TickerForTimeOut.once_ms(10,[doc]{
      write_device_config(doc);
      TickerForTimeOut.once_ms(10,[doc]{
        generate_mqtt_topics();
      });
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
    conf.WiFi_SSID = ssid;
    conf.WiFi_PASS = pass;
    conf.wifi_setup_done = true;
    write_config(conf);

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(conf.WiFi_SSID,conf.WiFi_PASS);

    String return_msg = "";
    StaticJsonDocument<200> return_doc;
    return_doc["done"] = true;
    serializeJson(return_doc, return_msg);
    request->send(200, "application/json", return_msg);
    TickerForTimeOut.once(15,[](){
      if(WiFi.status() == WL_CONNECTED)
      {
        ESP.reset();
      }
      else
      {
        reset();
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
    if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
    {
      return request->requestAuthentication();
    }

    request->send(200, "text/html", "<script>\
    function httpGet(action, options = {})\
    {\
        if(action == \"device\")\
        {\
            theUrl = '/control?device='+options;\
        }\
        try\
        {\
            var xmlHttp = new XMLHttpRequest();\
            xmlHttp.open( \"GET\", theUrl, false );\
            xmlHttp.send( null );\
            return xmlHttp.responseText;\
        }\
        catch(err){\
            return JSON.stringify({\"done\":false,\"error\":err});\
        }\
    }\
    </script>\
    <form method='POST' action='/update_flash' enctype='multipart/form-data'>\
      <input type='file' placeholder='firmware.bin' name='update'><input type='submit' value='Update'> firmware.bin\
    </form></ br>\
    <form method='POST' action='/update_spiffs' enctype='multipart/form-data'>\
      <input type='file' placeholder='spiffs.bin' name='update'><input type='submit' value='Update'> spiffs.bin\
    </form><br />\
    <button onclick=\"if(confirm('Are you sure you want to reset this device?')){httpGet('device','{\\'action\\':\\'reset\\'}')}\">Reset</button>");
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
void setup_web_server()
{
  if(debugging)
  {
    if (SPIFFS.exists("/config.json")) 
    {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) 
      {
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        configFile.close();
        StaticJsonDocument<500> jsonBuffer;
        Serial.print(buf.get());
      }
    }
  }
    
  read_config();
  bool setup_flag = bool(conf.setupFlag);
  serialDisplay("Setup Flag", String(setup_flag));
  bool wifi_setup_done = bool(conf.wifi_setup_done);
  serialDisplay("WiFI setup done", String(wifi_setup_done));
  if(setup_flag)
  {
    serialDisplay("Setup","Setup Flag is true.");
    enable_ap();
  }
  else if(wifi_setup_done)
  {
    WiFi.mode(WIFI_STA);
    WiFi.begin(conf.WiFi_SSID,conf.WiFi_PASS);
    serialDisplay("Setup","Setup Flag is false.");        
  }
  else
  {
    serialDisplay("Setup","Setup Flag is true.");
    enable_ap();
  }
  
/*-------HOST Name Setup------------------------------------*/
  WiFi.hostname("iot-connect-"+chipid);
  MDNS.begin("iot-connect-"+chipid);
  webSocket.begin();
  webSocket.enableHeartbeat(15000, 3000, 2);

/*-------HOST Name Setup------------------------------------*/
/*-------Web Update Server----------------------------------*/
  firmware_web_updater();
/*-------Web Update Server----------------------------------*/
/*-------Web Server Setup-----------------------------------*/
  webServer.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    handleWebControl(request);
  });

  webServer.on("/get_status", HTTP_GET, [](AsyncWebServerRequest *request){
    handleWebStatus(request);
  });
  webServer.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    web_scan_wifi(request);
  });
  webServer.on("/set_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    web_set_wifi(request);
  });
  webServer.on("/update_login", HTTP_GET, [](AsyncWebServerRequest *request){
    web_update_login(request);
  });
  webServer.on("/update_device_config", HTTP_GET, [](AsyncWebServerRequest *request){
    handleDeviceConfig(request);
  });
  webServer.serveStatic("/device_config.json", SPIFFS, "/device_config.json");
  webServer.serveStatic("/config.json", SPIFFS, "/config.json");
  webServer.serveStatic("/mqtt_topics.json", SPIFFS, "/mqtt_topics.json");
  webServer.serveStatic("/script.js", SPIFFS, "/script.js");
  webServer.serveStatic("/style.css", SPIFFS, "/style.css");
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    File index = SPIFFS.open("/index.html", "r");
    if (index) {
      if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
      {
        return request->requestAuthentication();
      }
      request->send(SPIFFS, "/index.html", "text/html");
    }
    else{
      request->redirect("/update");
    }
    index.close();
  });
  webServer.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  webServer.onNotFound([](AsyncWebServerRequest *request){
    if(!request->authenticate(conf.http_username.c_str(), conf.http_password.c_str()))
    {
      return request->requestAuthentication();
    }
    request->redirect("/");
  });

  webServer.begin();
/*-------Web Server Setup-----------------------------------*/
/*-------HOST Name Setup------------------------------------*/
  MDNS.addService("http", "tcp", 80);
/*-------HOST Name Setup------------------------------------*/
  if(WiFi.status()!= WL_CONNECTED)
    while(WiFi.status()!= WL_CONNECTED)
    {
      MDNS.update();
      webSocket.loop();
      dnsServer.processNextRequest();
    }
  WiFi.mode(WIFI_STA);
  conf.setupFlag = false;
  write_config(conf);
}