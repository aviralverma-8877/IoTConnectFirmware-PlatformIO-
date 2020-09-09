#ifndef fauxmo_handler
    #define fauxmo_handler
    #include <Arduino.h>
    #include <ESP8266WiFi.h>
    #include <ESPAsyncWebServer.h>
    #include "fauxmoESP.h"
    #include "global_var_one.h"
    #include "device_handler.h"
    
    void setup_fauxmo();
    void fauxmo_add_device(const char* device_name);
    void fauxmo_remove_device(const char* device_name);
    void fauxmo_remove_all_device();
    void fauxmo_add_device();
#endif