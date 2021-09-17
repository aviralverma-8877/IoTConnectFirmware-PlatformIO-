#ifndef device_handler
    #define device_handler
    #include<Arduino.h>
    #include<ArduinoJson.h>
    #include <ESPmDNS.h> 
    #include <DHT.h>
    #include <Ticker.h>
    #include "FS.h"
    #include "freertos/ringbuf.h"
    #include "SPIFFS.h"
    #include "mqtt_handler.h"
    #include "global_var_one.h"
    #include "global_var_two.h"
    #include "common_meathods.h"
    #include "web_sockets_handler.h"
    #include "mqtt_handler.h"
    #include "web_handler.h"

    void setup_tickers();
    void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info);
    void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
    String device_status();
    String read_mqtt_config();  
    void relay_action(String relay, bool value, String by);
    void updateESPSpiffs();
    void switch_wifi();
    void feedbackLED();
    void reset();
    void pinging();
    void setup_sensor();
    void sendSensorData();
    void checkReset();
    bool comp(const char *val1,const char *val2);
    void fetchIP();
    void connectToWiFi();
    void print_config();
    void read_config();
    void generate_mqtt_topics(DynamicJsonDocument doc);
    String read_device_config();
    void toggle_relay(String relay);
    void perform_action();
    void perform_action(String relay, bool value);
#endif