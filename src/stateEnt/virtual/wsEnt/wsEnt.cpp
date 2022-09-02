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

#include <ArduinoJson.h>

#include "wsEnt.h"
#include "pre.h"
#include "stateManager/stateManager.h"

WebSocketClient WSEnt::webSocketClient;
// Use WiFiClient class to create TCP connections
WiFiClient WSEnt::client;

void WSEnt::setup()
{
  Base::setup();

  connectToWifi();
  connectToWS();

  const char *h = StateManager::getWSServerInfo().host.c_str();
  const char *p = StateManager::getWSServerInfo().path.c_str();
  char h2[sizeof(h) / sizeof(char)];
  char p2[sizeof(p) / sizeof(char)];
  memcpy(&h2, h, sizeof(h2));
  memcpy(&p2, p, sizeof(p2));
  webSocketClient.host = h2;
  webSocketClient.path = p2;

  if (webSocketClient.handshake(client))
  {
    Serial.println("Handshake successful");
  }
  else
  {
    Serial.println("Handshake failed.");
  }
}

void WSEnt::loop()
{
  Base::loop();

  String data;

  if (client)
  {
    webSocketClient.getData(data);
    if (data.length() > 0)
    {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, data);
      pushInbox(doc);
    }
  }
  else
  {
    Serial.println("Client disconnected.");
  }

  // wait to fully let the client disconnect
  // delay(3000);
}

void WSEnt::connectToWS()
{
  // Connect to the websocket server
  if (client.connect(StateManager::getWSServerInfo().host.c_str(), StateManager::getWSServerInfo().port))
  {
    Serial.println("Websocket connected");
  }
  else
  {
    Serial.println("Websocket connection failed.");
  }
}
