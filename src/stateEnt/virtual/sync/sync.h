#ifndef STATEENT_VIRTUAL_SYNC_SYNC_H_
#define STATEENT_VIRTUAL_SYNC_SYNC_H_

#include <AF1.h>

typedef struct sync_data
{
  unsigned long ms;
} sync_data;

typedef void (*synced_task)();

class Sync : public Base
{
  unsigned long startTime;
  void scheduleStart();
  synced_task syncedTask;

protected:
  void setSyncedTask(synced_task t);

public:
  Sync();
  msg_handler getInboxHandler();
  bool doScanForPeersESPNow();
  void preStateChange(int s);
  synced_task getSyncedTask();
};

#endif