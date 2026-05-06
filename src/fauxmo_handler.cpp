#include "fauxmo_handler.h"
void setup_fauxmo()
{
    fauxmo_remove_all_device();
    fauxmo_add_device();
    fauxmo.createServer(false);
    fauxmo.setPort(80);
    fauxmo.enable(true);
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        serialDisplay("setup_fauxmo","Fauxmo called "+String(device_name));
        JsonDocument doc;
        String mqtt_data = read_mqtt_config();
        deserializeJson(doc, mqtt_data);

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

    fauxmo.onGetState([](unsigned char device_id, const char * device_name, bool& state, unsigned char& value) {
        String mqtt_data = read_mqtt_config();
        JsonDocument doc;
        JsonDocument filter;
        filter["relay"][0]["topic"] = true;
        filter["relay"][0]["comp"]  = true;
        filter["relay"][0]["pin"]   = true;
        filter["relay"][0]["status"] = true;
        deserializeJson(doc, mqtt_data, DeserializationOption::Filter(filter));
        doc.shrinkToFit();

        for (JsonObject ele : doc["relay"].as<JsonArray>()) {
            String topic = ele["topic"];
            if (comp(topic.c_str(), device_name)) {
                String comp_type = ele["comp"];
                int pin = ele["pin"];
                if (comp(comp_type.c_str(), "shift_reg"))
                    state = sr.get(pin);
                else if (comp(comp_type.c_str(), "gpio"))
                    state = digitalRead(pin);
                else
                    state = ele["status"];
                value = state ? 255 : 0;
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
    JsonDocument doc;
    JsonDocument filter;
    filter["relay"][0]["name"] = true;
    deserializeJson(doc, mqtt_data,DeserializationOption::Filter(filter));

    for( JsonObject kv : doc["relay"].as<JsonArray>())
    {
        const char* device_name = kv["name"];
        fauxmo_remove_device(device_name);
    }
}

void fauxmo_add_device()
{
    read_config();
    String fauxmo_relay = conf.fauxmo_relay;
    JsonDocument doc;
    deserializeJson(doc, fauxmo_relay);

    for( JsonPair kv : doc.as<JsonObject>())
    {
        bool value = kv.value();
        const char *key = kv.key().c_str();
        if(value)
        {
            serialDisplay("fauxmo_add_device", "Adding Device : " + String(key));
            fauxmo_add_device(key);
        }
        else{
            serialDisplay("fauxmo_add_device", "Removing Device : " + String(key));
            fauxmo_remove_device(key);
        }
    }
}

void fauxmo_sync_all_states()
{
    String mqtt_data = read_mqtt_config();
    JsonDocument doc;
    JsonDocument filter;
    filter["relay"][0]["name"] = true;
    filter["relay"][0]["topic"] = true;
    filter["relay"][0]["status"] = true;
    filter["relay"][0]["comp"] = true;
    filter["relay"][0]["pin"] = true;
    deserializeJson(doc, mqtt_data, DeserializationOption::Filter(filter));
    doc.shrinkToFit();

    for (JsonObject ele : doc["relay"].as<JsonArray>())
    {
        String name = ele["name"];
        String topic = ele["topic"];
        String comp_type = ele["comp"];
        int pin = ele["pin"];
        bool state = false;

        if (comp(comp_type.c_str(), "shift_reg"))
            state = sr.get(pin);
        else if (comp(comp_type.c_str(), "gpio"))
            state = digitalRead(pin);
        else
            state = ele["status"];

        fauxmo.setState(topic.c_str(), state, state ? 255 : 0);
    }
}

void fauxmo_sync_state(String relay_name, bool state)
{
    String mqtt_data = read_mqtt_config();
    JsonDocument doc;
    JsonDocument filter;
    filter["relay"][0]["name"] = true;
    filter["relay"][0]["topic"] = true;
    deserializeJson(doc, mqtt_data, DeserializationOption::Filter(filter));
    doc.shrinkToFit();

    for (JsonObject ele : doc["relay"].as<JsonArray>())
    {
        String name = ele["name"];
        if (comp(name.c_str(), relay_name.c_str()))
        {
            const char* topic = ele["topic"];
            fauxmo.setState(topic, state, state ? 255 : 0);
            break;
        }
    }
}