#include<Arduino.h>
#include"structures.h"

configuration conf = {{ 0,0,0,0,0,0,0,0 },false,false,true,2000,"admin","admin"};
String IpAddress = "";                    //Global variables
String LocalIP = "";                      //Global variables
bool mqtt_setup = false;                  //Global variables
bool first_connect = false;               //Global variables
bool inSetup = true;
byte loopCount = 0;                       //Global variables
uint8_t attempts = 0;                     //Global variables
bool shouldReboot = false;
String wifi_ssid = "";
String wifi_pass = "";
/*----------------------------------------------------------*/
/*--------------MQTT Configration---------------------------*/
String norttopic = "NORT";                //MQTT Topic for sending nortifications.
String sensortopic = "ESPSENSOR";         //MQTT Topic for sending sensor data.
String espstatus = "ESPSTATUS";           //MQTT Topic for sending relay status data.
String espraw = "ESPRAW";                 //MQTT Topic for sending device attendence.
/*--------------MQTT Configration---------------------------*/