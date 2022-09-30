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

#ifndef STATEENT_BASE_TIMEEVENT_H_
#define STATEENT_BASE_TIMEEVENT_H_

#define MAX_CB_CNT_INF -1;

class TECBArg
{
    const unsigned long curMs;
    const unsigned long startMs;
    const unsigned long intervalMs;
    const int cbCnt;
    const int maxCbCnt;

public:
    TECBArg(unsigned long c0, unsigned long s, unsigned long i, int c, int m);
    unsigned long getCurMs();
    unsigned long getStartMs();
    unsigned long getIntervalMs();
    int getCbCnt();
    int getMaxCbCnt();
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
    int maxCbCnt;
    bool transitory;
    int mode = TE_MODE_ACTIVE;
    int cbCnt;

public:
    TimeEvent();
    TimeEvent(unsigned long s, time_event_cb c);
    TimeEvent(unsigned long s, time_event_cb c, unsigned long i);
    TimeEvent(unsigned long s, time_event_cb c, unsigned long i, int m);
    TimeEvent(unsigned long s, time_event_cb c, unsigned long i, int m, bool t);
    TimeEvent(unsigned long s, time_event_cb c, unsigned long i, int m, bool t, int c2);
    TimeEvent(unsigned long s, time_event_cb c, unsigned long i, int m, bool t, int c2, int m2);
    unsigned long getStartMs();
    unsigned long getIntervalMs();
    int getMaxCbCnt();
    bool getTransitory();
    time_event_cb getCb();
    int getCbCnt();

    unsigned long getLastCbMs();
    unsigned long getNextCbMs();
    bool isTime(unsigned long curMs);
    bool cbIfTimeAndActive(unsigned long curMs);
    void setMode(int m);
    int getMode();

    void reset();
    void activate();
    void deactivate();
};

#endif // STATEENT_BASE_TIMEEVENT_H_
