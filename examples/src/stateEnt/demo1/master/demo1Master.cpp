
#include "demo1Master.h"
#include "state.h"
#include "led/led.h"

Demo1Master::Demo1Master()
{
  intervalEvents.push_back(IntervalEvent(MS_DEMO1_LOOP, demo1));
}

bool Demo1Master::demo1(IECBArg a)
{
  JSMessage msg;
  msg.setState(STATE_DEMO1);
  msg.setType(TYPE_RUN_DATA);
  msg.setMaxRetries(MS_DEMO1_LOOP >= 1000 ? 3 : 0);
  CRGB c = JSLED::getRandColor();
  msg.setData((uint8_t *)&c);
  pushOutbox(msg);

  return true;
}
