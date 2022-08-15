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

#ifndef STATEMANAGER_STATEMANAGER_H_
#define STATEMANAGER_STATEMANAGER_H_

#include <queue>
#include <map>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

#include "state/state.h"
#include "message/message.h"
#include "stateent/base/base.h"

typedef void (*string_input_handler)();

class StateManager
{
  StateManager(); // constructor
  int curState;
  int prevState;
  int requestedState;
  AsyncWebServer *webServer;
  Base *stateEnt;
  std::map<int, Base *> stateEntMap;
  std::map<String, string_input_handler> stringHandlerMap;
  std::map<int, String> stateNameMap;

protected:
  static void recvWebSerialMsg(uint8_t *data, size_t len);

public:
  static StateManager &getInstance();
  static int getCurState();
  static int getPrevState();
  static void setRequestedState(int s);
  static int getRequestedState();
  static void changeToRequestedState();
  static void handleUserInput(String s);
  static void initWebSerial();
  static void deinitWebSerial();
  static String stateToString(int s);
  static void setBuiltinLED(bool on);
  static bool handleStateChange(int s);
  static void init();
  static void loop();
  static void registerStateEnt(int i, Base *s, String n);
  static void registerStringHandler(String s, string_input_handler h);
};

#endif // STATEMANAGER_STATEMANAGER_H_
