#include <Arduino.h>


String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
bool debugging = false;                   //Turn On or Off the serial output.
String debugtopic = chipid+"-debug";      //MQTT Topic for sending debug data.
String outtopic = chipid+"-out";          //MQTT Topic for sending data from ESP.
String intopic = chipid+"-in";            //MQTT Topic for reciving data to ESP.
bool MQTTStatus = false;