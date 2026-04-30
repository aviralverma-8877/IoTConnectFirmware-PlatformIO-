#include<Arduino.h>
#include<ArduinoJson.h>
#include"device_handler.h"

/*-----Method for sending sensor data----------------------*/
void IRAM_ATTR handleData(float h, float t) {
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
  if(MQTTStatus)
    sendToMQTT(espsensor, s);
}

void IRAM_ATTR handleError(uint8_t e) {
  // DHT sensor error codes: 1=Timeout, 2=Checksum error, 3=Unknown
  serialDisplay("DHT_Error", "Sensor read failed, error code: " + String(e));
}

void setup_sensor()
{
  JsonDocument doc;
  JsonDocument filter;
  filter["device_config"]["dht"]["INSTALLED"] = true;
  String device_config = read_device_config();
  deserializeJson(doc, device_config, DeserializationOption::Filter(filter));

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
  deserializeJson(doc, device_config, DeserializationOption::Filter(filter));

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
    illuminance_value = map(analogRead(LDR_PIN), 0, ESP8266_ADC_MAX, 0, ILLUMINANCE_PERCENT_MAX);
  }
}
/*-----Method for sending sensor data----------------------*/