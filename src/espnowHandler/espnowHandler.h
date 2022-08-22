#ifndef ESPNOWHANDLER_ESPNOWHANDLER_H_
#define ESPNOWHANDLER_ESPNOWHANDLER_H_

#include <Arduino.h>
#include <esp_now.h>
#include <map>
#include <mutex>
#include <set>

#include "message/message.h"

typedef struct js_peer_info
{
  esp_now_peer_info_t espnowPeerInfo;
  bool handshakeRequest;
  bool handshakeResponse;
  JSMessage lastMsg;
  std::mutex mutex;
} js_peer_info;

class ESPNowHandler
{
  ESPNowHandler(); // Private
  std::map<int, js_peer_info> peerInfoMap;
  std::map<String, int> macToIDMap;
  static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);

public:
  static ESPNowHandler &getInstance(); // Public singleton instance getter
  static void scanForPeers();
  static void connectToPeers();
  static void initEspNow();
  static void handleHandshakes();
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

#endif // ESPNOWHANDLER_ESPNOWHANDLER_H_
