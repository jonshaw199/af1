
#include "demo3Slave.h"
#include "led/led.h"
#include "stateEnt/demo3/demo3.h"

void Demo3Slave::setup()
{
  Base::setup();
  JSLED::init();
}

bool Demo3Slave::preStateChange(int s)
{
  bool baseResult = Base::preStateChange(s);
  if (baseResult)
  {
    Serial.println("Turning off lights on the way out");
    JSLED::fillColor(CRGB::Black);
  }
  return baseResult;
}

bool Demo3Slave::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    demo3_data d;
    memcpy(&d, m.getData(), sizeof(d));
    JSLED::setBrightness(d.brightness);
    JSLED::fillColor(d.color);
    return true;
  }

  return Base::handleInboxMsg(m);
}

void Demo3Slave::setInboxMessageHandler()
{
  setInboxMsgHandler(handleInboxMsg);
}
