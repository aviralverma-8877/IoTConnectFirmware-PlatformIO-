//Including Libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266WebServer.h>             //Local WebServer used to serve the configuration portal
#include <AsyncMqttClient.h>              //Async MQTT Library
#include <Ticker.h>                       //Ticker for running multithread
#include <ESP_EEPROM.h>                   //Reading and writing on EEPROM
#include <ArduinoJson.h>                  //Encoading and Decoding JSON
#include <Adafruit_Sensor.h>              //For LDR
#include <DHT.h>                          //For DHT Temperature and Humidity Sensor
#include <DHT_U.h>                        //For DHT Temperature and Humidity Sensor
#include <ShiftRegister74HC595.h>         //For controlling Relays from 74HC595 shift register
#include <DNSServer.h>                    //For redirecting the user on connecting to device WiFi
#include <WiFiManager.h>                  //WiFi Manager library.
#include <ESP8266HTTPClient.h>            //HTTP Client library.
#include <ESP8266httpUpdate.h>            //ESP Update Library.

#define MQTT_HOST "iot-connect.in"        //MQTT Server address
#define MQTT_PORT 1883                    //MQTT Server port
//MQTT Cred
#define MQTT_UNAME "iotconnect"
#define MQTT_PASS "iot-12345"

#define LDR_PIN A0                        //LDR Pin address
#define reset_btn 4                       //Reset Button pin
#define indicator_led 13                  //LED Pin
#define DHTPIN 2                          //DHT single wire interface pin
#define DHTTYPE DHT11                     //Type of DHT sensor.

//Configuring Device
#define FIRMWARE_V "0.0.8"                //Current firmware version. (Displayed on Device Portal)
#define DEVICE_V   "v1"                   //Device type version (V1 - Without Sensor)
                                                              //(V2 - With Sensor)
                                          //Should not modify the vesions, as website device portal is set accordingly.
bool debugging = false;                   //Turn On or Off the serial output.

DynamicJsonBuffer jsonBuffer;             //JSON Buffer variable 1.
DynamicJsonBuffer jsonBuffert;            //JSON Buffer variable 2.

AsyncMqttClient mqtt;                     //Variable to initiate MQTT.
WiFiManager wifiManager;                  //Variable to initiate WiFi Manager

DHT_Unified dht(DHTPIN, DHTTYPE);         //Initializing DHT sensor. 

ShiftRegister74HC595 sr (1, 16, 14, 12);  //Setting up shift register.
String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
sensor_t sensor;                          //DTH sensor
sensors_event_t event;                    //Creating event variable for DHT sensor.

/*----------------------------------------------------------*/
String payload;                           //Global variables
String IpAddress = "";                    //Global variables
String data;                              //Global variables
String Wifi_ssid;                         //Global variables
bool mqtt_setup = false;                  //Global variables
byte loopCount = 0;                       //Global variables
uint8_t attempts = 0;                     //Global variables
uint8_t i;                                //Global variables
HTTPClient http;                          //Global variables
int temp, humid, light;                   //Global variables
uint32_t delayMS;                         //Global variables
const char *updateAddress; //Update address
/*----------------------------------------------------------*/

/*--------------MQTT Configration---------------------------*/
String outtopic = chipid+"-out";          //MQTT Topic for sending data from ESP.
String intopic = chipid+"-in";            //MQTT Topic for reciving data to ESP.
String debugtopic = chipid+"-debug";      //MQTT Topic for sending debug data.
String norttopic = "NORT";                //MQTT Topic for sending nortifications.
String sensortopic = "ESPSENSOR";         //MQTT Topic for sending sensor data.
String espstatus = "ESPSTATUS";           //MQTT Topic for sending relay status data.
String espraw = "ESPRAW";                 //MQTT Topic for sending device attendence.
/*--------------MQTT Configration---------------------------*/

/*--------------Relay Default Configration------------------*/
struct configuration{
  bool pinValues[8] = { 0,0,0,0,0,0,0,0 };  //Default relay values.
  bool setupFlag = false;
  bool updateFlag = false;
  int pingTime = 2000;
} conf;

