#include "global_var_one.h"

WiFiClient wifiClient;
HTTPClient httpAPI;
AsyncMqttClient mqtt;                     //Variable to initiate MQTT. 
ShiftRegister74HC595<1> sr (16, 14, 12);  //Setting up shift register.
uint8_t LDR_PIN = 0;
uint8_t dht_pin;
String DHTType;
DHT11 sensor_dht11;
DHT22 sensor_dht22;
byte indicator_led = 13;
byte reset_btn = 4;
bool hasSensor = false;
bool hasDHTSensor = false;
bool hasLightSensor = false;
bool def_led_value = HIGH;
bool def_btn_value = HIGH;
bool ap_enabled = true;
int MQTT_QoS = 2;
int illuminance_value = 0;
fauxmoESP fauxmo;
/*----------------------------------------------------------*/
/*----------------------------------------------------------*/
String payload;                           //Global variables
String data;                              //Global variables
String Wifi_ssid;                         //Global variables
String WiFi_gateway;                      //Global variables
uint8_t i;                                //Global variables
HTTPClient http;                          //Global variables
uint32_t delayMS;                         //Global variables
DNSServer dnsServer;                      //Global variables
AsyncWebSocket webSocket("/ws");          //Global variables
AsyncWebServer server(80);                //Global variables
/*----------------------------------------------------------*/
configuration conf = {false,false,true,false,2000,"N/A","admin","admin","","",false,"{}"};
String IpAddress = "";                    //Global variables
String LocalIP = "";                      //Global variables
byte loopCount = 0;                       //Global variables
bool shouldReboot = false;
bool initSetupDone = false;
IPAddress apIP(192, 168, 4, 1);
/*----------------------------------------------------------*/
/*--------------Tickers for Async Methods------------------*/
Ticker TickerForPinging;
Ticker TickerForsendSensorData;
Ticker TickerForcheckReset;
Ticker TickerForconnectToMqtt;
Ticker TickerForFeedbackLED;
Ticker TickerForTimeOut;
Ticker TickerForTimeOutTwo;
Ticker TickerForWebSocketStatus;
/*--------------Tickers for Async Methods------------------*/
void configure_gpio()
{
    serialDisplay("configure_gpio","Configuring GPIO");
    String config = read_device_config();
    JsonDocument doc;
    deserializeJson(doc, config);

    doc.shrinkToFit();
    bool shift_reg_avail = doc["device_config"]["shift_out_reg"]["avail"];
    if(shift_reg_avail)
    {
        byte clock_pin = doc["device_config"]["shift_out_reg"]["clockPin"];
        byte data_pin = doc["device_config"]["shift_out_reg"]["serialDataPin"];
        byte latch_pin = doc["device_config"]["shift_out_reg"]["latchPin"];
        sr = ShiftRegister74HC595<1>(data_pin,clock_pin,latch_pin);
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
    bool led_value = doc["device_config"]["status_led"]["def"];
    if(led_value)
    {
        def_led_value = HIGH;
    }
    else
    {
        def_led_value = LOW;
    }
    pinMode(status_led, OUTPUT);
    indicator_led = status_led;
    byte reset_pin = doc["device_config"]["reset_btn"]["btn_pin"];
    bool btn_value = doc["device_config"]["reset_btn"]["def"];
    if(btn_value)
    {
        def_btn_value = HIGH;
    }
    else
    {
        def_btn_value = LOW;
    }
    pinMode(reset_pin, INPUT);
    reset_btn = reset_pin;
    bool hasDHT = doc["device_config"]["dht"]["INSTALLED"];
    if(hasDHT)
    {
        hasSensor = true;
        hasDHTSensor = true;
        dht_pin = doc["device_config"]["dht"]["GPIO"];
        const char* DHTSensorType = doc["device_config"]["dht"]["TYPE"];
        DHTType = DHTSensorType;
    }
    bool hasLight = doc["device_config"]["light"]["INSTALLED"];
    if(hasLight)
    {
        hasSensor = true;
        hasLightSensor = true;
        const char* lumin_pin = doc["device_config"]["light"]["GPIO"];
        if(comp(lumin_pin, "A0"))
            LDR_PIN = 0;
    }
}
