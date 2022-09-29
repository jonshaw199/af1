#include "sync.h"
#include "state/state.h"

STArg::STArg(IECBArg a) : iecbArg(a) {}

IECBArg STArg::getIECBArg()
{
  return iecbArg;
}

Sync *Sync::getInstance()
{
  return static_cast<Sync *>(StateManager::getCurStateEnt());
}

void Sync::scheduleStart()
{
  Serial.println("Scheduling");
  Serial.print("Current time: ");
  Serial.print(millis());
  Serial.print("; Start time: ");
  Serial.print(getInstance()->startTime);
  Serial.print("; diff: ");
  Serial.println(getInstance()->startTime - millis());

  unsigned long dif = getInstance()->startTime - millis();
  unsigned long intervalMs = dif + StateManager::getCurStateEnt()->getElapsedMs();

  StateManager::getCurStateEnt()->getIntervalEventMap().insert(std::pair<String, IntervalEvent>("Sync_ScheduleStart", IntervalEvent(
                                                                                                                          intervalMs, [](IECBArg a)
                                                                                                                          {
            Serial.println("Starting");
            getInstance()->getSyncedTask()(a);
            return true; },
                                                                                                                          1, true)));
}

Sync::Sync()
{
  startTime = 0;
  // Default is blinking LED but overridden using setSyncedTask()
  syncedTask = [](STArg a)
  {
    StateManager::getCurStateEnt()->getIntervalEventMap().insert(std::pair<String, IntervalEvent>("Sync_Start", IntervalEvent(
                                                                                                                    300, [](IECBArg a)
                                                                                                                    {
            setBuiltinLED(a.getCbCnt() % 2);
            return true; },
                                                                                                                    -1, true)));
  };

#if MASTER
  // Schedule send start time
  intervalEventMap.insert(std::pair<String, IntervalEvent>("Sync_SendStartTime", IntervalEvent(
                                                                                     3000, [](IECBArg a)
                                                                                     {
            getInstance()->startTime = millis() + (unsigned long)6000;
            
            AF1Msg msg;
            msg.setState(STATE_SYNC);
            msg.setType(TYPE_RUN_DATA);
            sync_data d;
            d.ms = getInstance()->startTime;
            msg.setData((uint8_t *)&d);
            pushOutbox(msg);
            scheduleStart();
            return true; },
                                                                                     1)));
#endif
}

msg_handler Sync::getInboxHandler()
{
  return [](AF1Msg m)
  {
    Base::handleInboxMsg(m);
    if (m.getState() == STATE_SYNC)
    {
      switch (m.getType())
      {
      case TYPE_RUN_DATA:
        sync_data d;
        memcpy(&d, m.getData(), sizeof(d));
        Serial.print("Received time: ");
        Serial.println(d.ms);
        getInstance()->startTime = StateManager::convertTime(m.getSenderID(), d.ms);
        Serial.print("Converted time: ");
        Serial.println(getInstance()->startTime);
        scheduleStart();
        break;
      }
    }
  };
}

bool Sync::doScanForPeersESPNow()
{
  return false;
}

void Sync::preStateChange(int s)
{
  Base::preStateChange(s);
  setBuiltinLED(0);
}

void Sync::setSyncedTask(synced_task t)
{
  syncedTask = t;
}

synced_task Sync::getSyncedTask()
{
  return syncedTask;
}
