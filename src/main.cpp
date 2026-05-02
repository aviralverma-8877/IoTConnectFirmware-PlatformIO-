//Including Libraries
#include <Arduino.h>

#include "user_interface.h"
// removed: #include "uart.h"

#define BIT_RATE_115200 115200

// remove custom user_init() that called uart_init(...) (wrong prototype / unsafe)
// ...existing code...

//Include Local libraries
#include "global_var_two.h"
#include "global_var_one.h"
#include "structures.h"
#include "common_methods.h"
#include "web_handler.h"
#include "mqtt_handler.h"
#include "device_handler.h"
#include "sensor_handler.h"
#include "web_sockets_handler.h"
#include "fauxmo_handler.h"

//Include Global libraries
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h> 
#include <ESP8266mDNS.h>

void setup() 
{
  // disable SDK os_printf as early as possible
  system_set_os_print(false);

  // use Arduino Serial API instead of calling uart_init directly
  if (debugging) {
    Serial.begin(BIT_RATE_115200);
    delay(50); // let UART settle
  }

  if (!LittleFS.begin()) {
    serialDisplay("Setup","Failed to mount LittleFS. Formatting might be needed or power issue.");
    return;
  }
  else{
    serialDisplay("Setup","LittleFS initialized.");
  }
  if (!LittleFS.exists("/config.json")) 
  {
    serialDisplay("Setup","No config file.");
    configuration newConfig = {false,false,true,false,2000,"N/A","admin","admin","","",false,"{}"};
    newConfig.setupFlag = true;
    newConfig.wifi_setup_done = false;
    write_config(newConfig);
  }
  delay(100);
  read_config();
  print_config();
  {
    String dc = read_device_config();
    JsonDocument _dc;
    deserializeJson(_dc, dc);
    initSetupDone = _dc["init_setup_done"] | false;
  }
  delayMS = conf.pingTime;
  configure_gpio();
  if(conf.save_eeprom)
    perform_action();
  callback = &blank;
  if(!conf.setupFlag)
  {
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  }
  initWebSocket();
  setup_web_server();    //Webserver Handler
  serialDisplay("setup","ap_enabled"+String(ap_enabled));
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
  dnsServer.processNextRequest();
  MDNS.update();
  fauxmo.handle();
  webSocket.cleanupClients();
  callback();
}