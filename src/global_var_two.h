#ifndef global_var_two
    #define global_var_two
    #include <Arduino.h>
    #include <WebSocketsServer.h>
    /*--------------MQTT global Configration---------------------------*/
    #ifndef norttopic
        #define norttopic "NORTIFICATION"                //MQTT Topic for sending nortifications.
    #endif
    #ifndef espraw
        #define espraw "ESP_ATTENDANCE"                 //MQTT Topic for sending device attendance.
    #endif
    #ifndef espstatus
        #define espstatus "ESP_STATUS"           //MQTT Topic for sending relay status data.
    #endif
    #ifndef espsensor
        #define espsensor "ESP_SENSOR"           //MQTT Topic for sending sensor status data.
    #endif
    /*--------------MQTT global Configration---------------------------*/

    extern WebSocketsServer webSocket;
    extern String chipid;  //Fetching ESP device ID.
    extern bool debugging;                   //Turn On or Off the serial output.
    extern String debugtopic;      //MQTT Topic for sending debug data.
    extern String outtopic;          //MQTT Topic for sending data from ESP.
    extern String intopic;            //MQTT Topic for reciving data to ESP.
    extern bool MQTTStatus;
    extern String intopic;
#endif