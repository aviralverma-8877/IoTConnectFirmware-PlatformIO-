#ifndef web_handler
    #define web_handler
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


    void handleWebControl(AsyncWebServerRequest *request);
    void handleWebStatus(AsyncWebServerRequest *request);
    void handleDeviceConfig(AsyncWebServerRequest *request);
    void web_set_wifi(AsyncWebServerRequest *request);
    void web_scan_wifi(AsyncWebServerRequest *request);
    void web_update_login(AsyncWebServerRequest *request);
    void firmware_web_updater();
    void setup_web_server();
    void enable_ap();
    void disable_ap();
#endif