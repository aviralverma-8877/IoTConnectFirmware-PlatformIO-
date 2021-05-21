#include "global_var_two.h"

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
WebSocketsServer webSocket = WebSocketsServer(81);
bool reconnect_mqtt = false;
bool reset_btn_status = false;
byte reset_btn_press_count = 0;
unsigned long reset_btn_press_time = 0;
bool debugging = false;                   //Turn On or Off the serial output.
String websocket_msg = "";
bool subscribed_to_mqtt_topics = false;
/*--------------MQTT Configration---------------------------*/
bool MQTTStatus = false;
String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
String outtopic = chipid+"/RESPONSE";          //MQTT Topic for sending data from ESP.
String intopic = chipid+"/COMMAND";
String espsensor = chipid+"/ESP_SENSOR";           //MQTT Topic for sending sensor status data.
/*--------------MQTT Configration---------------------------*/
