#include "global_var_two.h"
bool reconnect_mqtt = true;
bool reset_btn_status = false;
byte reset_btn_press_count = 0;
unsigned long reset_btn_press_time = 0;
bool debugging = false;                   //Turn On or Off the serial output.
String websocket_msg = "";
/*--------------MQTT Configration---------------------------*/
bool MQTTStatus = false;
String chipid = getMacAddress();  //Fetching ESP device ID.
String outtopic = chipid+"/RESPONSE";          //MQTT Topic for sending data from ESP.
String intopic = chipid+"/COMMAND";
String espsensor = "ESP_SENSOR";           //MQTT Topic for sending sensor status data.
/*--------------MQTT Configration---------------------------*/
String getMacAddress() {
    uint8_t baseMac[6];
    // Get MAC address for WiFi station
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return String(baseMacChr);
}