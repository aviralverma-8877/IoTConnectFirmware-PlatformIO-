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
  }
  else{
    configuration newConfig;
    write_config(newConfig);
  }
/*--------Setting up the GPIOs-------------------------------*/
  pinMode(reset_btn,INPUT);
  pinMode(indicator_led,OUTPUT);
  pinMode(LDR_PIN,INPUT);
  TickerForFeedbackLED.attach(0.6, feedbackLED);
  TickerForcheckReset.attach_ms(10, checkReset);
/*--------Reading data from SPIFFS for last relay status ----*/
  for(int t=0; t<8; t++)
  {
    sr.set(t,conf.pinValues[t]);           //Overwriting saved values on default values.
    delay(100);
  }
/*--------Reading data from SPIFFS for last relay status ----*/

/*--------Setting up the GPIOs-------------------------------*/  
 AsyncWiFiManager wifiManager(&webServer,&dnsServer);
 if(debugging)
 {
  wifiManager.setDebugOutput(true); //Set WiFi manager debug output.
 }
 else
 {
   wifiManager.setDebugOutput(false); //Set WiFi manager debug output.
 }
/*-------Start WiFi Manager---------------------------------*/
  if(conf.setupFlag)
  {
    conf.setupFlag = false;
    write_config(conf);
    TickerForFeedbackLED.attach(0.2, feedbackLED);
    wifiManager.resetSettings();
    wifiManager.setBreakAfterConfig(true);
    wifiManager.startConfigPortal("IOT Connect");
  }
/*-------Start WiFi Manager---------------------------------*/
  callback = &blank;
  if(strcmp(DEVICE_V, "v2") == 0)
  {
    delayMS = conf.pingTime;
    dht.begin();          //Initalizing DHT sensor.
  }
/*-------Fetch IP Address-----------------------------------*/
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if(debugging)
      Serial.print(".");
    checkReset();
  }

  fetchIP();  
/*-------Fetch IP Address-----------------------------------*/
/*-------Setting up MQTT------------------------------------*/
  mqtt.onConnect(onMqttConnect);
  mqtt.onDisconnect(onMqttDisconnect);
  mqtt.onSubscribe(onMqttSubscribe);
  mqtt.onUnsubscribe(onMqttUnsubscribe);
  mqtt.onMessage(onMqttMessage);
//  mqtt.onPublish(onMqttPublish);
  mqtt.setCredentials(MQTT_UNAME, MQTT_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
/*-------Setting up MQTT------------------------------------*/
/*-------Setting up the trikers-----------------------------*/
  TickerForconnectToMqtt.attach_ms(10000, connectToMqtt);
  TickerForfetchIP.attach(30, fetchIP);
  if(!debugging)
  {
    TickerForSerialListner.attach_ms(10, SerialListner);
    send_status_uart();
  }
/*-------Setting up the trikers-----------------------------*/    
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
  webServer.serveStatic("/script.js", SPIFFS, "/script.js");
  webServer.serveStatic("/style.css", SPIFFS, "/style.css");
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    File index = SPIFFS.open("/index.html", "r");
    if (index) {
      request->send(SPIFFS, "/index.html", "text/html");
    }
    else{
      request->redirect("/update");
    }
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
  if(strcmp(DEVICE_V, "v2") == 0)
  {
    if(millis()%2000 == 0)
    {
      dht.temperature().getEvent(&event);
      if (!isnan(event.temperature))
        temp = event.temperature-8;
      dht.humidity().getEvent(&event);
      if (!isnan(event.relative_humidity))
        humid = event.relative_humidity;
      light = map(analogRead(LDR_PIN), 0, 255, 0, 100);
    }
  }
}