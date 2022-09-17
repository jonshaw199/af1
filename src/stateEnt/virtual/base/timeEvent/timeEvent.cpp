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

#include "timeEvent.h"

TECBArg::TECBArg(unsigned long long c0, unsigned long long s, unsigned long long i, int c, int m) : curMs(c0), startMs(s), intervalMs(i), cbCnt(c), maxCbCnt(m) {}

unsigned long long TECBArg::getCurMs()
{
  return curMs;
}

unsigned long long TECBArg::getStartMs()
{
  return startMs;
}

unsigned long long TECBArg::getIntervalMs()
{
  return intervalMs;
}

int TECBArg::getCbCnt()
{
  return cbCnt;
}

int TECBArg::getMaxCbCnt()
{
  return maxCbCnt;
}

TimeEvent::TimeEvent()
{
  startMs = 0;
  intervalMs = 0;
  maxCbCnt = 0;
  cbCnt = 0;
  mode = TE_MODE_INACTIVE;
}

TimeEvent::TimeEvent(unsigned long long s, time_event_cb c)
{
  startMs = s;
  cb = c;
  maxCbCnt = 1;
}

TimeEvent::TimeEvent(unsigned long long s, time_event_cb c, unsigned long long i)
{
  startMs = s;
  intervalMs = i;
  cb = c;
  maxCbCnt = MAX_CB_CNT_INF;
}

TimeEvent::TimeEvent(unsigned long long s, time_event_cb c, unsigned long long i, int m)
{
  startMs = s;
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
}

TimeEvent::TimeEvent(unsigned long long s, time_event_cb c, unsigned long long i, int m, int m2)
{
  startMs = s;
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
  mode = m2;
}

unsigned long long TimeEvent::getStartMs()
{
  return startMs;
}

unsigned long long TimeEvent::getIntervalMs()
{
  return intervalMs;
}

time_event_cb TimeEvent::getCb()
{
  return cb;
}

int TimeEvent::getMaxCbCnt()
{
  return maxCbCnt;
}

int TimeEvent::getCbCnt()
{
  return cbCnt;
}

unsigned long long TimeEvent::getLastCbMs()
{
  return intervalMs * cbCnt + startMs;
}

unsigned long long TimeEvent::getNextCbMs()
{
  return getLastCbMs() + intervalMs;
}

bool TimeEvent::isTime(unsigned long long curMs)
{
  return curMs >= getNextCbMs() && (maxCbCnt < 0 || cbCnt < maxCbCnt);
}

bool TimeEvent::cbIfTimeAndActive(unsigned long long curMs)
{
  if (mode == TE_MODE_ACTIVE && isTime(curMs) && intervalMs && cb(TECBArg(curMs, startMs, intervalMs, cbCnt, maxCbCnt))) // Checking intervalMs here since default constructor doesnt even define cb; might need stub there to be safe
  {
    unsigned long long elapsedMs = curMs - startMs;
    cbCnt = elapsedMs / intervalMs; // Setting cbCnt to expected value rather than just incrementing
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
