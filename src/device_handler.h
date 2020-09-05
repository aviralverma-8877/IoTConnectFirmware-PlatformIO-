#ifndef device_handler
    #define device_handler
    #include<Arduino.h>
    #include<ArduinoJson.h>

    void setup_tickers();
    String device_status();
    String read_mqtt_config();  
    void relay_action(String relay, bool value, String by);
    void updateESPSpiffs();
    void switch_wifi();
    void feedbackLED();
    void reset();
    void pinging();
    void setup_sensor();
    void sendSensorData();
    void checkReset();
    bool comp(const char *val1,const char *val2);
    void fetchIP();
    void read_config();
    void send_status_uart();  
    void manage_dns_request();
    void generate_mqtt_topics();
    String read_device_config();
    void toggle_relay(String relay);
    void perform_action();
    void perform_action(String relay, bool value);
    void enable_ap();
    void disable_ap();
#endif