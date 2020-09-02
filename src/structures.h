#ifndef structures
  #define structures
  #include <Arduino.h>

  struct configuration{
    bool setupFlag;
    bool updateFlag;
    bool led_enabled;
    bool save_eeprom;
    int pingTime;
    String http_username;
    String http_password;
    String WiFi_SSID;
    String WiFi_PASS;
    bool wifi_setup_done;
  };
#endif