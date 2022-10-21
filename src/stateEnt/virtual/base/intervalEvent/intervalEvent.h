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

#ifndef STATEENT_BASE_INTERVALEVENT_H_
#define STATEENT_BASE_INTERVALEVENT_H_

#include "stateEnt/virtual/base/event/event.h"

#define MAX_CB_CNT_INF -1;

class IECBArg
{
  const unsigned long elapsedMs;
  const unsigned long cbCnt;
  const unsigned long maxCbCnt;
  const String name;

public:
  IECBArg(String n, unsigned long e, unsigned long c, unsigned long m);
  String getName();
  unsigned long getElapsedMs();
  unsigned long getCbCnt();
  unsigned long getMaxCbCnt();
};

typedef bool (*interval_event_cb)(IECBArg a);

enum IntervalEventMode
{
  IE_MODE_ACTIVE,
  IE_MODE_INACTIVE
};

class IntervalEvent : public Event
{
  interval_event_cb cb;
  unsigned long maxCbCnt = MAX_CB_CNT_INF;
  bool temporary = false;
  uint8_t mode = IE_MODE_ACTIVE;

public:
  IntervalEvent();
  IntervalEvent(String n, unsigned long i, interval_event_cb c);
  IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m);
  IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m, bool t);
  IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m, bool t, unsigned long c2);
  IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m, bool t, unsigned long c2, uint8_t m2);
  unsigned long getMaxCbCnt();
  void setMaxCbCnt(unsigned long c);
  interval_event_cb getCb();
  void setCb(interval_event_cb c);

  unsigned long getLastCbMs();
  unsigned long getNextCbMs();
  bool isTime(unsigned long elapsedMs);
  bool cbIfTimeAndActive(unsigned long elapsedMs);
  void setMode(uint8_t m);
  uint8_t getMode();
  bool getTemporary();

  void reset();
  void activate();
  void deactivate();
};

#endif // STATEENT_BASE_INTERVALEVENT_H_
