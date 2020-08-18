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
    #include "device_handler.h"
    AsyncMqttClient mqtt;                     //Variable to initiate MQTT.
    DHT_Unified dht(2, DHT11);         //Initializing DHT sensor. 
    ShiftRegister74HC595<1> sr (16, 14, 12);  //Setting up shift register.
    uint8_t LDR_PIN = 0;
    byte indicator_led = 13;
    byte reset_btn = 4;
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
    void configure_gpio()
    {
        String config = read_device_config();
        StaticJsonDocument<500> doc;
        DeserializationError error = deserializeJson(doc, config);
        if(error)
        {
            return;
        }
        bool shift_reg_avail = doc["device_config"]["shift_out_reg"]["avail"];
        if(shift_reg_avail)
        {
            byte clock_pin = doc["device_config"]["shift_out_reg"]["clockPin"];
            byte data_pin = doc["device_config"]["shift_out_reg"]["serialDataPin"];
            byte latch_pin = doc["device_config"]["shift_out_reg"]["latchPin"];
            ShiftRegister74HC595<1> shift_reg(data_pin,clock_pin,latch_pin);
            sr = shift_reg;
        }
        byte relay_count = doc["device_config"]["relay"]["count"];
        if(relay_count > 0)
        {
            JsonArray array = doc["device_config"]["relay"]["GPIO"].as<JsonArray>();
            for(JsonVariant v : array) {
                byte pin = v.as<int>();
                pinMode(pin, OUTPUT);
            }
        }
        byte status_led = doc["device_config"]["status_led"]["led_pin"];
        pinMode(status_led, OUTPUT);
        indicator_led = status_led;
        byte reset_pin = doc["device_config"]["reset_btn"];
        pinMode(reset_pin, INPUT);
        reset_btn = reset_pin;
        bool hasDHT = doc["device_config"]["dht"]["INSTALLED"];
        if(hasDHT)
        {
            byte dht_pin = doc["device_config"]["dht"]["GPIO"];
            const char* DHTSensorType = doc["device_config"]["dht"]["TYPE"];
            byte DHTType;
            if(comp(DHTSensorType, "DHT11"))
            {
                DHTType = DHT11;
            }
            if(comp(DHTSensorType,"DHT22"))
            {
                DHTType = DHT22;
            }
            DHT_Unified dht_var(dht_pin, DHTType);         //Initializing DHT sensor. 
            dht = dht_var;
        }
        bool hasLight = doc["device_config"]["light"]["INSTALLED"];
        if(hasLight)
        {
            const char* lumin_pin = doc["device_config"]["light"]["INSTALLED"];
            if(comp(lumin_pin, "A0"))
                LDR_PIN = 0;
        }
    }
#endif