#include "demo4.h"
#include "led/led.h"

Demo4::Demo4() : WSEnt{"192.168.1.65", "/ws", 3000} {}

void Demo4::setup()
{
  WSEnt::setup();
  JSLED::init();
}

bool Demo4::validateStateChange(int s)
{
  Serial.println("Turning off lights on the way out");
  JSLED::fillColor(CRGB::Black);
  return true;
}

void Demo4::overrideInboxHandler()
{
  setInboxMsgHandler([](AF1Msg m)
                     {   
  WSEnt::handleInboxMsg(m); 
  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    uint8_t b = m.getJson()["brightness"];
    JSLED::setBrightness(b);
    uint8_t r = m.getJson()["red"];
    uint8_t g = m.getJson()["green"];
    uint8_t b2 = m.getJson()["blue"];
    JSLED::fillColor(CRGB(r, g, b2));
  } });
}
