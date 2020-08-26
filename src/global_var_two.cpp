#include <Arduino.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);
bool reset_btn_status = false;
byte reset_btn_press_count = 0;
unsigned long reset_btn_press_time = 0;
/*--------------MQTT Configration---------------------------*/
String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
bool debugging = false;                   //Turn On or Off the serial output.
String outtopic = chipid+"/RESPONSE";          //MQTT Topic for sending data from ESP.
bool MQTTStatus = false;
String intopic = chipid+"/COMMAND";
/*--------------MQTT Configration---------------------------*/
