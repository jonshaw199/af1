/*
  AF1 - An Arduino extension framework
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

#include "intervalEvent.h"

IntervalEvent::IntervalEvent()
{
  intervalMs = 0;
  maxCbCnt = 0;
}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c)
{
  intervalMs = i;
  cb = c;
  maxCbCnt = -1;
}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c, int m)
{
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
}

unsigned long IntervalEvent::getIntervalMs()
{
  return intervalMs;
}

interval_event_cb IntervalEvent::getCb()
{
  return cb;
}

int IntervalEvent::getMaxCbCnt()
{
  return maxCbCnt;
}

int IntervalEvent::getCbCnt()
{
  return cbCnt;
}

unsigned long IntervalEvent::getLastCbMs()
{
  return intervalMs * cbCnt;
}

unsigned long IntervalEvent::getNextCbMs()
{
  return getLastCbMs() + intervalMs;
}

bool IntervalEvent::isTime(unsigned long elapsedMs)
{
  return elapsedMs >= getNextCbMs() && (maxCbCnt < 0 || cbCnt < maxCbCnt);
}

bool IntervalEvent::cbIfTime(unsigned long elapsedMs)
{
  if (isTime(elapsedMs) && intervalMs && cb()) // Checking intervalMs here since default constructor doesnt even define cb; might need stub there to be safe
  {
    cbCnt++;
    return true;
  }
  return false;
}

void IntervalEvent::reset()
{
  cbCnt = 0;
}
