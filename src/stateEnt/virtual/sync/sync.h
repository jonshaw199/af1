#ifndef STATEENT_VIRTUAL_SYNC_SYNC_H_
#define STATEENT_VIRTUAL_SYNC_SYNC_H_

#include <AF1.h>

typedef struct sync_data
{
  unsigned long ms;
} sync_data;

class STArg
{
  const IECBArg iecbArg;

public:
  STArg(IECBArg a);
  IECBArg getIECBArg();
};

typedef void (*synced_task)(STArg a);

class Sync : public Base
{
  unsigned long startTime;
  synced_task syncedTask;
  static void scheduleStart();

protected:
  void setSyncedTask(synced_task t);

public:
  Sync();
  static Sync *getInstance();
  msg_handler getInboxHandler();
  bool doScanForPeersESPNow();
  void preStateChange(int s);
  synced_task getSyncedTask();
};

#endif