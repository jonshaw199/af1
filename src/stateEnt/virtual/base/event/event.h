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

#ifndef STATEENT_BASE_EVENT_H_
#define STATEENT_BASE_EVENT_H_

#include <Arduino.h>

enum start_time_type
{
  START_STATE_MS,
  START_DEVICE_MS,
  START_EPOCH_SEC
};

enum event_mode
{
  MODE_INACTIVE,
  MODE_ACTIVE
};

class ECBArg
{
public:
  const unsigned long curTime;
  const unsigned long intervalTime;
  const unsigned long startTime;
  const start_time_type startTimeType;
  const unsigned long cbCnt;
  const unsigned long maxCbCnt;
  const bool temporary;
  const String name;
  ECBArg(unsigned long e, unsigned long i, unsigned long s, start_time_type s2, unsigned long c, unsigned long m, String n, bool t);
};

typedef void (*event_cb)(ECBArg a);

class Event
{
private:
  unsigned long intervalTime;
  unsigned long cbCnt;
  unsigned long startTime;
  start_time_type startTimeType;
  String name;
  unsigned long maxCbCnt;
  bool temporary;
  event_mode mode;
  event_cb cb;

public:
  Event();
  Event(String name, event_cb cb, bool temporary = false, unsigned long intervalTime = 0, unsigned long maxCbCnt = 0, unsigned long startTime = 0, start_time_type = START_STATE_MS, event_mode mode = MODE_ACTIVE, unsigned long cbCnt = 0);
  unsigned long getIntervalTime();
  void setIntervalTime(unsigned long time);
  void setIntervalTime(unsigned long time, unsigned long curTime); // Updates cbCnt based on curTime; for "time bending"
  unsigned long getCbCnt();
  void setCbCnt(unsigned long c);
  String getName();
  void setName(String s);
  unsigned long getMaxCbCnt();
  void setMaxCbCnt(unsigned long c);
  unsigned long getStartTime();
  void setStartTime(unsigned long s);
  void setCb(event_cb c);
  event_cb getCb();

  unsigned long getLastCbTime();
  unsigned long getNextCbTime();
  bool isTime(unsigned long curTime);
  bool cbIfTimeAndActive(unsigned long curTime);
  unsigned long getCurTime();
  bool cbIfTimeAndActive();
  void setMode(event_mode m);
  event_mode getMode();
  bool getTemporary();

  void reset();
  void activate();
  void deactivate();
};

#endif // STATEENT_BASE_EVENT_H_
