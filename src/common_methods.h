#ifndef common_methods
    #define common_methods
    #include <Arduino.h>
    #include "structures.h"
    #include <Arduino.h>
    #include "FS.h"
    #include "LittleFS.h"
    #include <ArduinoJson.h>
    #include "mqtt_handler.h"
    #include "global_var_two.h"
    #include "global_var_one.h"
    extern void (*callback)(void);                                 //Callback function method
    void write_config(configuration config);
    void serialDisplay(String head,String body);
    void blank();
    String IpAddress2String(const IPAddress& ipAddress);
    void write_device_config(JsonDocument jsonBuffer);
    void write_mqtt_topics(String r);
    String config_to_json(configuration config);
#endif