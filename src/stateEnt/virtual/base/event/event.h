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

#ifndef STATEENT_BASE_EVENT_H_
#define STATEENT_BASE_EVENT_H_

#include <Arduino.h>

class Event
{
protected:
  unsigned long intervalMs;
  unsigned long cbCnt;
  String name;

public:
  Event();
  unsigned long getIntervalMs();
  void setIntervalMs(unsigned long m);
  void setIntervalMs(unsigned long m, unsigned long elapsedMs); // Updates cbCnt based on elapsedMs
  unsigned long getCbCnt();
  void setCbCnt(unsigned long c);
  String getName();
  void setName(String s);
};

#endif // STATEENT_BASE_EVENT_H_
