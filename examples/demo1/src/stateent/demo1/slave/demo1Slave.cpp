
#include "demo1Slave.h"

void Demo1Slave::setup()
{
  Base::setup();
  LED::init();
}

bool Demo1Slave::preStateChange(int s)
{
  bool baseResult = Base::preStateChange(s);
  if (baseResult)
  {
    Serial.println("Turning off lights on the way out");
    LED::fillColor(CRGB::Black);
  }
  return baseResult;
}

bool Demo1Slave::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    CRGB c;
    memcpy(&c, m.getData(), sizeof(c));
    LED::fillColor(c);
    return true;
  }

  return Base::handleInboxMsg(m);
}

void Demo1Slave::setInboxMessageHandler()
{
  MessageHandler::setInboxMsgHandler(handleInboxMsg);
}
