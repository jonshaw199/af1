#ifndef STATEENT_DEMO1_MASTER_DEMO1MASTER_H_
#define STATEENT_DEMO1_MASTER_DEMO1MASTER_H_

#include <AF1.h>

class Demo1Master : public ESPNowEnt
{
  static bool demo1(IECBArg a);

public:
  Demo1Master();
};

#endif // STATEENT_DEMO1_MASTER_DEMO1MASTER_H_