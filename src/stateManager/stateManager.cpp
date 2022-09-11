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

#include <set>

#include "stateManager.h"
#include "stateEnt/ota/ota.h"
#include "stateEnt/virtual/base/base.h"
#include "stateEnt/restart/restart.h"
#include "stateEnt/handshake/master/masterHandshake.h"
#include "stateEnt/handshake/slave/slaveHandshake.h"
#include "stateEnt/init/init.h"
#include "stateEnt/purg/purg.h"

int StateManager::curState;
int StateManager::prevState;
int StateManager::requestedState;
int StateManager::initialState;
int StateManager::stateAfterHandshake;
int StateManager::deviceID;
Base *StateManager::stateEnt;
std::map<int, Base *> StateManager::stateEntMap;
std::map<String, string_input_handler> StateManager::stringHandlerMap;
std::vector<wifi_ap_info> StateManager::wifiAPs;

// From espnowHandler
static std::map<int, af1_peer_info> peerInfoMap;
static std::map<String, int> macToIDMap;

StateManager::StateManager()
{
  stateEntMap[STATE_INIT] = new Init("STATE_INIT");
  stateEntMap[STATE_OTA] = new OTA("STATE_OTA");
#if MASTER
  stateEntMap[STATE_HANDSHAKE] = new MasterHandshake("STATE_HANDSHAKE");
#else
  stateEntMap[STATE_HANDSHAKE] = new SlaveHandshake("STATE_HANDSHAKE");
#endif
  stateEntMap[STATE_RESTART] = new Restart("STATE_RESTART");
  stateEntMap[STATE_PURG] = new Purg<Base>("STATE_PURG");
  stateEntMap[STATE_IDLE_BASE] = new Base("STATE_IDLE_BASE");

  stringHandlerMap["s"] = []()
  {
    StateManager::setRequestedState(STATE_INIT);
  };
  stringHandlerMap["o"] = []()
  {
    setRequestedState(STATE_OTA);
  };
  stringHandlerMap["h"] = []()
  {
    StateManager::setRequestedState(STATE_HANDSHAKE);
  };
  stringHandlerMap["r"] = []()
  {
    StateManager::setRequestedState(STATE_RESTART);
  };
  stringHandlerMap["i"] = []()
  {
    setRequestedState(STATE_IDLE_BASE);
  };

  initialState = STATE_IDLE_BASE;
  stateAfterHandshake = STATE_IDLE_BASE;
}

StateManager &StateManager::getInstance()
{
  static StateManager instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
  return instance;
}

void StateManager::setup(int id)
{
  getInstance(); // Ensuring instantiation
  deviceID = id;
  int s = STATE_INIT;
  curState = s;
  requestedState = s;
  handleStateChange(s); // Let Init stateEnt handle everything
}

void StateManager::loop()
{
  // Handling this first instead of last; allows us to use init.loop() if we need it before switching to the requested state (or maybe we don't want to request a new state during init at all?)
  stateEnt->loop();

  // Check if state change requested and proceed if stateEnt->validateStateChange() says its ok
  int curState = getCurState();
  int requestedState = getRequestedState();
  if (curState != requestedState)
  {
    Serial.println("Handling state change request: " + StateManager::stateToString(requestedState));
    if (stateEnt->validateStateChange(requestedState))
    {
      stateEnt->preStateChange(StateManager::getRequestedState());
      // Requested state may have changed between last and next function call
      StateManager::handleStateChange(StateManager::getRequestedState());
      Serial.println("State change complete");
    }
    else
    {
      Serial.println("State change rejected by validateStateChange");
    }
  }
}

int StateManager::getCurState()
{
  return curState;
}

int StateManager::getPrevState()
{
  return prevState;
}

void StateManager::setRequestedState(int s)
{
  if (stateEntMap.find(s) == stateEntMap.end())
  {
    Serial.print("Requested state ");
    Serial.print(s);
    Serial.println(" is not recognized, not setting.");
  }
  else
  {
    Serial.print("Setting requested state: ");
    Serial.print(stateEntMap[s]->);
    Serial.print(" (");
    Serial.print(s);
    Serial.println(")");
    requestedState = s;
  }
}

int StateManager::getRequestedState()
{
  return requestedState;
}

void StateManager::handleUserInput(String s)
{
  if (stringHandlerMap.count(s))
  {
    stringHandlerMap[s]();
  }
  else
  {
    Serial.println("String input not recognized");
  }
}

String StateManager::stateToString(int s)
{
  if (stateEntMap.count(s))
  {
    return stateEntMap[s]->getName();
  }
  return "Unknown state";
}

bool StateManager::handleStateChange(int s)
{
  prevState = curState;
  curState = s;

  stateEnt = stateEntMap[s];
  stateEnt->setup();
  stateEnt->overrideInboxHandler();
  stateEnt->overrideOutboxHandler();

  return true;
}

void StateManager::registerStateEnt(int i, Base *s)
{
  stateEntMap[i] = s;
}

void StateManager::registerStringHandler(String s, string_input_handler h)
{
  stringHandlerMap[s] = h;
}

void StateManager::registerWifiAP(String s, String p)
{
  wifi_ap_info i;
  i.ssid = s;
  i.pass = p;
  i.staticIP[0] = -1;
  i.staticIP[1] = -1;
  i.staticIP[2] = -1;
  i.staticIP[3] = -1;
  i.gatewayIP[0] = -1;
  i.gatewayIP[1] = -1;
  i.gatewayIP[2] = -1;
  i.gatewayIP[3] = -1;
  i.subnetIP[0] = -1;
  i.subnetIP[1] = -1;
  i.subnetIP[2] = -1;
  i.subnetIP[3] = -1;
  wifiAPs.push_back(i);
}

void StateManager::registerWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd)
{
  wifi_ap_info i;
  i.ssid = s;
  i.pass = p;
  i.staticIP[0] = a;
  i.staticIP[1] = b;
  i.staticIP[2] = c;
  i.staticIP[3] = d;
  i.gatewayIP[0] = ga;
  i.gatewayIP[1] = gb;
  i.gatewayIP[2] = gc;
  i.gatewayIP[3] = gd;
  i.subnetIP[0] = sa;
  i.subnetIP[1] = sb;
  i.subnetIP[2] = sc;
  i.subnetIP[3] = sd;
  wifiAPs.push_back(i);
}

const std::vector<wifi_ap_info> StateManager::getWifiAPs()
{
  return wifiAPs;
}

void StateManager::setInitialState(int s)
{
  initialState = s;
}

int StateManager::getInitialState()
{
  return initialState;
}

void StateManager::setStateAfterHandshake(int s)
{
  stateAfterHandshake = s;
}

int StateManager::getStateAfterHandshake()
{
  return stateAfterHandshake;
}

void StateManager::setPurgNext(int p, int n)
{
  (static_cast<Purg<Base> *>(stateEntMap[p]))->setNext(n);
}

int StateManager::getDeviceID()
{
  return deviceID;
}

void StateManager::setDefaultWSClientInfo(ws_client_info w)
{
  stateEntMap[STATE_IDLE_BASE]->setWSClientInfo(w);
}

std::set<int> StateManager::getPeerIDs()
{
  std::set<int> result;
  for (std::map<int, af1_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
  {
    result.insert(it->first);
  }
  return result;
}

std::map<int, af1_peer_info> &StateManager::getPeerInfoMap()
{
  return peerInfoMap;
}

std::map<String, int> &StateManager::getMacToIDMap()
{
  return macToIDMap;
}

const std::map<int, Base *> &StateManager::getStateEntMap()
{
  return stateEntMap;
}
