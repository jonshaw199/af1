#ifndef STATEENT_DEMO4_DEMO4_H_
#define STATEENT_DEMO4_DEMO4_H_

#include <AF1.h>

class Demo4 : public WSEnt
{
  Demo4();
  void setup();
  bool validateStateChange(int s);
  void overrideInboxHandler();
};

#endif // STATEENT_DEMO4_DEMO4_H_
