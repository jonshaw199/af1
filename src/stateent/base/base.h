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

#ifndef STATEENT_BASE_BASE_H_
#define STATEENT_BASE_BASE_H_

#include <Arduino.h>
#include <vector>

#include "intervalEvent/intervalEvent.h"
#include "state/state.h"
#include "message/message.h"

class Base
{
protected:
  unsigned long startMs;
  std::vector<IntervalEvent> intervalEvents;
  static bool handleInboxMsg(JSMessage m);
  static bool handleOutboxMsg(JSMessage m);

public:
  Base();
  virtual void setup();
  virtual void loop();
  virtual bool preStateChange(int s);
  virtual void setInboxMessageHandler();
  virtual void setOutboxMessageHandler();
  unsigned long getElapsedMs();
  void resetIntervalEvents();
};

#endif // STATEENT_BASE_BASE_H_