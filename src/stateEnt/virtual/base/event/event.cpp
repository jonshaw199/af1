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

ECBArg::ECBArg(unsigned long e, unsigned long i, unsigned long s, start_time_type s2, unsigned long c, unsigned long m, String n, event_type t)
    : name(n), curTime(e), cbCnt(c), maxCbCnt(m), intervalTime(i), startTime(s), startTimeType(s2), type(t) {}

AF1Event::AF1Event() : name(""), mode(MODE_INACTIVE), type(EVENT_TYPE_TEMP)
{
  cb = [](ECBArg a)
  { Serial.println("No event cb provided"); };
}

AF1Event::AF1Event(String n, event_cb c, event_type t, unsigned long i, unsigned long m, unsigned long s, start_time_type s2, event_mode m2, unsigned long c2)
    : name(n), cb(c), type(t), intervalTime(i), maxCbCnt(m), startTime(s), startTimeType(s2), mode(m2), cbCnt(c2) {}

unsigned long AF1Event::getIntervalTime()
{
  return intervalTime;
}

void AF1Event::setIntervalTime(unsigned long t)
{
  intervalTime = t;
}

void AF1Event::setIntervalTime(unsigned long t, unsigned long c)
{
  intervalTime = t;
  cbCnt = t > 0 ? c / t : 0;
}

unsigned long AF1Event::getCbCnt()
{
  return cbCnt;
}

void AF1Event::setCbCnt(unsigned long c)
{
  cbCnt = c;
}

String AF1Event::getName()
{
  return name;
}

void AF1Event::setName(String n)
{
  name = n;
}

unsigned long AF1Event::getMaxCbCnt()
{
  return maxCbCnt;
}

void AF1Event::setMaxCbCnt(unsigned long c)
{
  maxCbCnt = c;
}

unsigned long AF1Event::getStartTime()
{
  return startTime;
}

void AF1Event::setStartTime(unsigned long t)
{
  startTime = t;
}

start_time_type AF1Event::getStartTimeType()
{
  return startTimeType;
}

unsigned long AF1Event::getLastCbTime()
{
  return intervalTime * cbCnt + getStartTime();
}

unsigned long AF1Event::getNextCbTime()
{
  return intervalTime ? getLastCbTime() + intervalTime : getStartTime();
}

bool AF1Event::isTime(unsigned long curTime)
{
  return curTime >= getNextCbTime() && (!maxCbCnt || cbCnt < maxCbCnt);
}

bool AF1Event::cbIfTimeAndActive(unsigned long curTime)
{
  if (mode == MODE_ACTIVE && isTime(curTime))
  {
    cb(ECBArg(curTime, intervalTime, startTime, startTimeType, cbCnt, maxCbCnt, name, type));
    cbCnt = intervalTime > 0 ? curTime / intervalTime : startTime ? curTime / startTime
                                                                  : 0; // Setting cbCnt to expected value rather than just incrementing; don't divide by 0
    return true;
  }
  return false;
}

unsigned long AF1Event::getCurTime()
{
  switch (startTimeType)
  {
  case START_EPOCH_SEC:
    return Base::timeClient.isTimeSet() ? Base::timeClient.getEpochTime() : (Base::getCurStateEnt()->getElapsedMs() / 1000UL);
  case START_STATE_MS:
    return Base::getCurStateEnt()->getElapsedMs();
  case START_DEVICE_MS:
    return millis();
  default:
    return 0;
  }
}

bool AF1Event::cbIfTimeAndActive()
{
  return cbIfTimeAndActive(getCurTime());
}

event_type AF1Event::getType()
{
  return type;
}

void AF1Event::setMode(event_mode m)
{
  mode = m;
}

event_mode AF1Event::getMode()
{
  return mode;
}

void AF1Event::reset()
{
  cbCnt = 0;
}

void AF1Event::activate()
{
  mode = MODE_ACTIVE;
}

void AF1Event::deactivate()
{
  mode = MODE_INACTIVE;
}

event_cb AF1Event::getCb()
{
  return cb;
}

void AF1Event::setCb(event_cb c)
{
  cb = c;
}
