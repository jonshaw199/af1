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

#include "masterHandshake.h"
#include "stateManager/stateManager.h"
#include "stateEnt/virtual/base/intervalEvent/intervalEvent.h"
#include "pre.h"

bool handleHandshakesInternal(IECBArg a)
{
  ESPNowEnt::handleHandshakes();
  return true;
}

void MasterHandshake::setup()
{
  Base::setup();
  prepareWifi();
  setAPMode();
  initEspNow();
  intervalEvents.push_back(IntervalEvent(MS_MASTER_HANDSHAKE_LOOP, handleHandshakesInternal));
}

void MasterHandshake::loop()
{
  Base::loop();

  // Check if handshake has been completed for all slaves
  int numHandshakeComplete = 0;
  for (std::map<int, js_peer_info>::const_iterator it = getPeerInfoMap().begin(); it != getPeerInfoMap().end(); it++)
  {
    if (it->second.handshakeResponse)
    {
      numHandshakeComplete++;
    }
  }
  if (numHandshakeComplete >= SLAVE_CNT)
  {
    StateManager::setRequestedState(StateManager::getStateAfterHandshake());
  }
}