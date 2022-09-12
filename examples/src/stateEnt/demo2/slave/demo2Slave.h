#ifndef STATEENT_DEMO2_SLAVE_DEMO2SLAVE_H_
#define STATEENT_DEMO2_SLAVE_DEMO2SLAVE_H_

#include <AF1.h>

class Demo2Slave : public Base
{
public:
  void setup();
  void preStateChange(int s);
  void overrideInboxHandler();
};

#endif // STATEENT_DEMO2_SLAVE_DEMO1SLAVE_H_