#ifndef STATEENT_SYNCTEST_SYNCTEST_H_
#define STATEENT_SYNCTEST_SYNCTEST_H_

#include "stateEnt/virtual/base/base.h"

class SyncTest : public Base
{
public:
  bool doSync()
  {
    return true;
  }
};

#endif