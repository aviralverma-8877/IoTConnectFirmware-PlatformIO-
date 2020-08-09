#ifndef web_sockets_handler
    #define web_sockets_handlers
    #include <Arduino.h>
    #include <WebSocketsServer.h>

    extern WebSocketsServer webSocket;
    extern String webSocket_message;
    void send_data_to_webSocket(String options);
#endif