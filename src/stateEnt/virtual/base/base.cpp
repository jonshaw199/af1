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

#include "base.h"
#include "stateManager/stateManager.h"
#include "pre.h"

uint8_t Base::macAP[6];
uint8_t Base::macSTA[6];

Box Base::inbox;
Box Base::outbox;

HTTPClient Base::httpClient;

WiFiMulti Base::wifiMulti;

Base::Base()
{
}

void Base::setup()
{
  Serial.println("Base::setup()");
  resetIntervalEvents();
  activateIntervalEvents();
  startMs = millis();
  connectToWifi();
}

void Base::loop()
{
  inbox.handleMessages();
  outbox.handleMessages();

  // Handling user input
  if (Serial.available() > 0)
  {
    String s = Serial.readString();
    StateManager::handleUserInput(s);
  }

  // Interval events
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].cbIfTimeAndActive(getElapsedMs());
  }
}

bool Base::validateStateChange(int s)
{
  if (s == STATE_NONE)
  {
    Serial.println("Base::validateStateChange: Uh oh, idling...");
    StateManager::setRequestedState(STATE_IDLE_BASE);
  }
  return true;
}

void Base::preStateChange(int s)
{
  Serial.print("Switching to ");
  Serial.print(StateManager::getStateNameMap().at(s));
  Serial.println(" state now.");
  deactivateIntervalEvents();
}

unsigned long Base::getElapsedMs()
{
  unsigned long nowMs = millis();
  return nowMs - startMs;
}

void Base::handleInboxMsg(AF1Msg m)
{
  Serial.print("<");
#if PRINT_MSG_SEND
  m.print();
#endif

  switch (m.getType())
  {
  case TYPE_CHANGE_STATE:
    Serial.println("State change request message in inbox");
    StateManager::setRequestedState(m.getState());
  }

#if IMPLICIT_STATE_CHANGE
#ifndef MASTER
  if (m.getState() != StateManager::getCurState() && m.getState() != StateManager::getRequestedState())
  {
    Serial.println("Implicit state change to " + StateManager::stateToString(m.getState()));
    StateManager::setRequestedState(m.getState());
  }
#endif
#endif
}

void Base::handleOutboxMsg(AF1Msg m)
{
  Serial.print(">");
#if PRINT_MSG_SEND
  m.print();
#endif
}

void Base::overrideInboxHandler()
{
  setInboxMsgHandler(handleInboxMsg);
}

void Base::overrideOutboxHandler()
{
  setOutboxMsgHandler(handleOutboxMsg);
}

void Base::resetIntervalEvents()
{
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].reset();
  }
}

void Base::activateIntervalEvents()
{
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].activate();
  }
}

void Base::deactivateIntervalEvents()
{
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].deactivate();
  }
}

/*
  From WifiHandler
*/

void Base::prepareWifi()
{
  WiFi.disconnect(true);
  // delay(DELAY_PREPARE_WIFI);
}

void Base::setSTAMode()
{
  Serial.println("Setting wifi mode to STA");
  WiFi.mode(WIFI_STA);
}

void Base::setAPMode()
{
  Serial.println("Setting wifi mode to AP");
  WiFi.mode(WIFI_AP);
}

// Setup access point (aka open wifi network); this is used by slaves so master can find them
bool Base::broadcastAP()
{
  Serial.println("Broadcasting soft AP");
  String Prefix = DEVICE_PREFIX;
  String id = String(StateManager::getDeviceID());
  String SSID = Prefix + id;
  String Password = DEVICE_AP_PASS;
  return WiFi.softAP(SSID.c_str(), Password.c_str(), ESPNOW_CHANNEL, 0);
}

String Base::macToString(const uint8_t *m)
{
  char buffer[33];
  String result = "";
  for (int i = 0; i < 6; i++)
  {
    itoa(m[i], buffer, 16);
    result += buffer;
  }
  return result;
}

void Base::printMac(const uint8_t *m)
{
  Serial.print("{");
  for (int i = 0; i < 6; i++)
  {
    Serial.print("0x");
    Serial.print(m[i], HEX);
    if (i < 5)
      Serial.print(',');
  }
  Serial.println("}");
}

uint8_t *Base::getMacSTA()
{
  return macSTA;
}

uint8_t *Base::getMacAP()
{
  return macAP;
}

void Base::connectToWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Already connected to wifi");
    return;
  }

  /*
    To do: figure out how to handle static IP and WifiMulti properly
    Currently the static IP must be the same for all APs
  */

  std::vector<wifi_ap_info> v = StateManager::getWifiAPs();

  if (v.size() && v[0].staticIP[0] >= 0)
  {
    // Set your Static IP address
    IPAddress local_IP(v[0].staticIP[0], v[0].staticIP[1], v[0].staticIP[2], v[0].staticIP[3]);
    // Set your Gateway IP address
    IPAddress gateway(v[0].gatewayIP[0], v[0].gatewayIP[1], v[0].gatewayIP[2], v[0].gatewayIP[3]);
    IPAddress subnet(v[0].subnetIP[0], v[0].subnetIP[1], v[0].subnetIP[2], v[0].subnetIP[3]);
    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet))
    {
      Serial.println("Static IP Failed to configure");
    }
    else
    {
      Serial.println("Static IP config success");
    }
  }

  // Connect to Wi-Fi network with SSID and password
  for (wifi_ap_info i : v)
  {
    wifiMulti.addAP(i.ssid.c_str(), i.pass.c_str());
  }
  if (wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println("Wifi connection successful");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }
}

/*
  From MessageHandler
*/

const TSQueue<AF1Msg> &Base::getOutbox()
{
  return outbox;
}

const TSQueue<AF1Msg> &Base::getInbox()
{
  return inbox;
}

void Base::setInboxMsgHandler(msg_handler h)
{
  inbox.setMsgHandler(h);
}

void Base::setOutboxMsgHandler(msg_handler h)
{
  outbox.setMsgHandler(h);
}

void Base::pushOutbox(AF1Msg m)
{
  outbox.enqueue(m);
}

void Base::pushInbox(AF1Msg m)
{
  inbox.enqueue(m);
}

/*
  New
*/

DynamicJsonDocument Base::httpFetch(String url)
{
  DynamicJsonDocument result(1024);
  if ((WiFi.status() == WL_CONNECTED))
  {
    httpClient.begin(url);
    Serial.println("Making HTTP GET request...");
    Serial.print("URL: ");
    Serial.println(url);
    int httpCode = httpClient.GET();
    if (httpCode > 0)
    {
      String resBody = httpClient.getString();
      deserializeJson(result, resBody);
      Serial.println(resBody);
    }
    else
    {
      Serial.println("HTTP GET error");
    }
    httpClient.end();
  }
  return result;
}

DynamicJsonDocument Base::httpPost(String url, DynamicJsonDocument body)
{
  DynamicJsonDocument result(1024);
  if ((WiFi.status() == WL_CONNECTED))
  {
    httpClient.begin(url);
    httpClient.addHeader("Content-Type", "application/json");
    String serializedBody;
    serializeJson(body, serializedBody);
    Serial.println("Making HTTP POST request...");
    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("Body: ");
    Serial.println(serializedBody);
    int httpCode = httpClient.POST(serializedBody);
    if (httpCode > 0)
    {
      String resBody = httpClient.getString();
      deserializeJson(result, resBody);
      Serial.println(resBody);
    }
    else
    {
      Serial.print("HTTP POST error: ");
      Serial.println(httpClient.errorToString(httpCode));
    }
    httpClient.end();
  }
  return result;
}
