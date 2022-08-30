#include "espnowEnt.h"
#include "pre.h"

// From espnowHandler
static std::map<int, js_peer_info> peerInfoMap;
static std::map<String, int> macToIDMap;
// End ?

bool ESPNowEnt::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_HANDSHAKE_REQUEST:
    Serial.println("Handshake request message in inbox");
    receiveHandshakeRequest(m);
    sendHandshakeResponses({m.getSenderID()});
    return true;
  case TYPE_HANDSHAKE_RESPONSE:
    Serial.println("Handshake response message in inbox");
    receiveHandshakeResponse(m);
    return true;
  }

  return Base::handleInboxMsg(m);
}

void ESPNowEnt::setInboxMessageHandler()
{
  setInboxMsgHandler(handleInboxMsg);
}

bool ESPNowEnt::handleOutboxMsg(JSMessage m)
{
  sendMsg(m);
  return true;
}

void ESPNowEnt::setOutboxMessageHandler()
{
  setOutboxMsgHandler(handleOutboxMsg);
}

bool ESPNowEnt::preStateChange(int s)
{
#if MASTER
  bool baseResult = Base::preStateChange(s);
  if (baseResult)
  {
    int slaveState = s;
    switch (s)
    {
    case STATE_OTA:       // Slaves already notified
    case STATE_RESTART:   // Slaves already notified
    case STATE_HANDSHAKE: // Slaves can' be notified
      return true;
    case STATE_PURG_OTA:
      slaveState = STATE_OTA;
      break;
    case STATE_PURG_RESTART:
      slaveState = STATE_RESTART;
      break;
    }
    sendStateChangeMessages(slaveState);
  }
  return baseResult;
#endif
  return true;
}

/*
  From ESPNowHandler ?
*/

void ESPNowEnt::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.print("Last Packet Sent to ");
  // Serial.println(macStr);
  // Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    // Serial.println("Delivery Success");
  }
  else
  {
    // Serial.println("Delivery Fail");
    int peerDeviceID = macToIDMap[macToString(mac_addr)];
    Serial.println("Delivery failed to peer ID " + String(peerDeviceID));
    peerInfoMap[peerDeviceID].mutex.lock();
    // Check if there are more retries remaining and retry if so
    if (peerInfoMap[peerDeviceID].lastMsg.getSendCnt() - 1 < peerInfoMap[peerDeviceID].lastMsg.getMaxRetries())
    {
      Serial.println("Retrying send to device ID " + String(peerDeviceID));
      JSMessage msg = peerInfoMap[peerDeviceID].lastMsg;
      msg.incrementSendCnt();
      msg.setRecipients({peerDeviceID}); // Only resending to 1 device!
      pushOutbox(msg);
      // outbox.enqueue(msg);
    }
    else
    {
      Serial.println("Max retries reached; not sending");
    }
    peerInfoMap[peerDeviceID].mutex.unlock();
  }
}

void ESPNowEnt::onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  // Serial.print("Last Packet Recv from: ");
  // Serial.println(macStr);
  Serial.print(".");
  js_message msg;
  memcpy(&msg, incomingData, sizeof(msg));
  JSMessage msgWrapper = msg;
  pushInbox(msg);
  // inbox.enqueue(msgWrapper);
}

void ESPNowEnt::initEspNow()
{
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(onDataRecv);
    esp_now_register_send_cb(onDataSent);
  }
  else
  {
    Serial.println("ESP-NOW Init Failed");
    // delay(3000);
    // ESP.restart();
  }
}

bool ESPNowEnt::handleHandshakes()
{
  scanForPeers();
  connectToPeers();
  sendAllHandshakes();
  return true;
}

