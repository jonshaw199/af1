#include "espnowEntMaster.h"

bool ESPNowEntMaster::preStateChange(int s)
{
  bool baseResult = Base::preStateChange(s);
  if (baseResult)
  {
    int slaveState = s;
    switch (s)
    {
    case STATE_OTA:       // Slaves already notified
    case STATE_RESTART:   // Slaves already notified
    case STATE_HANDSHAKE: // Slaves can' be notified
      return true;
    case STATE_PURG_OTA:
      slaveState = STATE_OTA;
      break;
    case STATE_PURG_RESTART:
      slaveState = STATE_RESTART;
      break;
    }
    sendStateChangeMessages(slaveState);
  }
  return baseResult;
}