/*--------------Relay Default Configration------------------*/
/*--------------Tickers for Async Meathods------------------*/
Ticker TickerForPinging;
Ticker TickerForsendSensorData;
Ticker TickerForcheckReset;
Ticker TickerForfetchIP;
Ticker TickerForconnectToMqtt;
Ticker TickerForFeedbackLED;
/*--------------Tickers for Async Meathods------------------*/
void feedbackLED();
void connectToMqtt();
void serialDisplay(String head,String body);
void onMqttConnect(bool sessionPresent);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);
void reset();
void send_status();
void sendToMQTT(String topic, String msg);
void pinging();
void sendSensorData();
void checkReset();
bool comp(const char *val1,const char *val2);
void fetchIP();
void updateESP();
void blank();
void (*callback)(void);                                 //Callback function meathod

void setup() {
  Serial.begin(115200);
/*--------Reading Configs from EEPROM------------------------*/
  EEPROM.begin(sizeof(configuration));
  if(EEPROM.percentUsed()>=0) {
    EEPROM.get(0, conf);
    serialDisplay("EEPROM","EEPROM has data from a previous run.");
    serialDisplay("EEPROM",String(EEPROM.percentUsed())+"% of ESP flash space currently used");
  } else {
    serialDisplay("EEPROM","EEPROM size changed");
    EEPROM.commit();
  }
/*--------Reading Configs from EEPROM------------------------*/
/*--------Setting up the GPIOs-------------------------------*/
  pinMode(reset_btn,INPUT);
  pinMode(indicator_led,OUTPUT);
  pinMode(LDR_PIN,INPUT);
/*--------Setting up the GPIOs-------------------------------*/  
  wifiManager.setDebugOutput(false); //Set WiFi manager debug output.
  WiFi.mode(WIFI_STA);
/*-------Start WiFi Manager---------------------------------*/
  if(conf.setupFlag)
  {
    conf.setupFlag = false;
    EEPROM.put(0, conf);
    EEPROM.commit();
    TickerForFeedbackLED.attach(0.2, feedbackLED);
    wifiManager.startConfigPortal("IOT Connect");
  }
/*-------Start WiFi Manager---------------------------------*/
  callback = &blank;
  if(strcmp(DEVICE_V, "v2") == 0)
  {
    delayMS = conf.pingTime;
    dht.begin();          //Initalizing DHT sensor.
  }
/*--------Reading data from EEPROM for last relay status ----*/
  for(int t=0; t<8; t++)
  {
    sr.set(t,conf.pinValues[t]);           //Overwriting saved values on default values.
    delay(100);
  }
/*--------Reading data from EEPROM for last relay status ----*/
/*-------Fetch IP Address-----------------------------------*/
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    checkReset();
  }
  fetchIP();    
/*-------Fetch IP Address-----------------------------------*/
/*-------Setting up MQTT------------------------------------*/
  mqtt.onConnect(onMqttConnect);
  mqtt.onDisconnect(onMqttDisconnect);
  mqtt.onSubscribe(onMqttSubscribe);
  mqtt.onUnsubscribe(onMqttUnsubscribe);
  mqtt.onMessage(onMqttMessage);
  mqtt.onPublish(onMqttPublish);
  mqtt.setCredentials(MQTT_UNAME, MQTT_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
/*-------Setting up MQTT------------------------------------*/
/*-------Setting up the trikers-----------------------------*/
  TickerForcheckReset.attach_ms(10, checkReset);
  TickerForconnectToMqtt.attach_ms(10000, connectToMqtt);
  TickerForFeedbackLED.attach(0.6, feedbackLED);
/*-------Setting up the trikers-----------------------------*/    
}
/*-------feedbackLED----------------------------------------*/
void feedbackLED()
{
  int state = digitalRead(indicator_led);  // get the current state of GPIO1 pin
  digitalWrite(indicator_led, !state);     // set pin to the opposite state
}
/*-------feedbackLED----------------------------------------*/

