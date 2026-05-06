#ifndef device_handler
    #define device_handler
    #include<Arduino.h>
    #include<ArduinoJson.h>
    #include <ESP8266mDNS.h> 
    #include <DHT.h>
    #include "mqtt_handler.h"
    #include "global_var_one.h"
    #include "global_var_two.h"
    #include "common_methods.h"
    #include "web_sockets_handler.h"
    #include "sensor_handler.h"

    void setup_tickers();
    void onWifiConnect(const WiFiEventStationModeGotIP& event);
    void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);
    String device_status();
    String read_mqtt_config();  
    void relay_action(String relay, bool value, String by);
    void updateESPSpiffs();
    void switch_wifi();
    void feedbackLED();
    void reset();
    void pinging();
    void checkReset();
    bool comp(const char *val1,const char *val2);
    void fetchIP();
    void connectToWiFi();
    void print_config();
    void read_config();
    void generate_mqtt_topics();
    String read_device_config();
    void toggle_relay(String relay);
    void perform_action();
    void perform_action(String relay, bool value);
    void fauxmo_sync_state(String relay_name, bool state);
    void fauxmo_sync_all_states();
#endif