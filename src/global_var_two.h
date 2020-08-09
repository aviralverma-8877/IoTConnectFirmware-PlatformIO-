#ifndef global_var_two
    #define global_var_two 1
    #include <Arduino.h>
    extern String chipid;  //Fetching ESP device ID.
    extern bool debugging;                   //Turn On or Off the serial output.
    extern String debugtopic;      //MQTT Topic for sending debug data.
    extern String outtopic;          //MQTT Topic for sending data from ESP.
    extern String intopic;            //MQTT Topic for reciving data to ESP.
    extern String DefaultResponseHTML;
#endif