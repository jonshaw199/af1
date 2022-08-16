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

#ifndef MESSAGEHANDLER_MESSAGEHANDLER_H_
#define MESSAGEHANDLER_MESSAGEHANDLER_H_

#include <map>
#include <queue>
#include <set>
#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include <mutex>

#include "message/message.h"
#include "box/box.h"

typedef struct js_peer_info
{
  esp_now_peer_info_t espnowPeerInfo;
  bool handshakeRequest;
  bool handshakeResponse;
  JSMessage lastMsg;
  std::mutex mutex;
} js_peer_info;

class MessageHandler
{
  MessageHandler(); // constructor
  std::map<int, js_peer_info> peerInfoMap;
  std::map<String, int> macToIDMap;
  uint8_t macAP[6];
  uint8_t macSTA[6];
  Box inbox;
  Box outbox;
  static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

public:
  static MessageHandler &getInstance();
  static void scanForPeers(bool overwriteExisting = false);
  static void connectToPeers();
  static void initEspNow();
  static void sendHandshakeRequests(std::set<int> ids);
  static void receiveHandshakeRequest(JSMessage m);
  static void sendHandshakeResponses(std::set<int> ids);
  static void receiveHandshakeResponse(JSMessage m);
  static void sendStateChangeMessages(int s);
  static const std::map<int, js_peer_info> &getPeerInfoMap(); // Read only
  static void sendAllHandshakes();
  static std::set<int> getPeerIDs();
  static const TSQueue<JSMessage> &getOutbox(); // Read only
  static const TSQueue<JSMessage> &getInbox();  // Read only
  static void handleInboxMessages();
  static void handleOutboxMessages();
  static void setInboxMsgHandler(msg_handler h);
  static void setOutboxMsgHandler(msg_handler h);
  static void pushOutbox(JSMessage m);
  static void sendMsg(JSMessage msg);
  static void init();
  static void loop();
};

#endif // MESSAGEHANDLER_MESSAGEHANDLER_H_
