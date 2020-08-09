#ifndef global_var_one
    #define global_var_one 1
    #include <Arduino.h>
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

    #ifndef MQTT_HOST
        #define MQTT_HOST "iot-connect.in"        //MQTT Server address
    #endif
    #ifndef MQTT_PORT
        #define MQTT_PORT 1883                    //MQTT Server port
    #endif
    //MQTT Cred
    #ifndef MQTT_UNAME
        #define MQTT_UNAME "iotconnect"
    #endif
    #ifndef MQTT_PASS
        #define MQTT_PASS "iot-12345"
    #endif
    #ifndef LDR_PIN
        #define LDR_PIN A0                        //LDR Pin address
    #endif
    #ifndef reset_btn
        #define reset_btn 4                       //Reset Button pin
    #endif
    #ifndef indicator_led
        #define indicator_led 13                  //LED Pin
    #endif
    #ifndef DHTPIN
        #define DHTPIN 2                          //DHT single wire interface pin
    #endif
    #ifndef DHTTYPE
        #define DHTTYPE DHT11                     //Type of DHT sensor.
    #endif
    //Configuring Device
    #ifndef FIRMWARE_V
    #define FIRMWARE_V "2.1.1"                //Current firmware version. (Displayed on Device Portal)
    #endif
    #ifndef DEVICE_V
        #define DEVICE_V   "v1"                   //Device type version (V1 - Without Sensor)
                                                                //(V2 - With Sensor)
                                            //Should not modify the vesions, as website device portal is set accordingly.
    #endif


    configuration conf;

    AsyncMqttClient mqtt;                     //Variable to initiate MQTT.

    DHT_Unified dht(DHTPIN, DHTTYPE);         //Initializing DHT sensor. 


    ShiftRegister74HC595<1> sr (16, 14, 12);  //Setting up shift register.
    sensor_t sensor;                          //DTH sensor
    sensors_event_t event;                    //Creating event variable for DHT sensor.
    /*----------------------------------------------------------*/
    /*----------------------------------------------------------*/
    String payload;                           //Global variables
    String IpAddress;                    //Global variables
    String LocalIP;                      //Global variables
    String data;                              //Global variables
    String Wifi_ssid;                         //Global variables
    bool mqtt_setup;                  //Global variables
    bool first_connect;               //Global variables
    bool inSetup;
    byte loopCount;                       //Global variables
    uint8_t attempts;                     //Global variables
    uint8_t i;                                //Global variables
    HTTPClient http;                          //Global variables
    int temp, humid, light;                   //Global variables
    uint32_t delayMS;                         //Global variables
    String updateAddress;                     //Update address
    DNSServer dnsServer;                      //Global variables
    AsyncWebServer webServer(80);             //Global variables
    bool shouldReboot;
    String wifi_ssid;
    String wifi_pass;
    /*----------------------------------------------------------*/
    /*--------------MQTT Configration---------------------------*/
    String norttopic;                //MQTT Topic for sending nortifications.
    String sensortopic;         //MQTT Topic for sending sensor data.
    String espstatus;           //MQTT Topic for sending relay status data.
    String espraw;                 //MQTT Topic for sending device attendence.
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