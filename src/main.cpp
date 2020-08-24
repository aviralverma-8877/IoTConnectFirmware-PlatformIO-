//Including Libraries
#include <Arduino.h>
#include <ESP8266mDNS.h>        // Include the mDNS library
#include "global_var_two.h"
#include "global_var_one.h"
#include "structures.h"
#include "common_meathods.h"
#include "web_handler.h"
#include "mqtt_handler.h"
#include "device_handler.h"
#include "web_sockets_handler.h"

void setup() 
{
  Serial.begin(115200);
  SPIFFS.begin();
  webSocket.begin();
  if (SPIFFS.exists("/config.json")) {
    read_config();
    delayMS = conf.pingTime;
  }
  else{
    configuration newConfig;
    write_config(newConfig);
  }

/*--------Setting up the GPIOs-------------------------------*/  
  callback = &blank;
/*--------Setting up the GPIOs-------------------------------*/
  if(SPIFFS.exists("/device_config.json"))
  {
    String device_config = read_device_config();
    StaticJsonDocument<1000> doc;
    DeserializationError error = deserializeJson(doc, device_config);
    if(error)
    {}
    bool wifi_setup_done = doc["wifi_setup_done"];
    if(wifi_setup_done)
    {
      WiFi.mode(WIFI_STA);
      configure_gpio();
      TickerForTimeOut.once_ms(10,[](){
        perform_action();
      });
      TickerForFeedbackLED.attach(0.6, feedbackLED);
      TickerForcheckReset.attach_ms(10, checkReset);
      int retry = 0;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if(retry > 10)
          enable_ap();
        if(debugging)
          Serial.print(".");
        checkReset();
        retry++;
      }
      disable_ap();
      WiFiStatus = true;
      fetchIP();  
      mqtt.onConnect(onMqttConnect);
      mqtt.onDisconnect(onMqttDisconnect);
      mqtt.onSubscribe(onMqttSubscribe);
      mqtt.onUnsubscribe(onMqttUnsubscribe);
      mqtt.onMessage(onMqttMessage);
    //  mqtt.onPublish(onMqttPublish);
      connect_to_mqtt();
/*-------Setting up the trikers-----------------------------*/
      TickerForconnectToMqtt.attach_ms(10000, connectToMqtt);
      TickerForfetchIP.attach(30, fetchIP);
      // if(!debugging)
      // {
      //   TickerForSerialListner.attach_ms(10, SerialListner);
      //   send_status_uart();
      // }
/*-------Setting up the trikers-----------------------------*/    
    }
    else
    {
      enable_ap();
    }
  }
  else
  {
    enable_ap();
  }
/*-------HOST Name Setup------------------------------------*/
  WiFi.hostname("iot-connect-"+chipid);
  MDNS.begin("iot-connect-"+chipid);
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
  inSetup = false;
}



void loop() 
{
  MDNS.update();
  webSocket.loop();
  callback();
  if(hasSensor)
  {
    if(millis()%2000 == 0)
    {
      if(hasDHTSensor)
      {
        dht.temperature().getEvent(&event);
        if (!isnan(event.temperature))
          temp = event.temperature-8;
        dht.humidity().getEvent(&event);
        if (!isnan(event.relative_humidity))
          humid = event.relative_humidity;
      }
      if(hasLightSensor)
        light = map(analogRead(LDR_PIN), 0, 255, 0, 100);
    }
  }
}