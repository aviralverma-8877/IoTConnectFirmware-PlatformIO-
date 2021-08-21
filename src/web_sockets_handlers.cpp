#include "web_sockets_handler.h"

void send_data_to_webSocket(String options)
{
    webSocket.broadcastTXT(options);
    serialDisplay("Websocket Data", websocket_msg);
}

void send_websocket()
{
    webSocket.broadcastTXT(websocket_msg);
    callback = &blank;
}
 