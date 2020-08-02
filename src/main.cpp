//Including Libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>                  //https://github.com/esp8266/Arduino
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>          //Async WiFi Manager
#include <AsyncMqttClient.h>              //Async MQTT Library
#include <Ticker.h>                       //Ticker for running multithread
#include <ESP_EEPROM.h>                   //Reading and writing on EEPROM
#include <ArduinoJson.h>                  //Encoading and Decoding JSON
#include <Adafruit_Sensor.h>              //For LDR
#include <DHT.h>                          //For DHT Temperature and Humidity Sensor
#include <DHT_U.h>                        //For DHT Temperature and Humidity Sensor
#include <ShiftRegister74HC595.h>         //For controlling Relays from 74HC595 shift register
#include <DNSServer.h>                    //For redirecting the user on connecting to device WiFi
#include <ESP8266HTTPClient.h>            //HTTP Client library.
#include <ESP8266httpUpdate.h>            //ESP Update Library.
#include <ESP8266mDNS.h>                  //mdns for setting hostname
#include <FS.h>                           //File System Read
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
#define FIRMWARE_V "2.0.3"                //Current firmware version. (Displayed on Device Portal)
#define DEVICE_V   "v1"                   //Device type version (V1 - Without Sensor)
                                                              //(V2 - With Sensor)
                                          //Should not modify the vesions, as website device portal is set accordingly.
bool debugging = false;                   //Turn On or Off the serial output.

AsyncMqttClient mqtt;                     //Variable to initiate MQTT.

DHT_Unified dht(DHTPIN, DHTTYPE);         //Initializing DHT sensor. 

ShiftRegister74HC595<1> sr (16, 14, 12);  //Setting up shift register.
String chipid = String(ESP.getChipId());  //Fetching ESP device ID.
sensor_t sensor;                          //DTH sensor
sensors_event_t event;                    //Creating event variable for DHT sensor.
/*----------------------------------------------------------*/
/*-------------Webpage Data---------------------------------*/
String DefaultResponseHTML = "<!DOCTYPE html>\
                          <head>\
                            <meta name=\"viewport\" content=\"width=device-width,user-scalable=no,initial-scale=1\">\
                            <title>\
                              ESPID : "+chipid+" | IoT Connect : Solutions for smart homes.\
                            </title>\
                            <link href=\"https://fonts.googleapis.com/css2?family=Fjalla+One&display=swap\" rel=\"stylesheet\">\
                          </head>\
                          <style>\
                          body{\
                            background-color: #e7eaf9;\
                          }\
                          .style_btn\
                          {\
                              background-color: #4CAF50;\
                              border: none;\
                              color: white;\
                              padding: 15px 32px;\
                              text-align: center;\
                              text-decoration: none;\
                              display: inline-block;\
                              font-size: 16px;\
                              margin: 4px 2px;\
                              cursor: pointer;\
                          }\
                          </style>\
                          <script>\
                          function httpGet(relay, value, action)\
                          {\
                              if(action == \"toggle_relay\")\
                              {\
                                theUrl = '/control?command={\"relay\":'+relay+',\"action\":'+value+'}';\
                              }\
                              if(action == \"reset_device\")\
                              {\
                                theUrl = '/control?device={\"action\":\"reset\"}';\
                              }\
                              if(action == \"reboot_device\")\
                              {\
                                theUrl = '/control?device={\"action\":\"reboot\"}';\
                              }\
                              if(action == \"get_status\")\
                              {\
                                theUrl = '/get_status';\
                              }\
                              var xmlHttp = new XMLHttpRequest();\
                              xmlHttp.open( \"GET\", theUrl, false );\
                              xmlHttp.send( null );\
                              return xmlHttp.responseText;\
                          }\
                          function print_table(){\
                            var table = document.getElementById(\"relay_table\");\
                            var content = \"\";\
                            for(var i = 0; i<8; i++)\
                            {\
                              content = content + \"<tr>\
                                <th>\
                                  Relay \"+i+\" : <span id='status-\"+i+\"'></span>\
                                </th>\
                                <th>\
                                  <button class='style_btn' onclick='httpGet(\"+i+\",1,\\\"toggle_relay\\\")'>\
                                    ON\
                                  </button>\
                                  <button class='style_btn' onclick='httpGet(\"+i+\",0,\\\"toggle_relay\\\")'>\
                                    OFF\
                                  </button>\
                                </th>\
                              </tr>\
                              \";\
                            }\
                            table.innerHTML = content;\
                            setInterval(function()\
                            {\
                              data = httpGet(0, 0, 'get_status');\
                              json = JSON.parse(data);\
                              if(json != null)\
                              {\
                                for(var i=0; i<json['relay_status'].length; i++)\
                                {\
                                  var element = document.getElementById('status-'+i);\
                                  if(json['relay_status'][i] == 0)\
                                  {\
                                    element.innerHTML = 'OFF';\
                                  }\
                                  if(json['relay_status'][i] == 1)\
                                  {\
                                    element.innerHTML = 'ON';\
                                  }\
                                }\
                                var cont = \"\";\
                                wifi_ssid = json['wifi_ssid'];\
                                wifi_type = json['type'];\
                                cont = \"Connected to <b>\"+wifi_ssid+\"</b>\";\
                                cont += \" | Device Type <b>\"+wifi_type+\"</b>\";\
                                if(json['temp'] != undefined)\
                                {\
                                  temp = json['temp'];\
                                  humid = json['humid'];\
                                  lumin = json['lumin'];\
                                  cont = cont + \" | Temperature : \"+temp+\" C | Humidity : \"+humid+\" % | Lumin : \"+lumin+\" % \"\
                                }\
                                element = document.getElementById('WiFi_Status');\
                                element.innerHTML = cont;\
                              }\
                            },1000);\
                          }\
                          </script>\
                        <body onload=\"print_table()\">\
                            <h1 style=\"font-family: 'Fjalla One'\">IoT Connect : Solutions for smart homes.</h1>\
                            <hr /><center>\
                            <div id='WiFi_Status' align=\"left\" style=\"font-family: 'Fjalla One'\">\
                            </div>\
                            <div align=\"right\">\
                              <button class='style_btn' onclick=\"if(confirm('Are you sure you want to reset this device?')){httpGet(0,0,'reset_device')}\">Reset</button>\
                              <button class='style_btn' onclick=\"httpGet(0,0,'reboot_device')\">Reboot</button>\
                              <button class='style_btn' onclick=\"location.href = ('/update')\">Update Firmware</button>\
                            </div>\
                            <table id=\"relay_table\" cellspacing=\"15\" style=\"border:#ccc solid thin\">\
                            </table>\
                            </center>\
                        </body>\
                      </html>";
