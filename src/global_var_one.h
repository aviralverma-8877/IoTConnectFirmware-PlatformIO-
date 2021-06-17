#ifndef global_var_one
    #define global_var_one
    
    #include <Arduino.h>
    #include <AsyncMqttClient.h>              //Async MQTT Library
    #include <Ticker.h>                       //Ticker for running multithread
    #include <ArduinoJson.h>                  //Encoading and Decoding JSON
    #include <ESP8266httpUpdate.h>            //ESP Update Library.
    #include <ShiftRegister74HC595.h>         //For controlling Relays from 74HC595 shift register
    #include <DNSServer.h>            //For redirecting the user on connecting to device WiFi
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include <DHT.h>
    #include <WiFiClient.h>
    #include "fauxmoESP.h"
    #include "structures.h"
    #include "device_handler.h"
    #ifndef MQTT_HOST
        #define MQTT_HOST "mqtt.iot-connect.in"        //MQTT Server address
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
    #define FIRMWARE_V "2.1.9"                //Current firmware version. (Displayed on Device Portal)
    #endif
    extern WiFiClient wifiClient;
    extern HTTPClient httpAPI;
    extern int MQTT_QoS;
    extern bool hasSensor; 
    extern bool hasDHTSensor;
    extern bool hasLightSensor;
    extern bool def_led_value;
    extern bool def_btn_value;
    extern bool ap_enabled;
    extern configuration conf;
    extern AsyncMqttClient mqtt;                     //Variable to initiate MQTT.
    extern uint8_t dht_pin;
    extern String DHTType;
    extern DHT11 sensor_dht11;
    extern DHT22 sensor_dht22;
    extern uint8_t LDR_PIN;
    extern byte indicator_led;
    extern byte reset_btn;
    extern fauxmoESP fauxmo;
    extern ShiftRegister74HC595<1> sr;  //Setting up shift register.
    /*----------------------------------------------------------*/
    /*----------------------------------------------------------*/
    extern String payload;                    //Global variables
    extern String IpAddress;                  //Global variables
    extern String LocalIP;                    //Global variables
    extern String data;                       //Global variables
    extern String Wifi_ssid;                  //Global variables
    extern String WiFi_gateway;               //Global variables
    extern byte loopCount;                    //Global variables
    extern uint8_t attempts;                  //Global variables
    extern uint8_t i;                         //Global variables
    extern HTTPClient http;                   //Global variables
    extern uint32_t delayMS;                  //Global variables
    extern String updateAddress;              //Global variables
    extern DNSServer dnsServer;               //Global variables
    extern AsyncWebServer server;             //Global variables
    extern int illuminance_value;             //Global variables
    extern bool shouldReboot;                 //Global variables
    extern String wifi_ssid;                  //Global variables
    extern String wifi_pass;                  //Global variables
    extern const byte DNS_PORT;               //Global variables
    extern IPAddress apIP;                    //Global variables
    /*----------------------------------------------------------*/
    /*--------------Tickers for Async Meathods------------------*/
    extern Ticker TickerForPinging;
    extern Ticker TickerForsendSensorData;
    extern Ticker TickerForcheckReset;
    extern Ticker TickerForconnectToMqtt;
    extern Ticker TickerForFeedbackLED;
    extern Ticker TickerForSerialListner;
    extern Ticker TickerForUARTUpdater;
    extern Ticker TickerForTimeOut;
    extern Ticker TickerForTimeOutTwo;
    extern Ticker TickerForWiFiConnect;
    extern Ticker TickerForWebSocketStatus;
    /*--------------Tickers for Async Meathods------------------*/
     void configure_gpio();
#endif