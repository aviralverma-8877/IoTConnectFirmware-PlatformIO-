#ifndef common_meathods
    #define common_meathods 1
    #include <Arduino.h>
    void (*callback)(void);                                 //Callback function meathod
    void write_config(configuration config);
    void serialDisplay(String head,String body);
    void blank();
    String IpAddress2String(const IPAddress& ipAddress);
#endif