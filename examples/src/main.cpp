
#include <Arduino.h>

#include <AF1.h>

#if MASTER
#include "stateEnt/demo2/master/demo2Master.h"
#else
#include "stateEnt/demo2/slave/demo2Slave.h"
#endif
#include "state.h"
#include "stateEnt/demo4/demo4.h"

void setStateDemo2()
{
  AF1::setRequestedState(STATE_DEMO2);
}

void setStateDemo4()
{
  AF1::setRequestedState(STATE_DEMO4);
}

void setup()
{
  Serial.begin(JS_BAUD);
  AF1::setup();
#if MASTER
  AF1::registerStateEnt(STATE_DEMO2, new Demo2Master(), "STATE_DEMO2");
#else
  AF1::registerStateEnt(STATE_DEMO2, new Demo2Slave(), "STATE_DEMO2");
#endif
  AF1::registerStateEnt(STATE_DEMO4, new Demo4(), "STATE_DEMO4");
  AF1::registerStringHandler("2", setStateDemo2);
  AF1::registerStringHandler("4", setStateDemo4);
}

void loop()
{
  AF1::loop();
}
