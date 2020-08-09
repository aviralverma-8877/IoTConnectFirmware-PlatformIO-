#ifndef global_var_one
    #define global_var_one
    #include<Arduino.h>
    #include <AsyncMqttClient.h>              //Async MQTT Library
    #include <Ticker.h>                       //Ticker for running multithread
    #include <ArduinoJson.h>                  //Encoading and Decoding JSON
    #include <ESP8266httpUpdate.h>            //ESP Update Library.
    #include <DHT.h>                          //For DHT Temperature and Humidity Sensor
    #include <DHT_U.h>                        //For DHT Temperature and Humidity Sensor
    #include <ShiftRegister74HC595.h>         //For controlling Relays from 74HC595 shift register
    #include <DNSServer.h>                    //For redirecting the user on connecting to device WiFi
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include "structures.h"
    #ifndef DHTPIN
        #define DHTPIN 2                          //DHT single wire interface pin
    #endif
    #ifndef DHTTYPE
        #define DHTTYPE DHT11                     //Type of DHT sensor.
    #endif
    AsyncMqttClient mqtt;                     //Variable to initiate MQTT.

    DHT_Unified dht(DHTPIN, DHTTYPE);         //Initializing DHT sensor. 


    ShiftRegister74HC595<1> sr (16, 14, 12);  //Setting up shift register.
    sensor_t sensor;                          //DTH sensor
    sensors_event_t event;                    //Creating event variable for DHT sensor.
    /*----------------------------------------------------------*/
    /*----------------------------------------------------------*/
    String payload;                           //Global variables
    String data;                              //Global variables
    String Wifi_ssid;                         //Global variables
    uint8_t i;                                //Global variables
    HTTPClient http;                          //Global variables
    int temp, humid, light;                   //Global variables
    uint32_t delayMS;                         //Global variables
    String updateAddress;                     //Update address
    DNSServer dnsServer;                      //Global variables
    AsyncWebServer webServer(80);             //Global variables
    /*----------------------------------------------------------*/
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

#endif