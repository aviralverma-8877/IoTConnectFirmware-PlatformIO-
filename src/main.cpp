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
  if (SPIFFS.exists("/config.json")) {
    read_config();
    delayMS = conf.pingTime;
  }
  else{
    configuration newConfig = {false,false,true,2000,"admin","admin","","",false};
    newConfig.setupFlag = true;
    newConfig.wifi_setup_done = false;
    write_config(newConfig);
    read_config();
  }
  delay(1000);
  setup_web_server();
  callback = &blank;
  if(WiFi.status() == WL_CONNECTED)
  {
    configure_gpio();
    TickerForTimeOut.once_ms(10,[](){
      perform_action();
    });
/*-------Setting up the trikers-----------------------------*/
    TickerForconnectToMqtt.attach(10, connectToMqtt);
    TickerForfetchIP.attach(10, fetchIP);
    TickerForFeedbackLED.attach(0.6, feedbackLED);
    TickerForcheckReset.attach_ms(10, checkReset);
/*-------Setting up the trikers-----------------------------*/      
    mqtt.onConnect(onMqttConnect);
    mqtt.onDisconnect(onMqttDisconnect);
    mqtt.onSubscribe(onMqttSubscribe);
    mqtt.onUnsubscribe(onMqttUnsubscribe);
    mqtt.onMessage(onMqttMessage);
  //  mqtt.onPublish(onMqttPublish);
    fetchIP();
    connect_to_mqtt();
  }
}

void loop() 
{
  MDNS.update();
  webSocket.loop();
  callback();
}