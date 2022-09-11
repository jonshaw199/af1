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

#ifndef STATEENT_VIRTUAL_WSENT_WSENT_H_
#define STATEENT_VIRTUAL_WSENT_WSENT_H_

#include <WebSocketClient.h>

#include "stateEnt/virtual/base/base.h"

struct ws_client_info
{
  String host;
  String path;
  String protocol;
  int port;
};

class WSEnt : public Base
{
  ws_client_info wsClientInfo;
  void connectToWS();
  void handshakeWS();

protected:
  static void handleOutboxMsg(AF1Msg m);

public:
  static WebSocketClient webSocketClient;
  // Use WiFiClient class to create TCP connections
  static WiFiClient client;
  WSEnt();
  WSEnt(String host, String path, int port);
  WSEnt(String host, String path, int port, String protocol);
  void setup();
  void loop();
  void overrideOutboxHandler();
  bool validateStateChange(int s);
  void setWSClientInfo(String host, String path, int port, String protocol);
};

#endif