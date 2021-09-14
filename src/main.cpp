//Including Libraries
#include <Arduino.h>

//Include Local libraries
#include "global_var_two.h"
#include "global_var_one.h"
#include "structures.h"
#include "common_meathods.h"
#include "web_handler.h"
#include "mqtt_handler.h"
#include "device_handler.h"
#include "web_sockets_handler.h"
#include "fauxmo_handler.h"

//Include Global libraries
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h> 
#include <ESPmDNS.h>
#include "FS.h"
#include "SPIFFS.h"

void core_0_loop(void *parameter);
void dns_loop(void *parameter);
void fauxmo_loop(void *paramter);
void callback_loop(void *parameter);

void setup() 
{
  if(debugging)
    Serial.begin(115200);
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    serialDisplay("setup","SPIFFS Mount Failed");
    return;
  }
  if (!SPIFFS.exists("/config.json")) 
  {
    configuration newConfig = {false,false,true,false,2000,"N/A","admin","admin","","",false,"N/A","N/A","N/A"};
    newConfig.setupFlag = true;
    newConfig.wifi_setup_done = false;
    write_config(newConfig);
  }
  delay(100);
  read_config();
  print_config();
  delayMS = conf.pingTime;
  configure_gpio();
  if(conf.save_eeprom)
    perform_action();
  callback = &blank;
  if(!conf.setupFlag)
  {
    WiFi.onEvent(onWifiConnect, SYSTEM_EVENT_STA_GOT_IP);
    WiFi.onEvent(onWifiDisconnect, SYSTEM_EVENT_STA_DISCONNECTED);
  }
  initWebSocket();
  setup_web_server();    //Webserver Handler
  serialDisplay("setup","ap_enabled "+String(ap_enabled));
  serialDisplay("setup","Current Core : " + String(xPortGetCoreID()));
  if(!ap_enabled)
  {
    fetchIP();             //Fetching Public and Local IP
    setup_fauxmo();        //Fauxmo Alexa handler
    setup_sensor();        //DHT and LDR Setup
    setup_tickers();       //Ticker Setup
  }
  else{
    if(hasSensor)
    {
      setup_sensor();        //DHT and LDR Setup
      TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
    }
  }
  xTaskCreatePinnedToCore(core_0_loop, "core_0_loop", 10000, NULL, 1, &loop_run, 0);

}
void core_0_loop(void *parameter)
{
  xTaskCreate(dns_loop, "dns_loop", 10000, NULL, 2, NULL);
  xTaskCreate(fauxmo_loop, "fauxmo_loop", 10000, NULL, 3, NULL);
  xTaskCreate(callback_loop, "callback_loop", 10000, NULL, 4, NULL);
  vTaskDelete(NULL);
}
void dns_loop(void *parameter)
{
  for(;;)
  {
    dnsServer.processNextRequest();
    delay(100);
  }
  vTaskDelete(NULL);
}

void fauxmo_loop(void *paramter)
{
  for(;;)
  {
    fauxmo.handle();
    delay(10);
  }
  vTaskDelete(NULL);
}

void callback_loop(void *parameter)
{
  for(;;)
  {
    callback();
    delay(10);
  }
  vTaskDelete(NULL);
}

void loop() 
{
  vTaskDelete(NULL);
}