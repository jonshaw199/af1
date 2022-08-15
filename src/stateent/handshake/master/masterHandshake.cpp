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

#include "masterHandshake.h"
#include "messageHandler/messageHandler.h"
#include "util/wifi/wifiUtil.h"
#include "stateManager/stateManager.h"
#include "stateent/base/intervalEvent/intervalEvent.h"

bool handleHandshakes()
{
  MessageHandler::scanForPeers();
  MessageHandler::connectToPeers();
  MessageHandler::sendAllHandshakes();
  return true;
}

void MasterHandshake::setup()
{
  Base::setup();
  WifiUtil::prepareWifi();
  WiFi.mode(WIFI_AP);
  delay(DELAY_PREPARE_WIFI);
  MessageHandler::initEspNow();
  intervalEvents.push_back(IntervalEvent(MS_MASTER_HANDSHAKE_LOOP, handleHandshakes));
}

void MasterHandshake::loop()
{
  Base::loop();

  // Check if handshake has been completed for all slaves
  int numHandshakeComplete = 0;
  for (std::map<int, js_peer_info>::const_iterator it = MessageHandler::getPeerInfoMap().begin(); it != MessageHandler::getPeerInfoMap().end(); it++)
  {
    if (it->second.handshakeResponse)
    {
      numHandshakeComplete++;
    }
  }
  if (numHandshakeComplete >= SLAVE_CNT)
  {
    StateManager::setRequestedState(INITIAL_STATE);
  }
}