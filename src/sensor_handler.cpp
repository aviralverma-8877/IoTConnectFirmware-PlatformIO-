#include<Arduino.h>
#include<ArduinoJson.h>
#include"device_handler.h"
/*-----Meathod for sending sensor data----------------------*/

void ICACHE_RAM_ATTR handleData(float h, float t) {
  JsonDocument doc;
  volatile float humidity = h;
  volatile float temperature = t;
  doc["action"] = "sensor";
  doc["d"] = chipid;
  doc["t"] = temperature;
  doc["h"] = humidity;
  doc["l"] = illuminance_value;
  String s;
  serializeJson(doc, s);
  send_data_to_webSocket(s);
  sendToMQTT(espsensor, s);
}

void ICACHE_RAM_ATTR handleError(uint8_t e) {
}

void setup_sensor()
{
  JsonDocument doc;
  JsonDocument filter;
  filter["device_config"]["dht"]["INSTALLED"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
    return;
  bool has_dht = doc["device_config"]["dht"]["INSTALLED"];
  if(has_dht)
  {
    if(comp(DHTType.c_str(), "dht11"))
    {
      sensor_dht11.setup(dht_pin);
      sensor_dht11.onData(handleData);
      sensor_dht11.onError(handleError);
    }
    else if(comp(DHTType.c_str(), "dht22"))
    {
      sensor_dht22.setup(dht_pin);
      sensor_dht22.onData(handleData);
      sensor_dht22.onError(handleError);
    }  
  }
}

void sendSensorData()
{
  JsonDocument doc;
  JsonDocument filter;
  filter["device_config"]["dht"]["INSTALLED"] = true;
  filter["device_config"]["light"]["INSTALLED"] = true;
  String device_config = read_device_config();
  DeserializationError error = deserializeJson(doc, device_config, DeserializationOption::Filter(filter));
  if(error)
    return;
  bool has_dht = doc["device_config"]["dht"]["INSTALLED"];
  bool has_light = doc["device_config"]["light"]["INSTALLED"];
  if(has_dht)
  {
    if(comp(DHTType.c_str(), "dht11"))
    {
      sensor_dht11.read();
    }
    else if(comp(DHTType.c_str(), "dht22"))
    {
      sensor_dht22.read();
    }  
  }
  if(has_light)
  {
    illuminance_value = map(analogRead(LDR_PIN), 0, 1024, 0, 100);
  }
}
/*-----Meathod for sending sensor data----------------------*/