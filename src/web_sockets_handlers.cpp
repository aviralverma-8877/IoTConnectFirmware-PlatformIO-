#include "web_sockets_handler.h"

void send_data_to_webSocket(String msg)
{
  webSocket.textAll(msg);
}

void initWebSocket() {
  serialDisplay("initWebSocket","Initilizing Websockets");
  webSocket.onEvent(onEvent);
  server.addHandler(&webSocket);
  server.addHandler(&events);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        serialDisplay("onEvent","WebSocket client Connected");
        break;
      case WS_EVT_DISCONNECT:
        serialDisplay("onEvent","WebSocket client Disconnected");
        break;
      case WS_EVT_DATA:
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}