int8_t ESPNowEnt::scanForPeers()
{
  Serial.println("Scanning for peers");
  int8_t networkCnt = WiFi.scanNetworks();
  if (networkCnt)
  {
    for (int i = 0; i < networkCnt; ++i)
    {
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDStr = WiFi.BSSIDstr(i);
      if (PRINT_WIFI_SCAN_RESULTS)
      {
        // Print SSID and RSSI for each netowrk
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" [");
        Serial.print(BSSIDStr);
        Serial.print("]");
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      }
      // Check if the current network is one of our slaves
      if (SSID.indexOf(STRINGIFY(DEVICE_PREFIX)) == 0) // Technically must start with a prefix
      {
        int deviceID = SSID.substring(String(STRINGIFY(DEVICE_PREFIX)).length()).toInt();
        // Check the overwrite argument and only overwrite existing entries if true
        if (!peerInfoMap.count(deviceID))
        {
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(SSID);
          Serial.print(" [");
          Serial.print(BSSIDStr);
          Serial.print("]");
          Serial.print(" (");
          Serial.print(RSSI);
          Serial.print(")");
          Serial.println("");
          // Get BSSID (MAC Address) of the Slave
          int mac[6];
          if (6 == sscanf(BSSIDStr.c_str(), "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
          {
            esp_now_peer_info_t info;
            memset(&info, 0, sizeof(info));
            for (int j = 0; j < 6; ++j)
            {
              info.peer_addr[j] = (uint8_t)mac[j];
            }
            info.channel = ESPNOW_CHANNEL;
            info.encrypt = 0; // no encryption
            info.ifidx = WIFI_IF_AP;
            peerInfoMap[deviceID].espnowPeerInfo = info;
            peerInfoMap[deviceID].handshakeResponse = false;
            peerInfoMap[deviceID].lastMsg = JSMessage();
            macToIDMap[macToString(info.peer_addr)] = deviceID;
            Serial.println("Saved peer info for device ID " + String(deviceID));
          }
        }
        else
        {
          Serial.println("Peer info already collected for device ID " + String(deviceID));
        }
      }
    }
  }
  else
  {
    Serial.println("No peers found");
  }

  WiFi.scanDelete();

  return networkCnt;
}

void ESPNowEnt::connectToPeers()
{
  // Check if each slave is already connected to master
  // If not, then try to connect
  if (peerInfoMap.size())
  {
    for (std::map<int, js_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
    {
      // Check if the peer exists
      if (esp_now_is_peer_exist(it->second.espnowPeerInfo.peer_addr))
      {
        Serial.print("Peer ID ");
        Serial.print(it->first);
        Serial.println(" already paired");
      }
      else
      {
        // Slave not connected; attempt to connect
        esp_err_t connectStatus = esp_now_add_peer(&it->second.espnowPeerInfo);
        if (connectStatus == ESP_OK)
        {
          Serial.println("Pair success");
        }
        else if (connectStatus == ESP_ERR_ESPNOW_ARG)
        {
          Serial.println("Add Peer - Invalid Argument");
        }
        else if (connectStatus == ESP_ERR_ESPNOW_FULL)
        {
          Serial.println("Peer list full");
        }
        else if (connectStatus == ESP_ERR_ESPNOW_NO_MEM)
        {
          Serial.println("Out of memory");
        }
        else if (connectStatus == ESP_ERR_ESPNOW_EXIST)
        {
          Serial.println("Peer Exists");
        }
        else
        {
          Serial.println("Unknown connection error");
        }
      }
    }
  }
  else
  {
    // No slaves found in scan
    Serial.println("No slaves found to connect");
  }
}

void ESPNowEnt::sendMsg(JSMessage msg)
{
  // If recipients set is empty then send to all
  std::set<int> recipientIDs = msg.getRecipients().size() ? msg.getRecipients() : getPeerIDs();
  for (std::set<int>::iterator it = recipientIDs.begin(); it != recipientIDs.end(); it++)
  {
    peerInfoMap[*it].mutex.lock();
    // Update last msg sent for this peer (now doing this even if sending fails)
    peerInfoMap[*it].lastMsg = msg;
    // Serial.println("Sending message to device ID " + String(*it) + " (MAC address " + macToString(peerInfoMap[*it].espnowPeerInfo.peer_addr) + ")");
    esp_err_t result = esp_now_send(peerInfoMap[*it].espnowPeerInfo.peer_addr, (uint8_t *)&msg, sizeof(msg));
    // Serial.print("Send Status: ");
    if (result != ESP_OK)
    {
      Serial.println("Send failed to device ID " + String(*it) + "; reason: ");
    }
    if (result == ESP_OK)
    {
      // Serial.println("Success");

      // Update the send count of that last msg
      peerInfoMap[*it].lastMsg.incrementSendCnt();
    }
    else if (result == ESP_ERR_ESPNOW_NOT_INIT)
    {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
    }
    else if (result == ESP_ERR_ESPNOW_ARG)
    {
      Serial.println("Invalid Argument");
    }
    else if (result == ESP_ERR_ESPNOW_INTERNAL)
    {
      Serial.println("Internal Error");
    }
    else if (result == ESP_ERR_ESPNOW_NO_MEM)
    {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
      Serial.println("Peer not found.");
    }
    else if (result == ESP_ERR_ESPNOW_IF)
    {
      Serial.println("Current wifi interface doesnt match that of peer");
    }
    else
    {
      Serial.println("Not sure what happened");
    }
    peerInfoMap[*it].mutex.unlock();
    delay(DELAY_SEND);
  }
}

void ESPNowEnt::sendStateChangeMessages(int s)
{
  Serial.println("Sending state change messages");

  JSMessage msg = JSMessage();

  msg.setType(TYPE_CHANGE_STATE);
  msg.setSenderID(JS_ID);
  msg.setState(s);
  msg.setMaxRetries(DEFAULT_RETRIES);

  pushOutbox(msg);
}

void ESPNowEnt::sendHandshakeRequests(std::set<int> ids)
{
  Serial.println("Sending handshake requests");

  JSMessage msg = JSMessage();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_REQUEST);
  msg.setSenderID(JS_ID);
  msg.setState(STATE_HANDSHAKE);
  msg.setData(getMacAP());
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);

  for (std::set<int>::const_iterator it = ids.begin(); it != ids.end(); it++)
  {
    peerInfoMap[*it].handshakeRequest = true;
  }
}

