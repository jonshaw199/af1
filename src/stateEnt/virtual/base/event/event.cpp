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

#include "event.h"

Event::Event() : name(""), cbCnt(0), intervalMs(0)
{
}

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
