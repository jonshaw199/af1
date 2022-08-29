
#include "demo3Master.h"
#include "stateEnt/demo3/demo3.h"

Demo3Master::Demo3Master()
{
  intervalEvents.push_back(IntervalEvent(MS_DEMO3_LOOP, demo3));
}

bool Demo3Master::demo3(IECBArg a)
{
  return true;
}
