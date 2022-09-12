#ifndef STATEENT_Demo6_SLAVE_Demo6SLAVE_H_
#define STATEENT_Demo6_SLAVE_Demo6SLAVE_H_

#include <AF1.h>

class Demo6Slave : public Base
{
public:
  void setup();
  void preStateChange(int s);
  void overrideInboxHandler();
};

#endif // STATEENT_Demo6_SLAVE_DEMO1SLAVE_H_