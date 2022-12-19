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
#include <WebSocketsClient.h>
#include <vector>
#include <mutex>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "event/event.h"
#include "state/state.h"
#include "message/message.h"
#include "box/box.h"
#include "pre.h"

#define max(a, b) ((a) >= (b) ? (a) : (b))

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
  unsigned long reconnectMs;
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

class Base
{
  static void onESPNowDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onESPNowDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
  static void setInboxMsgHandler(msg_handler h);
  static void setOutboxMsgHandler(msg_handler h);
  static bool handleStateChange(int s);
  static void handleUserInput(String s);
  static void sendStateChangeMessages(int s);
  static void sendHandshakeRequests(std::set<String> ids);
  static void receiveHandshakeRequest(AF1Msg m);
  static void sendHandshakeResponses(std::set<String> ids);
  static void receiveHandshakeResponse(AF1Msg m);
  static void sendAllHandshakes(bool resend = false);
  static void sendMsgESPNow(AF1Msg msg);
  static void sendTimeSyncMsg(std::set<String> ids, bool isResponse = false);
  static void receiveTimeSyncMsg(AF1Msg m);
  static void sendAllTimeSyncMessages();
  static void sendMsgWS(AF1Msg msg);
  static void connectToWS();
  static void connectToWifi();
  static int8_t scanForPeersESPNow();
  static void connectToPeers();
  static const std::vector<wifi_ap_info> getWifiAPs();
  static unsigned long convertTime(String id, unsigned long t);
  static std::set<String> getPeerIDs();
  static void hexdump(const void *mem, uint32_t len, uint8_t cols = 16);
  static void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length);

  static std::map<String, Event> globalEventMap;
  static std::map<String, af1_peer_info> peerInfoMap;
  static std::map<String, String> macToIDMap;
  static WiFiUDP ntpUDP;
  static WebSocketsClient webSocketClient;

  void resetEvents();
  void activateEvents();
  void deactivateEvents();

  std::map<String, Event> eventMap;
  ws_client_info wsClientInfo;

protected:
  static void handleInboxMsg(AF1Msg &m);
  static void handleOutboxMsg(AF1Msg &m);
  static bool broadcastAP();
  static void handleHandshakes(bool resend = false);
  static void scheduleSyncStart();
  static void initEspNow();

  static uint8_t macAP[6];
  static uint8_t macSTA[6];
  static unsigned long syncStartTime;

public:
  Base();
  static void begin(String id);
  static void update();
  static String macToString(const uint8_t *m);
  static void printMac(const uint8_t *m);
  static void pushOutbox(AF1Msg m);
  static void pushInbox(AF1Msg m);
  static AF1JsonDoc httpGet(String url);
  static AF1JsonDoc httpPost(String url, AF1JsonDoc body);
  static void setBuiltinLED(bool on);
  static int getCurState();
  static int getPrevState();
  static void setRequestedState(int s);
  static int getRequestedState();
  static String stateToString(int s);
  static void setInitialState(int s);
  static int getInitialState();
  static void setPurgNext(int p, int n);
  static Base *getCurStateEnt();
  static String getDeviceID();
  static void setIntervalTime(String e, unsigned long t);
  static void detach(bool detach);
  static void setDefaultWS(String host, String path, int port, String protocol = "", unsigned long reconnectMs = WS_RECONNECT_MS);
  static void addEvent(Event e);
  static void removeEvent(String eventName);
  static void addStateEnt(int i, Base *s);
  static void removeStateEnt(int i);
  static void addStringHandler(String s, string_input_handler h);
  static void removeStringHandler(String s);
  static void addWifiAP(String s, String p);
  static void addWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd);

  static NTPClient timeClient;

  virtual void setup();
  virtual void loop();
  virtual void preStateChange(int s);
  virtual msg_handler getInboxHandler();
  virtual msg_handler getOutboxHandler();
  virtual String getName();
  virtual bool doScanForPeersESPNow();
  virtual bool doConnectToWSServer();
  virtual void doSynced();
  virtual bool doSync();
  virtual void onConnectWSServer();
  virtual void onDisconnectWSServer();
  virtual void onConnectWifi();
  virtual void onConnectWifiFailed();
  virtual void onConnectEspNowPeer(String peerId);

  unsigned long getStartMs();
  unsigned long getElapsedMs();
  void setWS(String host, String path, int port, String protocol = "", unsigned long reconnectMs = WS_RECONNECT_MS);
};

#endif // STATEENT_VIRTUAL_BASE_BASE_H_