void ESPNowEnt::receiveHandshakeRequest(JSMessage m)
{
  Serial.println("Receiving handshake request from ID " + String(m.getSenderID()));
  Serial.print("Mac: ");
  printMac(m.getData());

  esp_now_peer_info_t ei;
  memset(&ei, 0, sizeof(ei));
  memcpy(&ei.peer_addr, m.getData(), 6);
  ei.channel = ESPNOW_CHANNEL;
  ei.encrypt = 0; // No encryption
  ei.ifidx = WIFI_IF_AP;
  peerInfoMap[m.getSenderID()].espnowPeerInfo = ei;
  peerInfoMap[m.getSenderID()].handshakeResponse = false;
  peerInfoMap[m.getSenderID()].lastMsg = JSMessage();

  connectToPeers();
}

void ESPNowEnt::sendHandshakeResponses(std::set<int> ids)
{
  Serial.println("Sending handshake responses");

  JSMessage msg = JSMessage();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_RESPONSE);
  msg.setSenderID(JS_ID);
  msg.setState(STATE_HANDSHAKE);
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);
}

void ESPNowEnt::receiveHandshakeResponse(JSMessage m)
{
  Serial.println("Receiving handshake response from ID " + String(m.getSenderID()));
  peerInfoMap[m.getSenderID()].handshakeResponse = true;
}

const std::map<int, js_peer_info> &ESPNowEnt::getPeerInfoMap()
{
  return peerInfoMap;
}

void ESPNowEnt::sendAllHandshakes()
{
  for (std::map<int, js_peer_info>::const_iterator it = peerInfoMap.begin(); it != peerInfoMap.end() && !it->second.handshakeRequest; it++)
  {
    sendHandshakeRequests({it->first});
  }
}

std::set<int> ESPNowEnt::getPeerIDs()
{
  std::set<int> result;
  for (std::map<int, js_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
  {
    result.insert(it->first);
  }
  return result;
}