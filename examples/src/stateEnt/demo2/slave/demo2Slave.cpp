
#include "demo2Slave.h"
#include "led/led.h"
#include "stateEnt/demo2/demo2.h"

void Demo2Slave::setup()
{
  Base::setup();
  JSLED::init();
}

bool Demo2Slave::preStateChange(int s)
{
  bool baseResult = Base::preStateChange(s);
  if (baseResult)
  {
    Serial.println("Turning off lights on the way out");
    JSLED::fillColor(CRGB::Black);
  }
  return baseResult;
}

void Demo2Slave::setInboxMessageHandler()
{
  setInboxMsgHandler([](JSMessage m)
                     {  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    demo2_data d;
    memcpy(&d, m.getData(), sizeof(d));
    JSLED::setBrightness(d.brightness);
    JSLED::fillColor(d.color);
    return true;
  }

  return Base::handleInboxMsg(m); });
}