/*-------------Webpage Data---------------------------------*/
/*----------------------------------------------------------*/
String payload;                           //Global variables
String IpAddress = "";                    //Global variables
String LocalIP = "";                      //Global variables
String data;                              //Global variables
String Wifi_ssid;                         //Global variables
bool mqtt_setup = false;                  //Global variables
bool first_connect = false;               //Global variables
byte loopCount = 0;                       //Global variables
uint8_t attempts = 0;                     //Global variables
uint8_t i;                                //Global variables
HTTPClient http;                          //Global variables
int temp, humid, light;                   //Global variables
uint32_t delayMS;                         //Global variables
String updateAddress;                     //Update address
DNSServer dnsServer;                      //Global variables
AsyncWebServer webServer(80);             //Global variables
bool shouldReboot = false;
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
  bool led_enabled = true;
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
Ticker TickerForSerialListner;
Ticker TickerForUARTUpdater;
Ticker TickerForTimeOut;
/*--------------Tickers for Async Meathods------------------*/
void firmware_web_updater();
String serveHTML(String page);
void relay_action(int no, bool value, String by);

void handleWebControl(AsyncWebServerRequest *request);
void handleWebStatus(AsyncWebServerRequest *request);
void web_set_wifi(AsyncWebServerRequest *request);
void web_scan_wifi(AsyncWebServerRequest *request);


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
String IpAddress2String(const IPAddress& ipAddress);
StaticJsonDocument<500> scan_ssid();
void blank();
void (*callback)(void);                                 //Callback function meathod
void SerialListner();
void send_status_uart();
void setup() 
{
  Serial.begin(115200);
  SPIFFS.begin();
/*--------Reading Configs from EEPROM------------------------*/
  EEPROM.begin(sizeof(configuration));
  if(EEPROM.percentUsed()>=0) {
    EEPROM.get(0, conf);
  } else {
    EEPROM.commit();
  }
/*--------Reading Configs from EEPROM------------------------*/
/*--------Setting up the GPIOs-------------------------------*/
  pinMode(reset_btn,INPUT);
  pinMode(indicator_led,OUTPUT);
  pinMode(LDR_PIN,INPUT);
  TickerForFeedbackLED.attach(0.6, feedbackLED);
/*--------Setting up the GPIOs-------------------------------*/  
 AsyncWiFiManager wifiManager(&webServer,&dnsServer);
 if(debugging)
 {
  wifiManager.setDebugOutput(true); //Set WiFi manager debug output.
 }
 else
 {
   wifiManager.setDebugOutput(false); //Set WiFi manager debug output.
 }
/*-------Start WiFi Manager---------------------------------*/
  if(conf.setupFlag)
  {
    conf.setupFlag = false;
    EEPROM.put(0, conf);
    EEPROM.commit();
    TickerForFeedbackLED.attach(0.2, feedbackLED);
    wifiManager.resetSettings();
    wifiManager.setBreakAfterConfig(true);
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
    if(debugging)
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
//  mqtt.onPublish(onMqttPublish);
  mqtt.setCredentials(MQTT_UNAME, MQTT_PASS);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
/*-------Setting up MQTT------------------------------------*/
/*-------Setting up the trikers-----------------------------*/
  TickerForcheckReset.attach_ms(10, checkReset);
  TickerForconnectToMqtt.attach_ms(10000, connectToMqtt);
  TickerForfetchIP.attach(30, fetchIP);
  if(!debugging)
  {
    TickerForSerialListner.attach_ms(10, SerialListner);
    send_status_uart();
  }
/*-------Setting up the trikers-----------------------------*/    
/*-------HOST Name Setup------------------------------------*/
 WiFi.hostname("iot-connect-"+chipid);
 MDNS.begin("iot-connect-"+chipid);
/*-------HOST Name Setup------------------------------------*/
/*-------Web Update Server----------------------------------*/
  firmware_web_updater();
/*-------Web Update Server----------------------------------*/
/*-------Web Server Setup-----------------------------------*/
  webServer.on("/control", HTTP_GET, [](AsyncWebServerRequest *request){
    handleWebControl(request);
  });

  webServer.on("/get_status", HTTP_GET, [](AsyncWebServerRequest *request){
    handleWebStatus(request);
  });
  webServer.on("/scan_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    web_scan_wifi(request);
  });
  webServer.on("/set_wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    web_set_wifi(request);
  });
  webServer.serveStatic("/js", SPIFFS, "/script.js");
  webServer.serveStatic("/css", SPIFFS, "/style.css");
  webServer.serveStatic("/", SPIFFS, "/index.html");
  webServer.onNotFound([](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  webServer.begin();
/*-------Web Server Setup-----------------------------------*/
/*-------HOST Name Setup------------------------------------*/
 MDNS.addService("http", "tcp", 80);
/*-------HOST Name Setup------------------------------------*/
}

/*---------Firmware Update---------------------------------*/
// Simple Firmware Update Form
void firmware_web_updater()
{
  webServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  webServer.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      if(debugging)
        Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
      if(debugging)
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        if(debugging)
          Serial.printf("Update Success: %uB\n", index+len);
        ESP.reset();
      } else {
        if(debugging)
          Update.printError(Serial);
      }
    }
  });
}
/*---------Firmware Update---------------------------------*/
/*-------Serial Listener Setup-----------------------------------*/
void SerialListner()
{
  String message = "";
  StaticJsonDocument<200> doc;
  if(Serial.available())
  {
    char n;
    while(Serial.available())
    {
      n = Serial.read();
      message += n;
    }
    message.trim();
    DeserializationError error = deserializeJson(doc, message);
    if(error)
    {
      return;
    }
    String action = doc["a"];
    int relay = doc["n"];
    bool val = doc["v"];
    if(action == "R")
    {
      relay_action(relay, val, "");
    }
  }

}
/*-------Serial Listener Setup-----------------------------------*/
/*-------Web Server Controller------------------------------*/
void handleWebControl(AsyncWebServerRequest *request)
{
  String message;
  StaticJsonDocument<200> doc;
  int params = request->params();
  if(request->hasParam("command"))
  {
    String command = request->arg("command");
    DeserializationError error = deserializeJson(doc, command);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = 0;
      return_doc["error"] = "error in parsing";
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg);
      return;
    }
    int relay = doc["relay"];
    bool action = doc["action"];
    relay_action(relay, action, "");
    doc["done"] = 1;
    serializeJson(doc, message);
  }
  if(request->hasParam("device"))
  {
    String command = request->arg("device");
    DeserializationError error = deserializeJson(doc, command);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = 0;
      return_doc["error"] = "Error in parsing";
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg); 
      return;
    }
    String action = doc["action"];
    doc["done"] = 1;
    serializeJson(doc, message);
    if(comp(action.c_str(),"reset"))
      reset();
    if(comp(action.c_str(),"reboot"))
      ESP.reset();
    if(comp(action.c_str(),"toggle_onb"))
    {
      bool status = doc["status"];
      conf.led_enabled = status;
      EEPROM.put(0, conf);
      EEPROM.commit();
    }
  }
  request->send(200, "application/json", message);
}
void handleWebStatus(AsyncWebServerRequest *request)
{
  String return_msg = "";
  StaticJsonDocument<500> return_doc;
  JsonArray relay_status = return_doc.createNestedArray("relay_status");
  for(int t=0; t<8; t++)
  {
    relay_status.add(sr.get(t));
  }
  return_doc["type"] = DEVICE_V;
  return_doc["wifi_ssid"] = Wifi_ssid;
  return_doc["wifi_rssi"] = WiFi.RSSI();
  return_doc["onb_led"] = conf.led_enabled;
  if(strcmp(DEVICE_V, "v2") == 0)
  {
    return_doc["t"] = temp;
    return_doc["h"] = humid;
    return_doc["l"] = light;
  }
  serializeJson(return_doc, return_msg);
  request->send(200, "application/json", return_msg); 
}
//    Serial.printf("%d: %s, Ch:%d (%ddBm) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");

