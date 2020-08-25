#include <Arduino.h>
#include"global_var_two.h"

void send_data_to_webSocket(String options)
{
    webSocket.broadcastTXT(options);
}

 