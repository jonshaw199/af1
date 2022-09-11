
#include "demo2Slave.h"
#include "led/led.h"
#include "stateEnt/demo2/demo2.h"

void Demo2Slave::setup()
{
  Base::setup();
  JSLED::init();
}

void Demo2Slave::preStateChange(int s)
{
  Serial.println("Turning off lights on the way out");
  JSLED::fillColor(CRGB::Black);
}

void Demo2Slave::overrideInboxHandler()
{
  setInboxMsgHandler([](AF1Msg m)
                     { 
  Base::handleInboxMsg(m); 
  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    demo2_data d;
    memcpy(&d, m.getData(), sizeof(d));
    JSLED::setBrightness(d.brightness);
    JSLED::fillColor(d.color);
  } });
}