void web_scan_wifi(AsyncWebServerRequest *request)
{
  WiFi.scanNetworksAsync([request](int networksFound){
    StaticJsonDocument<500> wifi_ssid;    
    JsonArray ssid = wifi_ssid.createNestedArray("ssid");
    for(int i=0; i<networksFound; i++)
    {
      ssid.add(String(WiFi.SSID(i)));
    }
    String return_msg;
    serializeJson(wifi_ssid, return_msg);
    request->send(200, "application/json", return_msg);
  });
}
void web_set_wifi(AsyncWebServerRequest *request)
{
  if(request->hasParam("options"))
  {
    StaticJsonDocument<200> wifi_option;
    String option = request->arg("options");
    DeserializationError error = deserializeJson(wifi_option, option);
    if (error) 
    {
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = false;
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg); 
      return;
    }
    String ssid = wifi_option["ssid"];
    String pass = wifi_option["pass"];
    WiFi.disconnect();
    WiFi.begin(ssid,pass);
    TickerForTimeOut.once(10,[request](){
      if(WiFi.status() != WL_CONNECTED)
        reset();
      fetchIP();
      String return_msg = "";
      StaticJsonDocument<200> return_doc;
      return_doc["done"] = true;
      serializeJson(return_doc, return_msg);
      request->send(200, "application/json", return_msg); 
    });
  }
  else
  {
    request->send(404);
  }
  
}
/*-------Web Server Controller------------------------------*/
/*-------feedbackLED----------------------------------------*/
void feedbackLED()
{
  if(conf.led_enabled)
  {
    int state = digitalRead(indicator_led);  // get the current state of GPIO1 pin
    digitalWrite(indicator_led, !state);     // set pin to the opposite state
  }
  else
  {
    digitalWrite(indicator_led, LOW);
  }
  
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
    StaticJsonDocument<200> doc;
    doc["action"] = "display";
    doc["head"] = head;
    doc["body"] = body;
    String c;
    serializeJson(doc, c);
    Serial.println(c);
    sendToMQTT(debugtopic, c);
  }
}
/*-------Meathod for displaying serial data in JSON---------*/
/*-------Meathod called when connected to MQTT--------------*/
void onMqttConnect(bool sessionPresent) {
  serialDisplay("MQTT","MQTT is Connected");
  first_connect = true;
  mqtt.subscribe(intopic.c_str(), 2);
  TickerForPinging.attach_ms(10000, pinging);
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
    if(first_connect)
    {
      WiFi.reconnect();
      WiFi.waitForConnectResult();
    }
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
  StaticJsonDocument<200> root;
  serialDisplay("MQTT",p);
  DeserializationError error = deserializeJson(root, p);
  if (!error) 
  {
    const char *action;
    action = root["action"];
/*-------Action command for reconfigring WiFi---------------*/
    if(comp(action,"RESET_DEVICE"))
    {
      StaticJsonDocument<200> doc;
      doc["action"] = "ResetDevice";
      doc["stat"] = "Reset Success.";
      String r;
      serializeJson(doc, r);
      sendToMQTT(outtopic, r);
      reset();
    }
/*-------Action command for reconfigring WiFi---------------*/
/*-------Action command for fetching WiFi SSID--------------*/
    if(comp(action,"SSID"))
    {
      StaticJsonDocument<200> doc;
      doc["SSID"] = Wifi_ssid;
      doc["RSSI"] = String(WiFi.RSSI());
      String r;
      serializeJson(doc, r);
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
      StaticJsonDocument<200> doc;
      doc["FREQ"] = delayMS;
      String r;
      serializeJson(doc, r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for getting Sensor Frequency--------*/
/*-------Action command for fetching ESP Chip ID------------*/
    if(comp(action,"ESPID"))
    {
      StaticJsonDocument<200> doc;
      doc["CHIPID"] = chipid;
      String r;
      serializeJson(doc, r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for fetching ESP Chip ID------------*/

/*-------Action command for resetting ESP-------------------*/
    if(comp(action,"RESET"))
    {
      StaticJsonDocument<200> doc;
      doc["action"] = "ResetStatus";
      doc["stat"] = "Resetting Device...";
      String r;
      serializeJson(doc, r);
      sendToMQTT(outtopic, r);
      ESP.reset();
    }
/*-------Action command for resetting ESP-------------------*/

/*-------Action command for controlling Relays--------------*/
    else if(comp(action,"RELAY"))
    {
      int no = root["no"];
      bool value = root["value"];
      String by = root["by"];
      relay_action(no, value, by);
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
      StaticJsonDocument<200> doc;
      doc["action"] = "Device Version";
      doc["value"] = DEVICE_V;
      String r;
      serializeJson(doc, r);
      sendToMQTT(outtopic, r);
    }
/*-------Action command for getting getting device version--*/
/*-----Action command for getting getting firmware version--*/
    else if(comp(action,"GET_VERSION"))
    {
      StaticJsonDocument<200> doc;
      doc["action"] = "Firmware Version";
      doc["value"] = FIRMWARE_V;
      String r;
      serializeJson(doc, r);
      sendToMQTT(outtopic, r);
    }
/*-----Action command for getting getting firmware version--*/
/*-------Action command for updating firmware---------------*/
    else if(comp(action,"UPDATE"))
    {
      const char * c = root["url"];
      updateAddress = c;
      callback = &updateESP;
    }
/*-------Action command for updating firmware---------------*/
  }
}
/*-------Meathod called on reciving message from MQTT-------*/
void relay_action(int no, bool value, String by)
{
  sr.set(no, value);
//sending status
  send_status();  
//sending nortification
  StaticJsonDocument<200> doc;
  if(by != "")
  {
    doc["by"] = by;
    doc["no"] = no;
    doc["CHIPID"] = chipid;
    doc["input"] = value;
  }
  String r = "";
  serializeJson(doc, r);
  sendToMQTT(norttopic, r);

//saving to eeprom
  for(int t=0; t<8; t++)
  {
    conf.pinValues[t] = sr.get(t);
  }
  EEPROM.put(0, conf);
  EEPROM.commit();
}
/*-------Meathod to update ESP------------------------------*/
void updateESP()
{
  StaticJsonDocument<200> doc;
  doc["action"] = "UpdateStatus";
  doc["stat"] = "Updating Device...";
  String r;
  serializeJson(doc, r);
  sendToMQTT(outtopic, r);
  delay(100);
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateAddress.c_str());
  serialDisplay("Update Address",updateAddress);
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
  StaticJsonDocument<200> doc_1;
  doc_1["action"] = "UpdateStatus";
  doc_1["stat"] = stat;
  r = "";
  serializeJson(doc_1, r);
  sendToMQTT(outtopic, r);
  delay(100);
  serialDisplay("Update",stat);
  callback = &blank;
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
  EEPROM.wipe();
  conf.setupFlag = true;
  EEPROM.put(0, conf);
  EEPROM.commit();
  ESP.reset();
}
/*----Meathod for reconfiguring WiFi settings---------------*/
/*----Meathod for sending relay status----------------------*/
void send_status()
{
  StaticJsonDocument<200> doc;
  doc["d"] = chipid;
  doc["action"] = "s";
  JsonArray data = doc.createNestedArray("v");
  for(int t=0; t<8; t++)
  {
    data.add(sr.get(t));
  }
  String r;
  serializeJson(doc, r);
  sendToMQTT(outtopic, r);
  sendToMQTT(espstatus, r);
  send_status_uart();
}
/*----Meathod for sending relay status----------------------*/
/*----Meathod for sending relay status on UART--------------*/
void send_status_uart()
{
  StaticJsonDocument<200> doc;
  doc["a"] = "RS";
  JsonArray data = doc.createNestedArray("n");
  for(int t=0; t<8; t++)
  {
    data.add(sr.get(t));
  }
  serializeJson(doc, Serial);
  Serial.println();
}
/*----Meathod for sending relay status on UART--------------*/
/*----Meathod for sending MQTT Data-------------------------*/
void sendToMQTT(String topic, String msg)
{
  mqtt.publish(topic.c_str(), 2, false, msg.c_str(), msg.length());
}
/*----Meathod for sending MQTT Data-------------------------*/
/*---Meathod for pinging MQTT Server for active connection--*/
void pinging()
{
  StaticJsonDocument<200> doc;
  doc["d"] = chipid;
  if(strcmp(DEVICE_V, "v2") == 0)
    doc["s"] = true;
  else
    doc["s"] = false;
  doc["i"] = IpAddress;
  doc["l"] = LocalIP;
  String r;
  serializeJson(doc, r);
  sendToMQTT(espraw, r);
}
/*---Meathod for pinging MQTT Server for active connection--*/
/*-----Meathod for sending sensor data----------------------*/
void sendSensorData()
{
  StaticJsonDocument<200> doc;
  doc["d"] = chipid;
  doc["t"] = temp;
  doc["h"] = humid;
  doc["l"] = light;
  String s;
  serializeJson(doc, s);
  sendToMQTT(sensortopic, s);
}
/*-----Meathod for sending sensor data----------------------*/
/*-----Meathod for checking reset button--------------------*/
void checkReset()
{
  if(digitalRead(reset_btn) == HIGH)
  {
    reset();
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
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient httpAPI;
    httpAPI.begin("http://api.ipify.org/?format=json");
    int HttpCode = httpAPI.GET();
    if(HttpCode > 0)
    {  
      String payload = httpAPI.getString();
      httpAPI.end();
      StaticJsonDocument<200> doc;
      deserializeJson(doc, payload);
      IpAddress = "";
      const char* s = doc["ip"];
      IpAddress = s;
      Wifi_ssid = WiFi.SSID();
      LocalIP = IpAddress2String(WiFi.localIP());
      serialDisplay("SSID",Wifi_ssid);
      serialDisplay("IP Address",IpAddress);
    }
  }
}
/*-----Meathod for fetching IP Address----------------------*/
/*-----Meathod to convert IP Address to String -------------*/
String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}
/*-----Meathod to convert IP Address to String -------------*/
/*-----Blank function-----------------------------------------*/
void blank()
{
  
}
/*-----Blank function-----------------------------------------*/
void loop() 
{
  MDNS.update();
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
      light = map(analogRead(LDR_PIN), 0, 255, 0, 100);
    }
  }
}