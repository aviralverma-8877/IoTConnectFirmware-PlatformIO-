#include <Arduino.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);
bool reset_btn_status = false;
byte reset_btn_press_count = 0;
unsigned long reset_btn_press_time = 0;
bool debugging = false;                   //Turn On or Off the serial output.
/*--------------MQTT Configration---------------------------*/
bool MQTTStatus = false;
String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
String outtopic = chipid+"/RESPONSE";          //MQTT Topic for sending data from ESP.
String intopic = chipid+"/COMMAND";
String espsensor = chipid+"/ESP_SENSOR";           //MQTT Topic for sending sensor status data.
/*--------------MQTT Configration---------------------------*/
