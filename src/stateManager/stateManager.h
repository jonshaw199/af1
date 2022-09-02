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

#include "state/state.h"
#include "message/message.h"
#include "stateEnt/virtual/base/base.h"

typedef void (*string_input_handler)();

struct wifi_ap_info
{
  String ssid;
  String pass;
};

struct ws_server_info
{
  String host;
  String path;
  int port;
};

class StateManager
{
  StateManager();

protected:
  static int curState;
  static int prevState;
  static int requestedState;
  static Base *stateEnt;
  static std::map<int, Base *> stateEntMap;
  static std::map<String, string_input_handler> stringHandlerMap;
  static std::map<int, String> stateNameMap;
  static std::vector<wifi_ap_info> wifiAPs;
  static ws_server_info wsServerInfo;

public:
  static StateManager &getInstance();
  static void setup();
  static void loop();
  static int getCurState();
  static int getPrevState();
  static void setRequestedState(int s);
  static int getRequestedState();
  static void changeToRequestedState();
  static void handleUserInput(String s);
  static String stateToString(int s);
  static void setBuiltinLED(bool on);
  static bool handleStateChange(int s);
  static const std::vector<wifi_ap_info> getWifiAPs();
  static const ws_server_info getWSServerInfo();

  static void registerStateEnt(int i, Base *s, String n);
  static void registerStringHandler(String s, string_input_handler h);
  static void registerWifiAP(String s, String p);
  static void registerWSServer(String h, String p, int p2);
};

#endif // STATEMANAGER_STATEMANAGER_H_
