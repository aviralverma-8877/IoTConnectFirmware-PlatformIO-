#include "fauxmo_handler.h"
void setup_fauxmo()
{
    fauxmo.createServer(false);
    fauxmo.setPort(80);
    fauxmo.enable(true);
    fauxmo_remove_all_device();
    fauxmo_add_device();
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        serialDisplay("setup_fauxmo","Fauxmo called "+String(device_name));
        DynamicJsonDocument doc(1500);
        String mqtt_data = read_mqtt_config();
        DeserializationError error = deserializeJson(doc, mqtt_data);
        if(error)
        {
            return;
        }
        doc.shrinkToFit();
        JsonArray array = doc["relay"].as<JsonArray>();
        for (JsonObject ele : array) {
            String topic = ele["topic"];
            if(comp(topic.c_str(), device_name))
            {
                relay_action(ele["name"], state, "");
                break;
            }
        }
    });
}

void fauxmo_add_device(const char* device_name)
{
    if(!comp(device_name,"N/A"))
    {
        fauxmo.addDevice(device_name);
    }
}

void fauxmo_remove_device(const char* device_name)
{
    if(!comp(device_name,"N/A"))
    {
        fauxmo.removeDevice(device_name);
    }
}

void fauxmo_remove_all_device()
{
    String mqtt_data = read_mqtt_config();
    StaticJsonDocument<200> doc;
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