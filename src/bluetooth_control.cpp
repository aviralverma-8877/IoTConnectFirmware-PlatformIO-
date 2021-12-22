#include "bluetooth_control.h"

void bluetooth_init()
{
    serialDisplay("bluetooth_init","Enabling Classic Bluetooth");
    SerialBT.begin("ESP32test");
}

void send_to_bluetooth_serial(String msg)
{   
    serialDisplay("send_to_bluetooth_serial","BL Send: "+msg);
    SerialBT.printf(msg.c_str());
}