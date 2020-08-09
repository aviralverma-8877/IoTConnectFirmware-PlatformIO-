#ifndef global_var_two
    #define global_var_two 1
    #include <Arduino.h>
    String chipid;  //Fetching ESP device ID.
    bool debugging;                   //Turn On or Off the serial output.
    String debugtopic;      //MQTT Topic for sending debug data.
    String outtopic;          //MQTT Topic for sending data from ESP.
    String intopic;            //MQTT Topic for reciving data to ESP.
    String DefaultResponseHTML;
#endif