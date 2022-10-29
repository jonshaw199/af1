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

ECBArg::ECBArg(unsigned long e, unsigned long i, unsigned long s, unsigned long c, unsigned long m, String n) : name(n), elapsedMs(e), cbCnt(c), maxCbCnt(m), intervalMs(i), startMs(s) {}

unsigned long ECBArg::getElapsedMs()
{
  return elapsedMs;
}

unsigned long ECBArg::getCbCnt()
{
  return cbCnt;
}

unsigned long ECBArg::getMaxCbCnt()
{
  return maxCbCnt;
}

unsigned long ECBArg::getStartMs()
{
  return startMs;
}

unsigned long ECBArg::getIntervalMs()
{
  return intervalMs;
}

Event::Event() : name(""), mode(MODE_INACTIVE)
{
  cb = [](ECBArg a)
  { Serial.println("No event cb provided"); };
}

Event::Event(String n, event_cb c, unsigned long i, unsigned long m, unsigned long s, bool t, uint8_t m2, unsigned long c2)
    : name(n), cb(c), intervalMs(i), maxCbCnt(m), startMs(s), temporary(t), mode(m2), cbCnt(c2) {}

unsigned long Event::getIntervalMs()
{
  return intervalMs;
}

void Event::setIntervalMs(unsigned long m)
{
  intervalMs = m;
}

void Event::setIntervalMs(unsigned long m, unsigned long elapsedMs)
{
  intervalMs = m;
  cbCnt = m > 0 ? elapsedMs / m : 0;
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

unsigned long Event::getLastCbMs()
{
  return intervalMs * cbCnt + startMs;
}

unsigned long Event::getNextCbMs()
{
  return intervalMs ? getLastCbMs() + intervalMs : startMs;
}

bool Event::isTime(unsigned long curMs)
{
  return curMs >= getNextCbMs() && (!maxCbCnt || cbCnt < maxCbCnt);
}

bool Event::cbIfTimeAndActive(unsigned long curMs)
{
  if (mode == MODE_ACTIVE && isTime(curMs))
  {
    cb(ECBArg(curMs, intervalMs, startMs, cbCnt, maxCbCnt, name));
    cbCnt = intervalMs > 0 ? curMs / intervalMs : 0; // Setting cbCnt to expected value rather than just incrementing; don't divide by 0
    return true;
  }
  return false;
}

bool Event::getTemporary()
{
  return temporary;
}

void Event::setMode(uint8_t m)
{
  mode = m;
}

uint8_t Event::getMode()
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
