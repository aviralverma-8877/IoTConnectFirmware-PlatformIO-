#include "fauxmo_handler.h"

void setup_fauxmo()
{
    fauxmo.createServer(false);
    fauxmo.setPort(80);
    fauxmo.enable(true);
    fauxmo_add_all_device();
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        relay_action(device_name, state, "");
    });
}

void fauxmo_add_device(const char* device_name)
{
    fauxmo.addDevice(device_name);
}

void fauxmo_remove_device(const char* device_name)
{
    fauxmo.removeDevice(device_name);
}

void fauxmo_add_all_device()
{
    String mqtt_data = read_mqtt_config();
    DynamicJsonDocument doc(1000);
    StaticJsonDocument<200> filter;
    filter["relay"][0]["name"] = true;
    DeserializationError error = deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));
    if(error)
    return;
    int count = 0;
    for( JsonObject kv : doc["relay"].as<JsonArray>())
    {
        const char* device_name = kv["name"];
        serialDisplay("Fauxmo","Device Added "+String(device_name));
        fauxmo_add_device(device_name);
        count++;
        if(count >= 3)
            break;
    }
}