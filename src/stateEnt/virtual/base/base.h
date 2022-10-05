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
#include <map>
#include <queue>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <set>
#include <WebSocketClient.h>

#include "intervalEvent/intervalEvent.h"
#include "timeEvent/timeEvent.h"
#include "state/state.h"
#include "message/message.h"
#include "box/box.h"

typedef void (*string_input_handler)();

struct wifi_ap_info
{
  String ssid;
  String pass;
  int staticIP[4];
  int gatewayIP[4];
  int subnetIP[4];
};

typedef struct af1_peer_info
{
  esp_now_peer_info_t espnowPeerInfo;
  bool handshakeRequest;
  bool handshakeResponse;
  AF1Msg lastMsg;
  unsigned long otherTimeSync;
  unsigned long thisTimeSync;
  std::mutex mutex;
} af1_peer_info;

class ws_client_info
{
public:
  String host;
  String path;
  int port;
  String protocol;
  String toString()
  {
    return "ws_client_info: host=" + host + ";path=" + path + ";port=" + String(port) + ";protocol=" + protocol;
  }
  bool operator==(const ws_client_info &other)
  {
    return host == other.host && path == other.path && port == other.port;
  }
  operator bool() const
  {
    return host.length();
  }
};

typedef struct sync_data
{
  unsigned long ms;
} sync_data;

class STArg
{
  const IECBArg iecbArg;

public:
  STArg(IECBArg a);
  IECBArg getIECBArg();
};

class Base
{
  static void onESPNowDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onESPNowDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

  ws_client_info wsClientInfo;

  unsigned long syncStartTime;

protected:
  unsigned long startMs;
  std::map<String, IntervalEvent> intervalEventMap;
  std::map<String, TimeEvent> timeEventMap;
  static void handleInboxMsg(AF1Msg m);
  static void handleOutboxMsg(AF1Msg m);

  static uint8_t macAP[6];
  static uint8_t macSTA[6];

  static Box inbox;
  static Box outbox;

  static HTTPClient httpClient;
  static WiFiMulti wifiMulti;

public:
  Base();
  // Virtual
  virtual void setup();
  virtual void loop();
  virtual bool validateStateChange(int s);
  virtual void preStateChange(int s);
  virtual msg_handler getInboxHandler();
  virtual msg_handler getOutboxHandler();
  virtual String getName();
  virtual void serializeESPNow(AF1Msg &m);
  virtual void deserializeESPNow(AF1Msg &m);
  virtual bool doScanForPeersESPNow();
  virtual bool doConnectToWSServer();
  virtual void doSynced(STArg a);
  virtual bool doSync();
  // Interval Events
  unsigned long getElapsedMs();
  void resetIntervalEvents();
  void activateIntervalEvents();
  void deactivateIntervalEvents();
  void resetTimeEvents();
  void activateTimeEvents();
  void deactivateTimeEvents();

  void setWSClientInfo(ws_client_info w);
  ws_client_info getWSClientInfo();

  // Wifi
  static bool broadcastAP();
  static String macToString(const uint8_t *m);
  static void printMac(const uint8_t *m);
  static uint8_t *getMacSTA();
  static uint8_t *getMacAP();
  static void connectToWifi();
  // Inbox/Outbox
  static const TSQueue<AF1Msg> &getOutbox();
  static const TSQueue<AF1Msg> &getInbox();
  static void handleInboxMessages();
  static void handleOutboxMessages();
  static void setInboxMsgHandler(msg_handler h);
  static void setOutboxMsgHandler(msg_handler h);
  static void pushOutbox(AF1Msg m);
  static void pushInbox(AF1Msg m);
  // ESP-Now
  static int8_t scanForPeersESPNow();
  static void connectToPeers();
  static void initEspNow();
  static bool handleHandshakes();
  static void sendStateChangeMessages(int s);
  static void sendHandshakeRequests(std::set<int> ids);
  static void receiveHandshakeRequest(AF1Msg m);
  static void sendHandshakeResponses(std::set<int> ids);
  static void receiveHandshakeResponse(AF1Msg m);
  static void sendAllHandshakes();
  static void sendMsgESPNow(AF1Msg msg);
  static void sendTimeSyncMsg(std::set<int> ids, bool isResponse = false);
  static void receiveTimeSyncMsg(AF1Msg m);
  static void sendAllTimeSyncMessages();
  static void setTimeSyncMsgTime(AF1Msg &m);
  // Websocket
  static WebSocketClient webSocketClient;
  static WiFiClient client; // Use WiFiClient class to create TCP connections
  static void sendMsgWS(AF1Msg msg);
  static void connectToWS();
  // HTTP
  static DynamicJsonDocument httpFetch(String url);
  static DynamicJsonDocument httpPost(String url, DynamicJsonDocument body);
  // Other
  static void setBuiltinLED(bool on);
  std::map<String, IntervalEvent> &getIntervalEventMap();
  std::map<String, TimeEvent> &getTimeEventMap();
  unsigned long getStartMs();

  static void scheduleSyncStart();
  void setSyncStartTime(unsigned long s);
  unsigned long getSyncStartTime();
};

#endif // STATEENT_VIRTUAL_BASE_BASE_H_