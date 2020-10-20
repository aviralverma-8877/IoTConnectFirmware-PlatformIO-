#include "web_sockets_handler.h"

void send_data_to_webSocket(String options)
{
    callback = &send_websocket;
    websocket_msg = options;
}

void send_websocket()
{
    webSocket.broadcastTXT(websocket_msg);
    callback = &blank;
}
 