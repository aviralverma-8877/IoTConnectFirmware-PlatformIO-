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
    setup_tickers();
    setup_mqtt();
  }
}

void loop() 
{
  MDNS.update();
  webSocket.loop();
  callback();
}