void connectToMqtt() {
  if(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  else
  {
  }
  mqtt.connect();
}
/*-------Meathod for displaying serial data in JSON---------*/
void serialDisplay(String head,String body)
{
  if(debugging)
  {
    jsonBuffer.clear();
    JsonObject& root = jsonBuffer.createObject();
    root["action"] = "display";
    root["head"] = head;
    root["body"] = body;
    String c;
    root.printTo(c);
    Serial.println(c);
    sendToMQTT(debugtopic, c);
  }
}
/*-------Meathod for displaying serial data in JSON---------*/
/*-------Meathod called when connected to MQTT--------------*/
void onMqttConnect(bool sessionPresent) {
  serialDisplay("MQTT","MQTT is Connected");
  mqtt.subscribe(intopic.c_str(), 2);
  TickerForPinging.attach_ms(500, pinging);
  if(strcmp(DEVICE_V, "v2") == 0)
    TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
}
/*-------Meathod called when connected to MQTT--------------*/
/*-------Meathod called when subscribed to MQTT Topic-------*/
void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  serialDisplay("MQTT","Subscribed to MQTT");
  TickerForFeedbackLED.detach();
  digitalWrite(indicator_led, LOW);     // set pin to the opposite state
}
/*-------Meathod called when subscribed to MQTT Topic-------*/
/*-------Meathod called when unsubscribed from MQTT Topic---*/
void onMqttUnsubscribe(uint16_t packetId) {
  serialDisplay("MQTT","Unsubscribed from "+packetId);
}
/*-------Meathod called when unsubscribed from MQTT Topic---*/

/*-------Meathod called when disconnected from MQTT Topic---*/
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  serialDisplay("MQTT","MQTT is disconnected.");
  TickerForFeedbackLED.attach(0.6, feedbackLED);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  connectToMqtt();
}
/*-------Meathod called when disconnected from MQTT Topic---*/

