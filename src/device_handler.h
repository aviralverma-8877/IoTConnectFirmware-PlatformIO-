#ifndef device_handler
    #define device_handler
    #include<Arduino.h>
    #include<ArduinoJson.h>
    String read_mqtt_config();  
    void relay_action(String relay, bool value, String by);
    void updateESP();
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
    void manage_dns_request();
    void generate_mqtt_topics();
    String read_device_config();
    void perform_action();
    void enable_ap();
    void disable_ap();
#endif