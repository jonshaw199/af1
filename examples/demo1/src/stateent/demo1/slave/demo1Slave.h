#ifndef STATEENT_DEMO1_SLAVE_DEMO1SLAVE_H_
#define STATEENT_DEMO1_SLAVE_DEMO1SLAVE_H_

#include <framework.h>

class Demo1Slave : public Base
{
public:
  void setup();
  bool preStateChange(int s);
  static bool handleInboxMsg(JSMessage m);
  void setInboxMessageHandler();
};

#endif // STATEENT_DEMO1_SLAVE_DEMO1SLAVE_H_