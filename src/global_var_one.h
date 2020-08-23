#ifndef global_var_one
    #define global_var_one
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
    //Configuring Device
    #ifndef FIRMWARE_V
    #define FIRMWARE_V "2.1.2"                //Current firmware version. (Displayed on Device Portal)
    #endif
    #ifndef DEVICE_V
        #define DEVICE_V   "v1"                   //Device type version (V1 - Without Sensor)
                                                                //(V2 - With Sensor)
                                            //Should not modify the vesions, as website device portal is set accordingly.
    #endif


    extern configuration conf;
    extern AsyncMqttClient mqtt;                     //Variable to initiate MQTT.
    extern DHT_Unified dht;         //Initializing DHT sensor. 
    extern uint8_t LDR_PIN;
    extern byte indicator_led;
    extern byte reset_btn;
    extern ShiftRegister74HC595<1> sr;  //Setting up shift register.
    extern sensor_t sensor;                          //DTH sensor
    extern sensors_event_t event;                    //Creating event variable for DHT sensor.
    /*----------------------------------------------------------*/
    /*----------------------------------------------------------*/
    extern String payload;                           //Global variables
    extern String IpAddress;                    //Global variables
    extern String LocalIP;                      //Global variables
    extern String data;                              //Global variables
    extern String Wifi_ssid;                         //Global variables
    extern bool mqtt_setup;                  //Global variables
    extern bool first_connect;               //Global variables
    extern bool inSetup;
    extern byte loopCount;                       //Global variables
    extern uint8_t attempts;                     //Global variables
    extern uint8_t i;                                //Global variables
    extern HTTPClient http;                          //Global variables
    extern int temp, humid, light;                   //Global variables
    extern uint32_t delayMS;                         //Global variables
    extern String updateAddress;                     //Update address
    extern DNSServer dnsServer;                      //Global variables
    extern AsyncWebServer webServer;             //Global variables
    extern bool shouldReboot;
    extern String wifi_ssid;
    extern String wifi_pass;
    /*----------------------------------------------------------*/
    /*--------------Tickers for Async Meathods------------------*/
    extern Ticker TickerForPinging;
    extern Ticker TickerForsendSensorData;
    extern Ticker TickerForcheckReset;
    extern Ticker TickerForfetchIP;
    extern Ticker TickerForconnectToMqtt;
    extern Ticker TickerForFeedbackLED;
    extern Ticker TickerForSerialListner;
    extern Ticker TickerForUARTUpdater;
    extern Ticker TickerForTimeOut;
    /*--------------Tickers for Async Meathods------------------*/
     void configure_gpio();
#endif