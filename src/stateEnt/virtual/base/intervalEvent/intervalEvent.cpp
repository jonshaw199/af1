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

IECBArg::IECBArg(unsigned long e, int c, int m) : elapsedMs(e), cbCnt(c), maxCbCnt(m) {}

unsigned long IECBArg::getElapsedMs()
{
  return elapsedMs;
}

int IECBArg::getCbCnt()
{
  return cbCnt;
}

int IECBArg::getMaxCbCnt()
{
  return maxCbCnt;
}

IntervalEvent::IntervalEvent()
{
  intervalMs = 0;
  maxCbCnt = 0;
}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c)
{
  intervalMs = i;
  cb = c;
  maxCbCnt = MAX_CB_CNT_INF;
}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c, int m)
{
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c, int m, bool t)
{
  intervalMs = i;
  cb = c;
  maxCbCnt = m;
  transitory = t;
}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c, int m, bool t, int c2)
    : intervalMs(i), cb(c), maxCbCnt(m), transitory(t), cbCnt(c2) {}

IntervalEvent::IntervalEvent(unsigned long i, interval_event_cb c, int m, bool t, int c2, int m2)
    : intervalMs(i), cb(c), maxCbCnt(m), transitory(t), cbCnt(c2), mode(m2) {}

unsigned long IntervalEvent::getIntervalMs()
{
  return intervalMs;
}

void IntervalEvent::setIntervalMs(unsigned long m)
{
  intervalMs = m;
}

interval_event_cb IntervalEvent::getCb()
{
  return cb;
}

void IntervalEvent::setCb(interval_event_cb c)
{
  cb = c;
}

int IntervalEvent::getMaxCbCnt()
{
  return maxCbCnt;
}

void IntervalEvent::setMaxCbCnt(int c)
{
  maxCbCnt = c;
}

int IntervalEvent::getCbCnt()
{
  return cbCnt;
}

void IntervalEvent::setCbCnt(int c)
{
  cbCnt = c;
}

unsigned long IntervalEvent::getLastCbMs()
{
  return intervalMs * cbCnt;
}

unsigned long IntervalEvent::getNextCbMs()
{
  return getLastCbMs() + intervalMs;
}

bool IntervalEvent::getTransitory()
{
  return transitory;
}

bool IntervalEvent::isTime(unsigned long elapsedMs)
{
  return elapsedMs >= getNextCbMs() && (maxCbCnt < 0 || cbCnt < maxCbCnt);
}

bool IntervalEvent::cbIfTimeAndActive(unsigned long elapsedMs)
{
  if (mode == IE_MODE_ACTIVE && isTime(elapsedMs) && intervalMs && cb(IECBArg(elapsedMs, cbCnt, maxCbCnt))) // Checking intervalMs here since default constructor doesnt even define cb; might need stub there to be safe
  {
    cbCnt = elapsedMs / intervalMs; // Setting cbCnt to expected value rather than just incrementing
    return true;
  }
  return false;
}

void IntervalEvent::setMode(int m)
{
  mode = m;
}

int IntervalEvent::getMode()
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
