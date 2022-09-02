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

#ifndef STATEENT_VIRTUAL_ESPNOWENT_ESPNOWENT_H_
#define STATEENT_VIRTUAL_ESPNOWENT_ESPNOWENT_H_

#include <Arduino.h>
#include <esp_now.h>
#include <map>
#include <mutex>
#include <set>

#include "stateEnt/virtual/base/base.h"
#include "message/message.h"

typedef struct js_peer_info
{
  esp_now_peer_info_t espnowPeerInfo;
  bool handshakeRequest;
  bool handshakeResponse;
  JSMessage lastMsg;
  std::mutex mutex;
} js_peer_info;

class ESPNowEnt : public Base
{
  // From espnowHandler
  static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

protected:
  static bool handleInboxMsg(JSMessage m);
  static bool handleOutboxMsg(JSMessage m);

public:
  void setInboxMessageHandler();
  void setOutboxMessageHandler();
  bool preStateChange(int s); // From espnowEntMaster
  // From espnowHandler
  static int8_t scanForPeers();
  static void connectToPeers();
  static void initEspNow();
  static bool handleHandshakes();
  static void sendStateChangeMessages(int s);
  static void sendHandshakeRequests(std::set<int> ids);
  static void receiveHandshakeRequest(JSMessage m);
  static void sendHandshakeResponses(std::set<int> ids);
  static void receiveHandshakeResponse(JSMessage m);
  static const std::map<int, js_peer_info> &getPeerInfoMap(); // Read only
  static void sendAllHandshakes();
  static std::set<int> getPeerIDs();
  static void sendMsg(JSMessage msg);
};

#endif // STATEENT_VIRTUAL_ESPNOWENT_ESPNOWENT_H_