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

#include "handshake.h"
#include "stateManager/stateManager.h"
#include "stateEnt/virtual/base/intervalEvent/intervalEvent.h"
#include "pre.h"

bool handleHandshakesInternal(IECBArg a)
{
  Base::handleHandshakes();
  return true;
}

Handshake::Handshake()
{
  // intervalEventMap.insert(std::pair<String, IntervalEvent>("Handshake_1", IntervalEvent(MS_HANDSHAKE_LOOP, handleHandshakesInternal)));
}

void Handshake::setup()
{
  Base::setup();
  // initEspNow();
  broadcastAP();
}

String Handshake::getName()
{
  return "STATE_HANDSHAKE";
}
