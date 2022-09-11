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

#ifndef STATEENT_PURG_PURG_H_
#define STATEENT_PURG_PURG_H_

#include <Arduino.h>
#include "stateManager/stateManager.h"
#include "state/state.h"
#include "pre.h"

template <class T>
class Purg : public T
{
  unsigned long purgMs;
  int next;

public:
  Purg()
  {
    purgMs = MS_PURG_DEFAULT;
    next = STATE_IDLE_BASE;
  }
  Purg(int s)
  {
    purgMs = MS_PURG_DEFAULT;
    next = s;
  }
  void loop()
  {
    T::loop();

    if (T::getElapsedMs() > purgMs)
    {
      Serial.println("Purgatory over");
      StateManager::setRequestedState(next);
    }
  }
  void setPurgMs(unsigned long ms)
  {
    purgMs = ms;
  }
  void setNext(int s)
  {
    next = s;
  }
  String getName()
  {
    return "STATE_PURG_" + String(next);
  }
};

#endif // STATEENT_PURG_PURG_H_