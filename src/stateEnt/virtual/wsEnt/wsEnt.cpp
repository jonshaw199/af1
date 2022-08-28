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

#include <ArduinoJson.h>

#include "wsEnt.h"
#include "pre.h"
#include "stateManager/stateManager.h"

WebSocketClient WSEnt::webSocketClient;
// Use WiFiClient class to create TCP connections
WiFiClient WSEnt::client;

char path[] = STRINGIFY(WS_PATH);
char host[] = STRINGIFY(WS_HOST);

void WSEnt::setup()
{
  Base::setup();

  connectToWifi();

  // Connect to the websocket server
  if (client.connect(STRINGIFY(WS_HOST), WS_PORT))
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Connection failed.");
    while (1)
    {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client))
  {
    Serial.println("Handshake successful");
  }
  else
  {
    Serial.println("Handshake failed.");
    while (1)
    {
      // Hang on failure
    }
  }
}

void WSEnt::loop()
{
  Base::loop();

  String data;

  if (client.connected())
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
    while (1)
    {
      // Hang on disconnect.
    }
  }

  // wait to fully let the client disconnect
  // delay(3000);
}
