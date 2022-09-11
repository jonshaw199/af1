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

#ifndef STATEMANAGER_STATEMANAGER_H_
#define STATEMANAGER_STATEMANAGER_H_

#include <queue>
#include <map>
#include <vector>
#include <Arduino.h>
#include <esp_now.h>
#include <mutex>

#include "state/state.h"
#include "message/message.h"
#include "stateEnt/virtual/base/base.h"

class StateManager
{
  StateManager();

protected:
  static int curState;
  static int prevState;
  static int requestedState;
  static int initialState;
  static int stateAfterHandshake;
  static int deviceID;
  static Base *stateEnt;
  static std::map<int, Base *> stateEntMap;
  static std::map<String, string_input_handler> stringHandlerMap;
  static std::vector<wifi_ap_info> wifiAPs;

public:
  static StateManager &getInstance();
  static void setup(int id);
  static void loop();
  static int getCurState();
  static int getPrevState();
  static void setRequestedState(int s);
  static int getRequestedState();
  static void handleUserInput(String s);
  static String stateToString(int s);
  static bool handleStateChange(int s);
  static const std::vector<wifi_ap_info> getWifiAPs();
  static void setInitialState(int s);
  static int getInitialState();
  static void setStateAfterHandshake(int s);
  static int getStateAfterHandshake();
  static void setPurgNext(int p, int n);
  static const std::map<int, Base *> &getStateEntMap();
  static int getDeviceID();
  static void setDefaultWSClientInfo(ws_client_info w);
  static std::map<int, af1_peer_info> &getPeerInfoMap();
  static std::map<String, int> &getMacToIDMap();
  static std::set<int> getPeerIDs();

  static void registerStateEnt(int i, Base *s, String n);
  static void registerStateEnt(int i, Base *s, String n, ws_client_info w);
  static void registerStringHandler(String s, string_input_handler h);
  static void registerWifiAP(String s, String p);
  static void registerWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd);
};

#endif // STATEMANAGER_STATEMANAGER_H_
