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
#include "fauxmo_handler.h"

void setup() 
{
  if(debugging)
    Serial.begin(115200);
  SPIFFS.begin();
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
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  if(conf.setupFlag)
  {
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);
  }
  setup_web_server();    //Webserver Handler
  serialDisplay("ap_enabled",String(ap_enabled));
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
}

void loop() 
{
  MDNS.update();
  webSocket.loop();
  if(ap_enabled)
  {
    dnsServer.processNextRequest();  
  }
  else{
    fauxmo.handle();
  }
  callback();
}