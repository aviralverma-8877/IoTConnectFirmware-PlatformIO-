#ifndef common_meathods
    #define common_meathods
    #include <Arduino.h>
    #include "structures.h"
    extern void (*callback)(void);                                 //Callback function meathod
    void write_config(configuration config);
    void serialDisplay(String head,String body);
    void blank();
    String IpAddress2String(const IPAddress& ipAddress);
    void write_device_config(StaticJsonDocument<500> jsonBuffer);
    void write_mqtt_topics(StaticJsonDocument<1000> jsonBuffer);
#endif