#include "fauxmo_handler.h"

void setup_fauxmo()
{
    fauxmo.createServer(false);
    fauxmo.setPort(80);
    fauxmo.enable(true);
    fauxmo_remove_all_device();
    fauxmo_add_device();
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        relay_action(device_name, state, "");
    });
}

void fauxmo_add_device(const char* device_name)
{
    if(!comp(device_name,"N/A"))
    {
        serialDisplay("Fauxmo Device Added",device_name);
        fauxmo.addDevice(device_name);
    }
}

void fauxmo_remove_device(const char* device_name)
{
    if(!comp(device_name,"N/A"))
    {
        serialDisplay("Fauxmo Device Removed",device_name);
        fauxmo.removeDevice(device_name);
    }
}

void fauxmo_remove_all_device()
{
    String mqtt_data = read_mqtt_config();
    DynamicJsonDocument doc(1000);
    StaticJsonDocument<200> filter;
    filter["relay"][0]["name"] = true;
    DeserializationError error = deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));
    if(error)
    return;
    for( JsonObject kv : doc["relay"].as<JsonArray>())
    {
        const char* device_name = kv["name"];
        fauxmo_remove_device(device_name);
    }
}

void fauxmo_add_device()
{
    read_config();
    fauxmo_add_device(conf.fauxmo_relay_1.c_str());
    fauxmo_add_device(conf.fauxmo_relay_2.c_str());
    fauxmo_add_device(conf.fauxmo_relay_3.c_str());
}