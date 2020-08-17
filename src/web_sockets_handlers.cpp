#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

void send_data_to_webSocket(String options)
{
    webSocket.broadcastTXT(options);
}

 