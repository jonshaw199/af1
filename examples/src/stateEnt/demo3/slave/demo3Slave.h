#ifndef STATEENT_DEMO3_SLAVE_DEMO3SLAVE_H_
#define STATEENT_DEMO3_SLAVE_DEMO3SLAVE_H_

#include <AF1.h>

class Demo3Slave : public ESPNowEnt
{
public:
  void setup();
  bool preStateChange(int s);
  static bool handleInboxMsg(JSMessage m);
  void setInboxMessageHandler();
};

#endif // STATEENT_DEMO3_SLAVE_DEMO1SLAVE_H_