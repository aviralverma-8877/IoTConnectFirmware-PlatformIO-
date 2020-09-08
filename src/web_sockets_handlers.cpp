#include "web_sockets_handler.h"

void send_data_to_webSocket(String options)
{
    webSocket.broadcastTXT(options);
}

 