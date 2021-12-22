#ifndef bluetooth_control
    #define bluetooth_control
    #include <Arduino.h>
    #include "structures.h"
    #include <Arduino.h>
    #include <ArduinoJson.h>
    #include "BluetoothSerial.h"
    #include "FS.h"
    #include "SPIFFS.h"
    #include "mqtt_handler.h"
    #include "global_var_two.h"
    #include "global_var_one.h"
    void bluetooth_init();
    void send_to_bluetooth_serial(String msg);
#endif