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

#include "event/event.h"
#include "state/state.h"
#include "message/message.h"
#include "box/box.h"

// #include <queue>
// #include <map>
#include <vector>
// #include <Arduino.h>
// #include <esp_now.h>
#include <mutex>
#include <NTPClient.h>
#include <WiFiUdp.h>
// #include "state/state.h"
// #include "message/message.h"
// #include "stateEnt/virtual/base/base.h"
#include "modeEnt/virtual/base/base.h"

class SHArg
{
  String string;
  String value;

public:
  SHArg(String s, String v = "") : string(s), value(v){};
  String getString()
  {
    return string;
  }
  String getValue()
  {
    return value;
  }
};

typedef void (*string_input_handler)(SHArg a);

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

class Base
{
  static void onESPNowDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onESPNowDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

  ws_client_info wsClientInfo;

protected:
  static void handleInboxMsg(AF1Msg m);
  static void handleOutboxMsg(AF1Msg m);

  static uint8_t macAP[6];
  static uint8_t macSTA[6];

  // Wifi
  static bool broadcastAP();
  static void connectToWifi();
  // ESP-Now
  static int8_t scanForPeersESPNow();
  static void connectToPeers();
  static void initEspNow();
  static void sendStateChangeMessages(int s);
  static void sendHandshakeRequests(std::set<int> ids);
  static void receiveHandshakeRequest(AF1Msg m);
  static void sendHandshakeResponses(std::set<int> ids);
  static void receiveHandshakeResponse(AF1Msg m);
  static void sendAllHandshakes(bool resend = false);
  static void sendMsgESPNow(AF1Msg msg);
  static void sendTimeSyncMsg(std::set<int> ids, bool isResponse = false);
  static void receiveTimeSyncMsg(AF1Msg m);
  static void sendAllTimeSyncMessages();
  static void setTimeSyncMsgTime(AF1Msg &m);
  // Websocket
  static void sendMsgWS(AF1Msg msg);
  static void connectToWS();
  // Info (currently just WS)
  static void sendMsgInfo(std::set<int> recipients);

  std::map<String, Event> eventMap;

public:
  Base();
  static void begin(uint8_t id);
  static void update();
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
  virtual void doSynced();
  virtual bool doSync();
  virtual void onConnectWSServer();
  virtual DynamicJsonDocument getInfo();
  // Wifi
  static String macToString(const uint8_t *m);
  static void printMac(const uint8_t *m);
  static uint8_t *getMacSTA();
  static uint8_t *getMacAP();
  // ESP-Now
  static void handleHandshakes(bool resend = false);
  // Inbox/Outbox
  static void setInboxMsgHandler(msg_handler h);
  static void setOutboxMsgHandler(msg_handler h);
  static void pushOutbox(AF1Msg m);
  static void pushInbox(AF1Msg m);
  // Websocket
  void setWSClientInfo(ws_client_info w);
  ws_client_info getWSClientInfo();
  // HTTP
  static DynamicJsonDocument httpFetch(String url);
  static DynamicJsonDocument httpPost(String url, DynamicJsonDocument body);
  // Other
  static void setBuiltinLED(bool on);
  std::map<String, Event> &getEventMap();
  unsigned long getStartMs();
  // Sync
  static void scheduleSyncStart();
  void setSyncStartTime(unsigned long s);
  unsigned long getSyncStartTime();
  // Interval Events
  unsigned long getElapsedMs();
  void resetEvents();
  void activateEvents();
  void deactivateEvents();

  static int getCurState();
  static int getPrevState();
  static void setRequestedState(int s);
  static int getRequestedState();
  static int getCurMode();
  static int getPrevMode();
  static void setRequestedMode(int m);
  static int getRequestedMode();
  static void setInitialMode(int m);
  static int getInitialMode();
  static bool handleModeChange(int m);
  static String modeToString(int s);
  static void handleUserInput(String s);
  static String stateToString(int s);
  static bool handleStateChange(int s);
  static const std::vector<wifi_ap_info> getWifiAPs();
  static void setInitialState(int s);
  static int getInitialState();
  static void setPurgNext(int p, int n);
  static const std::map<int, Base *> &getStateEntMap();
  static Base *getCurStateEnt();
  static int getDeviceID();
  static void setDefaultWSClientInfo(ws_client_info w);
  static ws_client_info getDefaultWSClientInfo();
  static std::map<int, af1_peer_info> &getPeerInfoMap();
  static std::map<String, int> &getMacToIDMap();
  static std::set<int> getPeerIDs();
  static void setCurWSClientInfo(ws_client_info i);
  static ws_client_info getCurWSClientInfo();
  static WiFiUDP ntpUDP;
  static NTPClient timeClient;
  static unsigned long convertTime(int id, unsigned long t);
  static void setIEIntervalMs(String e, unsigned long m);
  static void setTEIntervalMs(String e, unsigned long m);
  static void set(Event e);

  static void registerStateEnt(int i, Base *s);
  static void registerStringHandler(String s, string_input_handler h);
  static void registerWifiAP(String s, String p);
  static void registerWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd);
};

#endif // STATEENT_VIRTUAL_BASE_BASE_H_