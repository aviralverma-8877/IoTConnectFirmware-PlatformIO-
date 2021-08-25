#include "web_sockets_handler.h"

void send_data_to_webSocket(String msg)
{
  if(ESP.getFreeHeap()>8000)
    webSocket.textAll(msg);
}

void initWebSocket() {
  webSocket.onEvent(onEvent);
  server.addHandler(&webSocket);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        serialDisplay("WebSocket client","Connected");
        send_status();
        break;
      case WS_EVT_DISCONNECT:
        serialDisplay("WebSocket client","Disconnected");
        break;
      case WS_EVT_DATA:
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}