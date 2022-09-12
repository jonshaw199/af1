
#include <Arduino.h>

#include <AF1.h>

#if MASTER
#include "stateEnt/demo2/master/demo2Master.h"
#include "stateEnt/demo6/master/demo6Master.h"
#else
#include "stateEnt/demo2/slave/demo2Slave.h"
#include "stateEnt/demo6/slave/demo6Slave.h"
#endif
#include "state.h"
#include "stateEnt/demo4/demo4.h"

void setup()
{
  Serial.begin(JS_BAUD);
  AF1::setup(JS_ID);
  AF1::registerWifiAP("js-guest", "B1g5lams!");
#if MASTER
  AF1::registerStateEnt(STATE_DEMO2, new Demo2Master());
  AF1::registerStateEnt(STATE_DEMO6, new Demo6Master());
#else
  AF1::registerStateEnt(STATE_DEMO2, new Demo2Slave());
  AF1::registerStateEnt(STATE_DEMO6, new Demo6Slave());
#endif
  Demo4 *d = new Demo4();
  d->setWSClientInfo({"192.168.1.65", "/ws", 3000, ""});
  AF1::registerStateEnt(STATE_DEMO4, d);
  AF1::registerStringHandler("2", []()
                             { AF1::setRequestedState(STATE_DEMO2); });
  AF1::registerStringHandler("4", []()
                             { AF1::setRequestedState(STATE_DEMO4); });
  AF1::registerStringHandler("6", []()
                             { AF1::setRequestedState(STATE_DEMO6); });
}

void loop()
{
  AF1::loop();
}
