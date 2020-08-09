#ifndef structures
  #define structures 1
  #include <Arduino.h>

  struct configuration{
    uint8 pinValues[8];  //Default relay values.
    bool setupFlag;
    bool updateFlag;
    bool led_enabled;
    int pingTime;
    String http_username;
    String http_password;
  };
#endif