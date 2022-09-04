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

#ifndef STATEENT_VIRTUAL_BASE_BASE_H_
#define STATEENT_VIRTUAL_BASE_BASE_H_

#include <Arduino.h>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <mutex>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "intervalEvent/intervalEvent.h"
#include "state/state.h"
#include "message/message.h"
#include "box/box.h"

class Base
{
protected:
  unsigned long startMs;
  std::map<String, IntervalEvent> intervalEventMap;
  static bool handleInboxMsg(JSMessage m);
  static bool handleOutboxMsg(JSMessage m);
  // From WifiHandler
  static uint8_t macAP[6];
  static uint8_t macSTA[6];
  // From MessageHandler
  static Box inbox;
  static Box outbox;
  // New
  static HTTPClient httpClient;
  static WiFiMulti wifiMulti;

public:
  Base();
  virtual void setup();
  virtual void loop();
  virtual bool validateStateChange(int s);
  virtual void preStateChange(int s);
  virtual void overrideInboxHandler();
  virtual void overrideOutboxHandler();
  unsigned long getElapsedMs();
  void resetIntervalEvents();
  void activateIntervalEvents();
  void deactivateIntervalEvents();
  // From WifiHandler
  static void prepareWifi();
  static void setSTAMode();
  static void setAPMode();
  static bool broadcastAP();
  static String macToString(const uint8_t *m);
  static void printMac(const uint8_t *m);
  static uint8_t *getMacSTA();
  static uint8_t *getMacAP();
  static void connectToWifi();
  // From MessageHandler
  static const TSQueue<JSMessage> &getOutbox(); // Read only
  static const TSQueue<JSMessage> &getInbox();  // Read only
  static void handleInboxMessages();
  static void handleOutboxMessages();
  static void setInboxMsgHandler(msg_handler h);
  static void setOutboxMsgHandler(msg_handler h);
  static void pushOutbox(JSMessage m);
  static void pushInbox(JSMessage m);
  // New
  static DynamicJsonDocument httpFetch(String url);
  static DynamicJsonDocument httpPost(String url, DynamicJsonDocument body);
};

#endif // STATEENT_VIRTUAL_BASE_BASE_H_