/*-------Meathod called on reciving message from MQTT-------*/
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String p = "";
  for(int i=index; (unsigned)i<len;i++)
  {
    p += payload[i];
  }
  jsonBuffert.clear();
  serialDisplay("MQTT",p);
  JsonObject& root = jsonBuffert.parseObject(p);
  if (root.success()) 
  {
    const char *action;
    action = root["action"];
/*-------Action command for reconfigring WiFi---------------*/
    if(comp(action,"RESET_DEVICE"))
    {
      jsonBuffer.clear();
      JsonObject& sJsont = jsonBuffer.createObject();
      sJsont["action"] = "ResetDevice";
      sJsont["stat"] = "Reset Success.";
      String r;
      sJsont.printTo(r);
      sendToMQTT(outtopic, r);
      reset();
    }
/*-------Action command for reconfigring WiFi---------------*/
/*-------Action command for fetching WiFi SSID--------------*/
    if(comp(action,"SSID"))
    {
      jsonBuffer.clear();
      JsonObject& sJson = jsonBuffer.createObject();
      sJson["SSID"] = Wifi_ssid;
      String r;
      sJson.printTo(r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for fetching WiFi SSID--------------*/
/*-------Action command for setting Sensor Frequency--------------*/
    if(comp(action,"FREQ"))
    {
      delayMS = root["f"];
      conf.pingTime = delayMS;      //Saving the delay time in eeprom
      EEPROM.put(0, conf);
      EEPROM.commit();
      TickerForsendSensorData.detach();
      TickerForsendSensorData.attach_ms(delayMS, sendSensorData);
    }
/*-------Action command for setting Sensor Frequency--------------*/
/*-------Action command for getting Sensor Frequency--------------*/
    if(comp(action,"GETFREQ"))
    {
      jsonBuffer.clear();
      JsonObject& sJson = jsonBuffer.createObject();
      sJson["FREQ"] = delayMS;
      String r;
      sJson.printTo(r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for getting Sensor Frequency--------*/
/*-------Action command for fetching ESP Chip ID------------*/
    if(comp(action,"ESPID"))
    {
      jsonBuffer.clear();
      JsonObject& sJson = jsonBuffer.createObject();
      sJson["CHIPID"] = chipid;
      String r;
      sJson.printTo(r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for fetching ESP Chip ID------------*/

/*-------Action command for resetting ESP-------------------*/
    if(comp(action,"RESET"))
    {
      jsonBuffer.clear();
      JsonObject& sJsont = jsonBuffer.createObject();
      sJsont["action"] = "ResetStatus";
      sJsont["stat"] = "Resetting Device...";
      String r;
      sJsont.printTo(r);
      sendToMQTT(outtopic, r);
      ESP.reset();
    }
/*-------Action command for resetting ESP-------------------*/

/*-------Action command for controlling Relays--------------*/
    else if(comp(action,"RELAY"))
    {
      sr.set(root["no"], root["value"]);
//sending status
      send_status();
      delay(100);
      
//sending nortification
      jsonBuffer.clear();
      JsonObject& sJson = jsonBuffer.createObject();
      if(root.containsKey("by"))
      {
        sJson["by"] = root["by"];
        sJson["no"] = root["no"];
        sJson["CHIPID"] = chipid;
        sJson["input"] = root["value"];
      }
      String r = "";
      sJson.printTo(r);
      sendToMQTT(norttopic, r);

//saving to eeprom
      for(int t=0; t<8; t++)
      {
        conf.pinValues[t] = sr.get(t);
      }
      EEPROM.put(0, conf);
      EEPROM.commit();
    }
/*-------Action command for controlling Relays--------------*/
/*-------Action command for getting Relays status-----------*/
    else if(comp(action,"STATUS"))
    {
      send_status();
    }
/*-------Action command for getting Relays status-----------*/
/*-------Action command for getting getting device version--*/
    else if(comp(action,"GET_DEVICE_VERSION"))
    {
      jsonBuffer.clear();
      JsonObject& sJsont = jsonBuffer.createObject();
      sJsont["action"] = "Device Version";
      sJsont["value"] = DEVICE_V;
      String r;
      sJsont.printTo(r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for getting getting device version--*/
/*-----Action command for getting getting firmware version--*/
    else if(comp(action,"GET_VERSION"))
    {
      jsonBuffer.clear();
      JsonObject& sJsont = jsonBuffer.createObject();
      sJsont["action"] = "Firmware Version";
      sJsont["value"] = FIRMWARE_V;
      String r;
      sJsont.printTo(r);
      sendToMQTT(outtopic, r);
    }
/*-----Action command for getting getting firmware version--*/
/*-------Action command for updating firmware---------------*/
    else if(comp(action,"UPDATE"))
    {
      updateAddress = root["url"];
      callback = &updateESP;
    }
/*-------Action command for updating firmware---------------*/
  }
}
/*-------Meathod called on reciving message from MQTT-------*/
/*-------Meathod to update ESP------------------------------*/
void updateESP()
{
  jsonBuffer.clear();
  JsonObject& sJson = jsonBuffer.createObject();
  sJson["action"] = "UpdateStatus";
  sJson["stat"] = "Updating Device...";
  String r;
  sJson.printTo(r);
  sendToMQTT(outtopic, r);
  delay(100);
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateAddress);
  String stat = "";
  switch(ret) 
  {
    case HTTP_UPDATE_FAILED:
        stat = "Failed to update.";
        break;
    case HTTP_UPDATE_NO_UPDATES:
        stat = "No Update Available...";
        break;
    case HTTP_UPDATE_OK:
        stat = "Update Successfull...";
        break;
  }
  jsonBuffer.clear();
  JsonObject& sJsont = jsonBuffer.createObject();
  sJsont["action"] = "UpdateStatus";
  sJsont["stat"] = stat;
  r = "";
  sJsont.printTo(r);
  sendToMQTT(outtopic, r);
  delay(100);
  serialDisplay("Update",stat);
}
/*-------Meathod to update ESP------------------------------*/
/*----Meathod called on sending/publishing message on MQTT--*/
void onMqttPublish(uint16_t packetId) {
  serialDisplay("MQTT","Message sent on "+packetId);
}
/*----Meathod called on sending/publishing message on MQTT--*/

/*----Meathod for reconfiguring WiFi settings---------------*/
void reset()
{
  digitalWrite(indicator_led,HIGH);
  WiFi.disconnect();
  conf.setupFlag = true;
  EEPROM.put(0, conf);
  EEPROM.commit();
  ESP.reset();
}
/*----Meathod for reconfiguring WiFi settings---------------*/
/*----Meathod for sending relay status----------------------*/
void send_status()
{
  jsonBuffer.clear();
  JsonObject& sJson = jsonBuffer.createObject();
  sJson["d"] = chipid;
  sJson["action"] = "s";
  JsonArray& data = sJson.createNestedArray("v");
  for(int t=0; t<8; t++)
  {
    data.add(sr.get(t));
  }
  String r;
  sJson.printTo(r);
  sendToMQTT(outtopic, r);
  sendToMQTT(espstatus, r);
}
/*----Meathod for sending relay status----------------------*/
/*----Meathod for sending MQTT Data-------------------------*/
void sendToMQTT(String topic, String msg)
{
  mqtt.publish(topic.c_str(), 0, false, msg.c_str(), msg.length());
}
/*----Meathod for sending MQTT Data-------------------------*/
/*---Meathod for pinging MQTT Server for active connection--*/
void pinging()
{
  jsonBuffer.clear();
  JsonObject& sJson = jsonBuffer.createObject();
  sJson["d"] = chipid;
  if(strcmp(DEVICE_V, "v2") == 0)
    sJson["s"] = true;
  else
    sJson["s"] = false;
  sJson["i"] = IpAddress;
  String r;
  sJson.printTo(r);
  sendToMQTT(espraw, r);
}
/*---Meathod for pinging MQTT Server for active connection--*/
/*-----Meathod for sending sensor data----------------------*/
void sendSensorData()
{
  jsonBuffer.clear();
  JsonObject& sJson = jsonBuffer.createObject();
  sJson["d"] = chipid;
  sJson["t"] = temp;
  sJson["h"] = humid;
  sJson["l"] = light;
  String s;
  sJson.printTo(s);
  sendToMQTT(sensortopic, s);
}
/*-----Meathod for sending sensor data----------------------*/
/*-----Meathod for checking reset button--------------------*/
void checkReset()
{
  if(digitalRead(reset_btn) == HIGH)
  {
    digitalWrite(indicator_led,HIGH);
    WiFi.disconnect();
    conf.setupFlag = true;
    EEPROM.put(0, conf);
    EEPROM.commit();
    ESP.reset();
//    wifiManager.startConfigPortal("IOT Connect");
  }
}
/*-----Meathod for checking reset button--------------------*/
/*-----Meathod for comparing string-------------------------*/
bool comp(const char *val1,const char *val2)
{
  bool res = true;
  if(strlen(val1) != strlen(val2))
    res = false;
  for(int y=0;(unsigned)y<strlen(val1);y++)
  {
    if(val1[y] != val2[y])
      res = false;
  }
  return res;
}
/*-----Meathod for comparing string-------------------------*/
/*-----Meathod for feyching IP Address----------------------*/
void fetchIP()
{
  http.begin("http://api.ipify.org/?format=json");
  //int httpCode = http.GET();
  String payload = http.getString();
  http.end();
  jsonBuffer.clear();
  JsonObject& root = jsonBuffer.parseObject(payload);
  IpAddress = "";
  root["ip"].printTo(IpAddress);
  jsonBuffer.clear();
  Wifi_ssid = WiFi.SSID();
  serialDisplay("SSID",Wifi_ssid);
  serialDisplay("IP Address",IpAddress);
}
/*-----Meathod for fetching IP Address----------------------*/
/*-----Blank function-----------------------------------------*/
void blank()
{
  
}
/*-----Blank function-----------------------------------------*/
void loop() 
{
  callback();
  if(strcmp(DEVICE_V, "v2") == 0)
  {
    if(millis()%2000 == 0)
    {
      dht.temperature().getEvent(&event);
      if (!isnan(event.temperature))
        temp = event.temperature-8;
      dht.humidity().getEvent(&event);
      if (!isnan(event.relative_humidity))
        humid = event.relative_humidity;
      light = map(analogRead(LDR_PIN), 0, 1023, 0, 100);
    }
  }
}