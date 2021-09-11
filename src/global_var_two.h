#ifndef global_var_two
    #define global_var_two
    #include <Arduino.h>
    #include <ESPAsyncWebServer.h>
    /*--------------MQTT global Configration---------------------------*/
    #ifndef norttopic
        #define norttopic "NORTIFICATION"                //MQTT Topic for sending nortifications.
    #endif
    #ifndef espraw
        #define espraw "ESP_ATTENDANCE"                 //MQTT Topic for sending device attendance.
    #endif
    #ifndef espaction
        #define espaction "ESP_ACTION"           //MQTT Topic for sending sensor status data.
    #endif
    /*--------------MQTT global Configration---------------------------*/
    extern bool reconnect_mqtt;
    extern bool reset_btn_status;
    extern byte reset_btn_press_count;
    extern unsigned long reset_btn_press_time;
    extern String chipid;  //Fetching ESP device ID.
    extern bool debugging;                   //Turn On or Off the serial output.
    extern String outtopic;          //MQTT Topic for sending data from ESP.
    extern String intopic;            //MQTT Topic for reciving data to ESP.
    extern bool MQTTStatus;
    extern String intopic;
    extern String espsensor;
    extern String websocket_msg;
    String getMacAddress();
#endif