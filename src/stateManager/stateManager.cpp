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

#include "stateManager.h"
#include "stateEnt/ota/ota.h"
#include "stateEnt/base/base.h"
#include "stateEnt/restart/restart.h"
#include "stateEnt/idle/idle.h"
#include "stateEnt/handshake/master/masterHandshake.h"
#include "stateEnt/handshake/slave/slaveHandshake.h"
#include "stateEnt/init/init.h"
#include "stateEnt/purg/purg.h"

StateManager &StateManager::getInstance()
{
  static StateManager instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
  return instance;
}

int StateManager::getCurState()
{
  return getInstance().curState;
}

int StateManager::getPrevState()
{
  return getInstance().prevState;
}

void setStatePurgOTA()
{
  StateManager::getInstance().setRequestedState(STATE_PURG_OTA);
}

void setStateIdle()
{
  StateManager::getInstance().setRequestedState(STATE_IDLE);
}

void setStatePurgRestart()
{
  StateManager::getInstance().setRequestedState(STATE_PURG_RESTART);
}

void setStateHandshake()
{
  StateManager::getInstance().setRequestedState(STATE_HANDSHAKE);
}

void setStateInit()
{
  StateManager::getInstance().setRequestedState(STATE_INIT);
}

StateManager::StateManager()
{
  stateEntMap[STATE_INIT] = new Init();
  stateEntMap[STATE_OTA] = new OTA();
#ifdef MASTER
  stateEntMap[STATE_HANDSHAKE] = new MasterHandshake();
#else
  stateEntMap[STATE_HANDSHAKE] = new SlaveHandshake();
#endif
  stateEntMap[STATE_RESTART] = new Restart();
  stateEntMap[STATE_IDLE] = new Idle();
  stateEntMap[STATE_PURG_OTA] = new Purg(STATE_OTA);
  stateEntMap[STATE_PURG_RESTART] = new Purg(STATE_RESTART);

  stringHandlerMap["s"] = setStateInit;
  stringHandlerMap["o"] = setStatePurgOTA;
  stringHandlerMap["h"] = setStateHandshake;
  stringHandlerMap["k"] = setStatePurgRestart;
  stringHandlerMap["i"] = setStateIdle;

  stateNameMap[STATE_NONE] = "STATE_NONE";
  stateNameMap[STATE_INIT] = "STATE_INIT";
  stateNameMap[STATE_PURG_OTA] = "STATE_PURG_OTA";
  stateNameMap[STATE_OTA] = "STATE_OTA";
  stateNameMap[STATE_IDLE] = "STATE_IDLE";
  stateNameMap[STATE_PURG_RESTART] = "STATE_PURG_RESTART";
  stateNameMap[STATE_RESTART] = "STATE_RESTART";
  stateNameMap[STATE_HANDSHAKE] = "STATE_HANDSHAKE";
}

void StateManager::init()
{
  int s = STATE_INIT;
  getInstance().curState = s;
  getInstance().requestedState = s;
  getInstance().webServer = new AsyncWebServer(80);

  handleStateChange(s); // Let Init stateEnt handle everything
}

void StateManager::setRequestedState(int s)
{
  getInstance().requestedState = s;
}

int StateManager::getRequestedState()
{
  return getInstance().requestedState;
}

void StateManager::changeToRequestedState()
{
  getInstance().prevState = getInstance().curState;
  getInstance().curState = getInstance().requestedState;
}

void StateManager::handleUserInput(String s)
{
  if (getInstance().stringHandlerMap.count(s))
  {
    getInstance().stringHandlerMap[s]();
  }
  else
  {
    Serial.println("String input not recognized");
  }
}

void StateManager::recvWebSerialMsg(uint8_t *data, size_t len)
{
  WebSerial.println("Received Data...");
  String d = "";
  for (int i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  WebSerial.println(d);
  handleUserInput(d);
}

void StateManager::initWebSerial()
{
  Serial.println("Initializing WebSerial");
  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(getInstance().webServer);
  WebSerial.msgCallback(recvWebSerialMsg);
  getInstance().webServer->begin();
}

void StateManager::deinitWebSerial()
{
  Serial.println("Deinitializing WebSerial");
  getInstance().webServer->end();
}

String StateManager::stateToString(int s)
{
  return getInstance().stateNameMap[s];
}

void StateManager::setBuiltinLED(bool on)
{
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, on);
#endif
}

bool StateManager::handleStateChange(int s)
{
  getInstance().stateEnt = getInstance().stateEntMap[s];
  getInstance().stateEnt->setup();
  getInstance().stateEnt->setInboxMessageHandler();
  getInstance().stateEnt->setOutboxMessageHandler();
  return true;
}

void StateManager::loop()
{
  // Handling this first instead of last; allows us to use init.loop() if we need it before switching to the requested state (or maybe we don't want to request a new state during init at all?)
  getInstance().stateEnt->loop();

  // Check if state change requested and proceed if stateEnt->preStateChange() says its ok
  int curState = getCurState();
  int requestedState = getRequestedState();
  if (curState != requestedState)
  {
    Serial.println("Requested state change: " + StateManager::stateToString(requestedState));
    if (getInstance().stateEnt->preStateChange(requestedState))
    {
      StateManager::changeToRequestedState();
      StateManager::handleStateChange(requestedState);
      Serial.println("State change complete");
    }
    else
    {
      Serial.println("State change rejected by preStateChange");
    }
  }
  // Handling user input
  if (Serial.available() > 0)
  {
    String s = Serial.readString();
    handleUserInput(s);
  }
}

void StateManager::registerStateEnt(int i, Base *s, String n)
{
  getInstance().stateEntMap[i] = s;
  getInstance().stateNameMap[i] = n;
}

void StateManager::registerStringHandler(String s, string_input_handler h)
{
  getInstance().stringHandlerMap[s] = h;
}
