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
  resetIntervalEvents();
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
    return true;
  }

#ifndef MASTER
  if (m.getState() != StateManager::getCurState() && m.getState() != StateManager::getRequestedState())
  {
    Serial.println("Implicit state change to " + StateManager::stateToString(m.getState()));
    StateManager::setRequestedState(m.getState());
    return true;
  }
#endif

  return false;
}

bool Base::handleOutboxMsg(JSMessage m)
{
  return false;
}

void Base::setInboxMessageHandler()
{
  MessageHandler::setInboxMsgHandler(handleInboxMsg);
}

void Base::setOutboxMessageHandler()
{
  MessageHandler::setOutboxMsgHandler(handleOutboxMsg);
}

void Base::resetIntervalEvents()
{
  for (std::vector<IntervalEvent>::iterator it = intervalEvents.begin(); it != intervalEvents.end(); it++)
  {
    it->reset();
  }
}
