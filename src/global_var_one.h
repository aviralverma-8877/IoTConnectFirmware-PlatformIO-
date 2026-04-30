#ifndef global_var_one
#define global_var_one

#include <Arduino.h>
#include <AsyncMqttClient.h>      //Async MQTT Library
#include <Ticker.h>               //Ticker for running multithread
#include <ArduinoJson.h>          //Encoading and Decoding JSON
#include <ESP8266httpUpdate.h>    //ESP Update Library.
#include <ShiftRegister74HC595.h> //For controlling Relays from 74HC595 shift register
#include <DNSServer.h>            //For redirecting the user on connecting to device WiFi
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <WiFiClient.h>
#include "fauxmoESP.h"
#include "structures.h"
#include "device_handler.h"
// Configuring Device
#ifndef FIRMWARE_V
#define FIRMWARE_V "3.0.0" // Current firmware version. (Displayed on Device Portal)
#endif

// Timing Constants (in milliseconds)
#define WIFI_RECONNECT_INTERVAL_SEC 5       // WiFi/MQTT reconnection check interval (seconds)
#define LED_BLINK_INTERVAL_SEC 0.6          // LED blink rate during connection attempts (seconds)
#define RESET_CHECK_INTERVAL_MS 10          // Reset button check interval (milliseconds)
#define DEFAULT_PING_INTERVAL_MS 2000       // Default device ping/heartbeat interval
#define CONFIG_SAVE_TIMEOUT_SEC 1           // Timeout before reboot after config save (seconds)

// ADC Constants
#define ESP8266_ADC_MAX 1023                // ESP8266 ADC maximum value (10-bit)
#define ILLUMINANCE_PERCENT_MAX 100         // Maximum percentage for light sensor mapping
/*----------------------------------------------------------*/
/*----------------------------------------------------------*/
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
extern AsyncMqttClient mqtt; // Variable to initiate MQTT.
extern uint8_t dht_pin;
extern String DHTType;
extern DHT11 sensor_dht11;
extern DHT22 sensor_dht22;
extern uint8_t LDR_PIN;
extern byte indicator_led;
extern byte reset_btn;
extern fauxmoESP fauxmo;
extern ShiftRegister74HC595<1> sr; // Setting up shift register.
/*----------------------------------------------------------*/
/*----------------------------------------------------------*/
extern String payload;           // Global variables
extern String IpAddress;         // Global variables
extern String LocalIP;           // Global variables
extern String data;              // Global variables
extern String Wifi_ssid;         // Global variables
extern String WiFi_gateway;      // Global variables
extern byte loopCount;           // Global variables
extern uint8_t i;                // Global variables
extern HTTPClient http;          // Global variables
extern uint32_t delayMS;         // Global variables
extern DNSServer dnsServer;      // Global variables
extern AsyncWebSocket webSocket; // Global variables
extern AsyncWebServer server;    // Global variables
extern int illuminance_value;    // Global variables
extern bool shouldReboot;        // Global variables
extern String wifi_ssid;         // Global variables
extern String wifi_pass;         // Global variables
extern IPAddress apIP;           // Global variables
/*----------------------------------------------------------*/
/*--------------Tickers for Async Methods------------------*/
extern Ticker TickerForPinging;
extern Ticker TickerForsendSensorData;
extern Ticker TickerForcheckReset;
extern Ticker TickerForconnectToMqtt;
extern Ticker TickerForFeedbackLED;
extern Ticker TickerForTimeOut;
extern Ticker TickerForTimeOutTwo;
extern Ticker TickerForWebSocketStatus;
/*--------------Tickers for Async Methods------------------*/
void configure_gpio();
#endif