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
  resetIntervalEvents();
  startMs = millis();
}

void Base::loop()
{
  inbox.handleMessages();
  outbox.handleMessages();

  // Interval events
  for (std::vector<IntervalEvent>::iterator it = intervalEvents.begin(); it != intervalEvents.end(); it++)
  {
    it->cbIfTime(getElapsedMs());
  }
}

bool Base::preStateChange(int s)
{
  return true;
}

unsigned long Base::getElapsedMs()
{
  unsigned long nowMs = millis();
  return nowMs - startMs;
}

bool Base::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_CHANGE_STATE:
    Serial.println("State change request message in inbox");
    StateManager::setRequestedState(m.getState());
    return true;
  }

#if IMPLICIT_STATE_CHANGE
#ifndef MASTER
  if (m.getState() != StateManager::getCurState() && m.getState() != StateManager::getRequestedState())
  {
    Serial.println("Implicit state change to " + StateManager::stateToString(m.getState()));
    StateManager::setRequestedState(m.getState());
    return true;
  }
#endif
#endif

  Serial.println("Inbox message going to the abyss");
  return false;
}

bool Base::handleOutboxMsg(JSMessage m)
{
  Serial.println("Outbox message going to the abyss");
  return false;
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
  for (std::vector<IntervalEvent>::iterator it = intervalEvents.begin(); it != intervalEvents.end(); it++)
  {
    it->reset();
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
  String Prefix = STRINGIFY(DEVICE_PREFIX);
  String id = String(JS_ID);
  String SSID = Prefix + id;
  String Password = STRINGIFY(DEVICE_AP_PASS);
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

#if JS_IP_A
  // Set your Static IP address
  IPAddress local_IP(JS_IP_A, JS_IP_B, JS_IP_C, JS_IP_D);
  // Set your Gateway IP address
  IPAddress gateway(GATEWAY_A, GATEWAY_B, GATEWAY_C, GATEWAY_D);
  IPAddress subnet(SUBNET_A, SUBNET_B, SUBNET_C, SUBNET_D);
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet))
  {
    Serial.println("Static IP Failed to configure");
  }
  else
  {
    Serial.println("Static IP config success");
  }
#endif

  // Connect to Wi-Fi network with SSID and password
  for (wifi_ap_info i : StateManager::getWifiAPs())
  {
    wifiMulti.addAP(i.ssid.c_str(), i.pass.c_str());
  }
  if (wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println("Wifi connection successful");
  }
}

/*
  From MessageHandler
*/

const TSQueue<JSMessage> &Base::getOutbox()
{
  return outbox;
}

const TSQueue<JSMessage> &Base::getInbox()
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

void Base::pushOutbox(JSMessage m)
{
  outbox.enqueue(m);
}

void Base::pushInbox(JSMessage m)
{
  inbox.enqueue(m);
}

/*
  New
*/

DynamicJsonDocument Base::httpFetch(String url)
{
  DynamicJsonDocument result(1024);
  connectToWifi();
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
  connectToWifi();
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
