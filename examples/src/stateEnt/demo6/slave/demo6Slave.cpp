
#include "demo6Slave.h"
#include "led/led.h"
#include "stateEnt/demo6/demo6.h"

void Demo6Slave::setup()
{
  Base::setup();
  JSLED::init();
}

void Demo6Slave::preStateChange(int s)
{
  Serial.println("Turning off lights on the way out");
  JSLED::fillColor(CRGB::Black);
}

void Demo6Slave::overrideInboxHandler()
{
  setInboxMsgHandler([](AF1Msg m)
                     { 
  Base::handleInboxMsg(m); 
  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    demo6_data d;
    memcpy(&d, m.getData(), sizeof(d));
    JSLED::setBrightness(d.brightness);
    JSLED::fillColor(d.color);
  } });
}
