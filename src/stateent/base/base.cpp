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

#include "base.h"
#include "stateManager/stateManager.h"
#include "messageHandler/messageHandler.h"

Base::Base()
{
}

void Base::setup()
{
  startMs = millis();
}

void Base::loop()
{
  // Interval events
  for (std::vector<IntervalEvent>::iterator it = intervalEvents.begin(); it != intervalEvents.end(); it++)
  {
    it->cbIfTime(getElapsedMs());
  }
}

bool Base::preStateChange(int s)
{
#ifdef MASTER
  int slaveState = s;
  switch (s)
  {
  case STATE_OTA:
    return true; // Slaves already notified
  case STATE_PURG_OTA:
    slaveState = STATE_OTA;
    break;
  case STATE_PURG_RESTART:
    slaveState = STATE_RESTART;
    break;
  }
  MessageHandler::sendStateChangeMessages(s);
#endif

  return true;
}

unsigned long Base::getElapsedMs()
{
  unsigned long nowMs = millis();
  return nowMs - startMs;
}

bool Base::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_CHANGE_STATE:
    Serial.println("State change request message in inbox");
    StateManager::setRequestedState(m.getState());
    break;
  case TYPE_HANDSHAKE_REQUEST:
    Serial.println("Handshake request message in inbox");
    MessageHandler::receiveHandshakeRequest(m);
    MessageHandler::sendHandshakeResponses({m.getSenderID()});
    break;
  case TYPE_HANDSHAKE_RESPONSE:
    Serial.println("Handshake response message in inbox");
    MessageHandler::receiveHandshakeResponse(m);
    break;
  }

#ifndef MASTER
  if (m.getState() != StateManager::getCurState() && m.getState() != StateManager::getRequestedState())
  {
    Serial.println("Implicit state change to " + StateManager::stateToString(m.getState()));
    StateManager::setRequestedState(m.getState());
  }
#endif

  return true;
}

bool Base::handleOutboxMsg(JSMessage m)
{
  MessageHandler::sendMsg(m);
  return true;
}

void Base::setInboxMessageHandler()
{
  MessageHandler::setInboxMsgHandler(handleInboxMsg);
}

void Base::setOutboxMessageHandler()
{
  MessageHandler::setOutboxMsgHandler(handleOutboxMsg);
}
