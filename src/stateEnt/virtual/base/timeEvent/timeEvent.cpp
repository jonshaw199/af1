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

#include "timeEvent.h"

TECBArg::TECBArg(unsigned long c0, unsigned long s, unsigned long i, unsigned long c, unsigned long m) : curMs(c0), startMs(s), intervalMs(i), cbCnt(c), maxCbCnt(m) {}

unsigned long TECBArg::getCurMs()
{
  return curMs;
}

unsigned long TECBArg::getStartMs()
{
  return startMs;
}

unsigned long TECBArg::getIntervalMs()
{
  return intervalMs;
}

unsigned long TECBArg::getCbCnt()
{
  return cbCnt;
}

unsigned long TECBArg::getMaxCbCnt()
{
  return maxCbCnt;
}

TimeEvent::TimeEvent() : startMs(0), intervalMs(0), maxCbCnt(0), cbCnt(0) {}

TimeEvent::TimeEvent(unsigned long s, time_event_cb c) : startMs(s), intervalMs(0), cb(c), maxCbCnt(1), cbCnt(0) {}

TimeEvent::TimeEvent(unsigned long s, time_event_cb c, unsigned long i) : startMs(s), intervalMs(i), cb(c), cbCnt(0)
{
  maxCbCnt = MAX_CB_CNT_INF;
}

TimeEvent::TimeEvent(unsigned long s, time_event_cb c, unsigned long i, unsigned long m) : startMs(s), intervalMs(i), cb(c), maxCbCnt(m), cbCnt(0) {}

TimeEvent::TimeEvent(unsigned long s, time_event_cb c, unsigned long i, unsigned long m, bool t) : startMs(s), intervalMs(i), cb(c), maxCbCnt(m), cbCnt(0), transitory(t) {}

TimeEvent::TimeEvent(unsigned long s, time_event_cb c, unsigned long i, unsigned long m, bool t, unsigned long c2) : startMs(s), intervalMs(i), cb(c), maxCbCnt(m), cbCnt(c2), transitory(t) {}

TimeEvent::TimeEvent(unsigned long s, time_event_cb c, unsigned long i, unsigned long m, bool t, unsigned long c2, int m2) : startMs(s), intervalMs(i), cb(c), maxCbCnt(m), cbCnt(c2), transitory(t), mode(m2) {}

unsigned long TimeEvent::getStartMs()
{
  return startMs;
}

void TimeEvent::setStartMs(unsigned long s)
{
  startMs = s;
}

unsigned long TimeEvent::getIntervalMs()
{
  return intervalMs;
}

void TimeEvent::setIntervalMs(unsigned long i)
{
  intervalMs = i;
}

time_event_cb TimeEvent::getCb()
{
  return cb;
}

void TimeEvent::setCb(time_event_cb c)
{
  cb = c;
}

unsigned long TimeEvent::getMaxCbCnt()
{
  return maxCbCnt;
}

void TimeEvent::setMaxCbCnt(unsigned long m)
{
  maxCbCnt = m;
}

unsigned long TimeEvent::getCbCnt()
{
  return cbCnt;
}

void TimeEvent::setCbCnt(unsigned long c)
{
  cbCnt = c;
}

bool TimeEvent::getTransitory()
{
  return transitory;
}

void TimeEvent::setTransitory(bool t)
{
  transitory = t;
}

// Unlike IntervalEvents, this is only used when an intervalMs is defined
unsigned long TimeEvent::getLastCbMs()
{
  return intervalMs * cbCnt + startMs;
}

unsigned long TimeEvent::getNextCbMs()
{
  return intervalMs ? getLastCbMs() + intervalMs : startMs;
}

bool TimeEvent::isTime(unsigned long curMs)
{
  return curMs >= getNextCbMs() && (maxCbCnt < 0 || cbCnt < maxCbCnt);
}

bool TimeEvent::cbIfTimeAndActive(unsigned long curMs)
{
  if (mode == TE_MODE_ACTIVE && isTime(curMs) && startMs && cb(TECBArg(curMs, startMs, intervalMs, cbCnt, maxCbCnt))) // Checking intervalMs here since default constructor doesnt even define cb; might need stub there to be safe
  {
    unsigned long elapsedMs = curMs - startMs;
    cbCnt = intervalMs ? elapsedMs / intervalMs : cbCnt + 1; // Setting cbCnt to expected value rather than just incrementing (unless intervalMs is 0)
    return true;
  }
  return false;
}

void TimeEvent::setMode(int m)
{
  mode = m;
}

int TimeEvent::getMode()
{
  return mode;
}

void TimeEvent::reset()
{
  cbCnt = 0;
}

void TimeEvent::activate()
{
  mode = TE_MODE_ACTIVE;
}

void TimeEvent::deactivate()
{
  mode = TE_MODE_INACTIVE;
}
