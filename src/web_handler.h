#ifndef web_handler
    #define web_handler
    #include <Arduino.h>
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>

    void handleWebControl(AsyncWebServerRequest *request);
    void handleWebStatus(AsyncWebServerRequest *request);
    void handleDeviceConfig(AsyncWebServerRequest *request);
    void web_set_wifi(AsyncWebServerRequest *request);
    void web_scan_wifi(AsyncWebServerRequest *request);
    void web_update_login(AsyncWebServerRequest *request);
    void firmware_web_updater();
#endif