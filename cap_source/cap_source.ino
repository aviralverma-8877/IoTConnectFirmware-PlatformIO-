#include <ArduinoJson.h>
#include <ShiftRegister74HC595.h>

// create shift register object (number of shift registers, data pin, clock pin, latch pin)
ShiftRegister74HC595<1> sr (8, 10, 9); 
int pins[] = {3,4,5,6,7,11,12,13};
bool val[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
int lastPushed = -1;
void setup() { 
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  for(int i=0; i<8; i++)
  {
    pinMode(pins[i], INPUT);
    sr.set(i, val[i]);
  }
}

void loop() 
{
  bool pressed = false;
  digitalWrite(2, LOW);
  for(int i=0; i<8; i++)
  {
    if(digitalRead(pins[i]) == HIGH)
    {
      StaticJsonDocument<200> doc;
      digitalWrite(2, HIGH);
      pressed = true;
      if(lastPushed != i)
      {
        lastPushed = i;
        bool value = !val[i];
        doc["a"] = "R";
        doc["n"] = i;
        doc["v"] = value;
        serializeJson(doc, Serial);
        delay(100);
      }
    }
  }
  if(pressed == false)
    lastPushed = -1;
  if(Serial.available())
  {
    char r;
    String data = "";
    delay(10);
    while(Serial.available())
    {
      r = Serial.read();
      data += r;
    }
    data.trim();
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);
    if(error)
    {
      return;
    }
    String action = doc["a"];
    if(action == "RS")
    {
      for(int i=0; i<8; i++)
      {
        
        bool value = doc["n"][i];
        sr.set(i, value);
        val[i] = value;
      }
    }
  }
}
