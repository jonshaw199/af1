#ifndef STATEENT_DEMO4_DEMO4_H_
#define STATEENT_DEMO4_DEMO4_H_

#include <AF1.h>

class Demo4 : public WSEnt
{
  void setup();
  bool preStateChange(int s);
  static bool handleInboxMsg(JSMessage m);
  void setInboxMessageHandler();
};

#endif // STATEENT_DEMO4_DEMO4_H_
