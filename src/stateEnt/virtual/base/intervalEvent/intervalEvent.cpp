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

#include "intervalEvent.h"

IECBArg::IECBArg(String n, unsigned long e, unsigned long c, unsigned long m) : name(n), elapsedMs(e), cbCnt(c), maxCbCnt(m) {}

unsigned long IECBArg::getElapsedMs()
{
  return elapsedMs;
}

unsigned long IECBArg::getCbCnt()
{
  return cbCnt;
}

unsigned long IECBArg::getMaxCbCnt()
{
  return maxCbCnt;
}

IntervalEvent::IntervalEvent()
{
  maxCbCnt = 0;
}

IntervalEvent::IntervalEvent(String n, unsigned long i, interval_event_cb c)
{
  name = n;
  intervalMs = i;
  cb = c;
  maxCbCnt = MAX_CB_CNT_INF;
}

IntervalEvent::IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m)
{
  name = n;
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
}

IntervalEvent::IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m, bool t)
{
  name = n;
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
  temporary = t;
}

IntervalEvent::IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m, bool t, unsigned long c2)
{
  name = n;
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
  temporary = t;
  cbCnt = c2;
}

IntervalEvent::IntervalEvent(String n, unsigned long i, interval_event_cb c, unsigned long m, bool t, unsigned long c2, uint8_t m2)
{
  name = n;
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
  temporary = t;
  cbCnt = c2;
  mode = m2;
}

interval_event_cb IntervalEvent::getCb()
{
  return cb;
}

void IntervalEvent::setCb(interval_event_cb c)
{
  cb = c;
}

unsigned long IntervalEvent::getMaxCbCnt()
{
  return maxCbCnt;
}

void IntervalEvent::setMaxCbCnt(unsigned long c)
{
  maxCbCnt = c;
}

unsigned long IntervalEvent::getLastCbMs()
{
  return intervalMs * cbCnt;
}

unsigned long IntervalEvent::getNextCbMs()
{
  return getLastCbMs() + intervalMs;
}

bool IntervalEvent::getTemporary()
{
  return temporary;
}

bool IntervalEvent::isTime(unsigned long elapsedMs)
{
  return elapsedMs >= getNextCbMs() && (maxCbCnt < 0 || cbCnt < maxCbCnt);
}

bool IntervalEvent::cbIfTimeAndActive(unsigned long elapsedMs)
{
  if (mode == IE_MODE_ACTIVE && isTime(elapsedMs))
  {
    cb(IECBArg(name, elapsedMs, cbCnt, maxCbCnt));
    cbCnt = intervalMs > 0 ? elapsedMs / intervalMs : 0; // Setting cbCnt to expected value rather than just incrementing
    return true;
  }
  return false;
}

void IntervalEvent::setMode(uint8_t m)
{
  mode = m;
}

uint8_t IntervalEvent::getMode()
{
  return mode;
}

void IntervalEvent::reset()
{
  cbCnt = 0;
}

void IntervalEvent::activate()
{
  mode = IE_MODE_ACTIVE;
}

void IntervalEvent::deactivate()
{
  mode = IE_MODE_INACTIVE;
}
