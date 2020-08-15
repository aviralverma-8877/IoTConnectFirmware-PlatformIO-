#ifndef device_handler
    #define device_handler
    #include<Arduino.h>
    #include<ArduinoJson.h>
    void relay_action(int no, bool value, String by);
    void updateESP();
    void SerialListner();
    void switch_wifi();
    void feedbackLED();
    void reset();
    void pinging();
    void sendSensorData();
    void checkReset();
    bool comp(const char *val1,const char *val2);
    void fetchIP();
    void read_config();
    void send_status_uart();
    String read_device_config();
#endif