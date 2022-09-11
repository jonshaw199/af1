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

WSEnt::WSEnt() : Base()
{
  setWSClientInfo("", "", -1, "");
}

WSEnt::WSEnt(String host, String path, int port) : Base()
{
  setWSClientInfo(host, path, port, "");
}

void WSEnt::setup()
{
  Base::setup();

  connectToWifi();
  if (client)
  {
    Serial.println("Already connected to websocket");
  }
  else
  {
    connectToWS();
    handshakeWS();
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
      Serial.print(".");
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, data);
      pushInbox(doc);
    }
  }
  else
  {
    Serial.println("Client disconnected, switching to fallback state");
    StateManager::setRequestedState(STATE_WS_FALLBACK);
  }

  // wait to fully let the client disconnect
  // delay(3000);
}

void WSEnt::connectToWS()
{
  // Connect to the websocket server
  if (client.connect(wsClientInfo.host.c_str(), wsClientInfo.port))
  {
    Serial.println("Websocket connected");
  }
  else
  {
    Serial.println("Websocket connection failed, switching to fallback state");
    StateManager::setRequestedState(STATE_WS_FALLBACK);
  }
}

void WSEnt::handshakeWS()
{
  int lenH = wsClientInfo.host.length() + 1;
  int lenP = wsClientInfo.path.length() + 1;
  int lenPr = wsClientInfo.protocol.length() + 1;
  char h[lenH];
  char p[lenP];
  char pr[lenPr];
  wsClientInfo.host.toCharArray(h, lenH);
  wsClientInfo.path.toCharArray(p, lenP);
  wsClientInfo.protocol.toCharArray(pr, lenPr);
  webSocketClient.host = h;
  webSocketClient.path = p;
  webSocketClient.protocol = pr;

  if (webSocketClient.handshake(client))
  {
    Serial.println("Handshake successful");
  }
  else
  {
    Serial.println("Handshake failed, switching to fallback state");
    StateManager::setRequestedState(STATE_WS_FALLBACK);
  }
}

void WSEnt::overrideOutboxHandler()
{
  setOutboxMsgHandler(handleOutboxMsg);
}

void WSEnt::handleOutboxMsg(AF1Msg m)
{
  Base::handleOutboxMsg(m);
  if (client)
  {
    String s;
    serializeJson(m.getJson(), s);
#if PRINT_MSG_SEND
    Serial.println("Sending websocket message");
#endif
    webSocketClient.sendData(s);
  }
  else
  {
#if PRINT_MSG_SEND
    Serial.println("Websocket client not connected; unable to send message");
#endif
  }
}

bool WSEnt::validateStateChange(int s)
{
  if (s == STATE_NONE)
  {
    Serial.println("WSEnt::validateStateChange: Uh oh, idling...");
    StateManager::setRequestedState(STATE_IDLE_WS);
  }
  return true;
}

void WSEnt::setWSClientInfo(String host, String path, int port, String protocol)
{
  wsClientInfo.host = host;
  wsClientInfo.path = path;
  wsClientInfo.port = port;
  wsClientInfo.protocol = protocol;
}
