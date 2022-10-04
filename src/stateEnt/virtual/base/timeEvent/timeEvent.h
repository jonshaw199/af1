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

#include <Arduino.h>

#ifndef STATEENT_BASE_TIMEEVENT_H_
#define STATEENT_BASE_TIMEEVENT_H_

#define MAX_CB_CNT_INF -1;

class TECBArg
{
    const unsigned long curMs;
    const unsigned long startMs;
    const unsigned long intervalMs;
    const unsigned long cbCnt;
    const unsigned long maxCbCnt;
    String name;

public:
    TECBArg(String n, unsigned long c0, unsigned long s, unsigned long i, unsigned long c, unsigned long m);
    String getName();
    unsigned long getCurMs();
    unsigned long getStartMs();
    unsigned long getIntervalMs();
    unsigned long getCbCnt();
    unsigned long getMaxCbCnt();
};

typedef bool (*time_event_cb)(TECBArg a);

enum TimeEventMode
{
    TE_MODE_ACTIVE,
    TE_MODE_INACTIVE
};

class TimeEvent
{
    unsigned long startMs;
    unsigned long intervalMs;
    time_event_cb cb;
    unsigned long maxCbCnt;
    bool temporary;
    uint8_t mode = TE_MODE_ACTIVE;
    unsigned long cbCnt;
    String name;

public:
    TimeEvent();
    TimeEvent(String n, unsigned long s, time_event_cb c);
    TimeEvent(String n, unsigned long s, time_event_cb c, unsigned long i);
    TimeEvent(String n, unsigned long s, time_event_cb c, unsigned long i, unsigned long m);
    TimeEvent(String n, unsigned long s, time_event_cb c, unsigned long i, unsigned long m, bool t);
    TimeEvent(String n, unsigned long s, time_event_cb c, unsigned long i, unsigned long m, bool t, unsigned long c2);
    TimeEvent(String n, unsigned long s, time_event_cb c, unsigned long i, unsigned long m, bool t, unsigned long c2, uint8_t m2);
    unsigned long getStartMs();
    void setStartMs(unsigned long m);
    unsigned long getIntervalMs();
    void setIntervalMs(unsigned long m);
    unsigned long getMaxCbCnt();
    void setMaxCbCnt(unsigned long m);
    bool getTemporary();
    void setTemporary(bool t);
    time_event_cb getCb();
    void setCb(time_event_cb c);
    unsigned long getCbCnt();
    void setCbCnt(unsigned long c);
    void setName(String s);
    String getName();

    unsigned long getLastCbMs();
    unsigned long getNextCbMs();
    bool isTime(unsigned long curMs);
    bool cbIfTimeAndActive(unsigned long curMs);
    void setMode(uint8_t m);
    uint8_t getMode();

    void reset();
    void activate();
    void deactivate();
};

#endif // STATEENT_BASE_TIMEEVENT_H_
