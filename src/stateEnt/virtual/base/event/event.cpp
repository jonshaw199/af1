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

#include "event.h"
#include "stateEnt/virtual/base/base.h"

ECBArg::ECBArg(unsigned long e, unsigned long i, unsigned long s, start_time_type s2, unsigned long c, unsigned long m, String n, bool t)
    : name(n), curTime(e), cbCnt(c), maxCbCnt(m), intervalTime(i), startTime(s), startTimeType(s2), temporary(t) {}

Event::Event() : name(""), mode(MODE_INACTIVE)
{
  cb = [](ECBArg a)
  { Serial.println("No event cb provided"); };
}

Event::Event(String n, event_cb c, bool t, unsigned long i, unsigned long m, unsigned long s, start_time_type s2, event_mode m2, unsigned long c2)
    : name(n), cb(c), temporary(t), intervalTime(i), maxCbCnt(m), startTime(s), startTimeType(s2), mode(m2), cbCnt(c2) {}

unsigned long Event::getIntervalTime()
{
  return intervalTime;
}

void Event::setIntervalTime(unsigned long t)
{
  intervalTime = t;
}

void Event::setIntervalTime(unsigned long t, unsigned long c)
{
  intervalTime = t;
  cbCnt = t > 0 ? c / t : 0;
}

unsigned long Event::getCbCnt()
{
  return cbCnt;
}

void Event::setCbCnt(unsigned long c)
{
  cbCnt = c;
}

String Event::getName()
{
  return name;
}

void Event::setName(String n)
{
  name = n;
}

unsigned long Event::getMaxCbCnt()
{
  return maxCbCnt;
}

void Event::setMaxCbCnt(unsigned long c)
{
  maxCbCnt = c;
}

unsigned long Event::getStartTime()
{
  return startTime;
}

void Event::setStartTime(unsigned long t)
{
  startTime = t;
}

unsigned long Event::getLastCbTime()
{
  return intervalTime * cbCnt + getStartTime();
}

unsigned long Event::getNextCbTime()
{
  return intervalTime ? getLastCbTime() + intervalTime : getStartTime();
}

bool Event::isTime(unsigned long curTime)
{
  return curTime >= getNextCbTime() && (!maxCbCnt || cbCnt < maxCbCnt);
}

bool Event::cbIfTimeAndActive(unsigned long curTime)
{
  if (mode == MODE_ACTIVE && isTime(curTime))
  {
    cb(ECBArg(curTime, intervalTime, startTime, startTimeType, cbCnt, maxCbCnt, name, temporary));
    cbCnt = intervalTime > 0 ? curTime / intervalTime : 0; // Setting cbCnt to expected value rather than just incrementing; don't divide by 0
    return true;
  }
  return false;
}

bool Event::cbIfTimeAndActive()
{
  unsigned long t;
  switch (startTimeType)
  {
  case START_EPOCH_SEC:
    t = Base::timeClient.isTimeSet() ? Base::timeClient.getEpochTime() : (Base::getCurStateEnt()->getElapsedMs() / 1000UL);
    break;
  case START_STATE_MS:
    t = Base::getCurStateEnt()->getElapsedMs();
    break;
  default:
    t = millis();
  }
  return cbIfTimeAndActive(t);
}

bool Event::getTemporary()
{
  return temporary;
}

void Event::setMode(event_mode m)
{
  mode = m;
}

event_mode Event::getMode()
{
  return mode;
}

void Event::reset()
{
  cbCnt = 0;
}

void Event::activate()
{
  mode = MODE_ACTIVE;
}

void Event::deactivate()
{
  mode = MODE_INACTIVE;
}

event_cb Event::getCb()
{
  return cb;
}

void Event::setCb(event_cb c)
{
  cb = c;
}
