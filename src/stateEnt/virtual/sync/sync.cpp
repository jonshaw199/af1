/*
  AF1
  Copyright (c) 2022 Jon Shaw. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the license, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

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
                                                                                                                    -1, true, StateManager::getCurStateEnt()->getElapsedMs() / 300)));
  };

#if MASTER
  // Schedule send start time
  intervalEventMap.insert(std::pair<String, IntervalEvent>("Sync_SendStartTime", IntervalEvent(
                                                                                     MS_TIME_SYNC_SCHEDULE_START, [](IECBArg a)
                                                                                     {
            getInstance()->startTime = millis() + (unsigned long)MS_TIME_SYNC_START;
            
            AF1Msg msg;
            msg.setState(StateManager::getCurState());
            msg.setType(TYPE_TIME_SYNC_START);
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
    switch (m.getType())
    {
    case TYPE_TIME_SYNC_START:
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
