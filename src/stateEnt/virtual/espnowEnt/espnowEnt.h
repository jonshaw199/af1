#ifndef STATEENT_VIRTUAL_ESPNOWENT_ESPNOWENT_H_
#define STATEENT_VIRTUAL_ESPNOWENT_ESPNOWENT_H_

#include "stateEnt/virtual/base/base.h"

class ESPNowEnt : public Base
{
protected:
  static bool handleInboxMsg(JSMessage m);
  static bool handleOutboxMsg(JSMessage m);

public:
  virtual void setInboxMessageHandler();
  virtual void setOutboxMessageHandler();
  virtual bool preStateChange(int s);
};

#endif // STATEENT_VIRTUAL_ESPNOWENT_ESPNOWENT_H_