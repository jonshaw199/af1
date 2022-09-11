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

#include "espnowEnt.h"
#include "pre.h"
#include "stateManager/stateManager.h"

void ESPNowEnt::handleInboxMsg(AF1Msg m)
{
  Base::handleInboxMsg(m);
  switch (m.getType())
  {
  case TYPE_HANDSHAKE_REQUEST:
    Serial.println("Handshake request message in inbox");
    receiveHandshakeRequest(m);
    sendHandshakeResponses({m.getSenderID()});
  case TYPE_HANDSHAKE_RESPONSE:
    Serial.println("Handshake response message in inbox");
    receiveHandshakeResponse(m);
  }
}

void ESPNowEnt::overrideInboxHandler()
{
  setInboxMsgHandler(handleInboxMsg);
}

void ESPNowEnt::handleOutboxMsg(AF1Msg m)
{
  Base::handleOutboxMsg(m);
  sendMsg(m);
}

void ESPNowEnt::overrideOutboxHandler()
{
  setOutboxMsgHandler(handleOutboxMsg);
}

bool ESPNowEnt::validateStateChange(int s)
{
#if MASTER
  if (s == STATE_NONE)
  {
    Serial.println("ESPNowEnt::validateStateChange: uh oh, idling...");
    StateManager::setRequestedState(STATE_IDLE_ESPNOW);
  }
  else if (StateManager::getCurState() != STATE_PURG_ESPNOW)
  {
    sendStateChangeMessages(s);
    StateManager::setRequestedState(STATE_PURG_ESPNOW);
    StateManager::setPurgNext(STATE_PURG_ESPNOW, s);
  }
#endif
  return true;
}

/*
  From ESPNowHandler
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
#if PRINT_MSG_SEND
    Serial.println("Delivery Success");
#endif
  }
  else
  {
    // Serial.println("Delivery Fail");
    int peerDeviceID = StateManager::getMacToIDMap()[macToString(mac_addr)];
#if PRINT_MSG_SEND
    Serial.println("Delivery failed to peer ID " + String(peerDeviceID));
#endif
    StateManager::getPeerInfoMap()[peerDeviceID].mutex.lock();
    // Check if there are more retries remaining and retry if so
    if (StateManager::getPeerInfoMap()[peerDeviceID].lastMsg.getSendCnt() - 1 < StateManager::getPeerInfoMap()[peerDeviceID].lastMsg.getMaxRetries())
    {
#if PRINT_MSG_SEND
      Serial.println("Retrying send to device ID " + String(peerDeviceID));
#endif
      AF1Msg msg = StateManager::getPeerInfoMap()[peerDeviceID].lastMsg;
      msg.incrementSendCnt();
      msg.setRecipients({peerDeviceID}); // Only resending to 1 device!
      pushOutbox(msg);
      // outbox.enqueue(msg);
    }
    else
    {
#if PRINT_MSG_SEND
      Serial.println("Max retries reached; not sending");
#endif
    }
    StateManager::getPeerInfoMap()[peerDeviceID].mutex.unlock();
  }
}

void ESPNowEnt::onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  Serial.print(".");
#if PRINT_MSG_RECV
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Last Packet Recv from: ");
  Serial.println(macStr);
#endif
  af1_msg msg;
  memcpy(&msg, incomingData, sizeof(msg));
  AF1Msg msgWrapper = msg;
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
    Serial.println("ESP-NOW Init Failed, switching to fallback state");
    StateManager::setRequestedState(STATE_ESPNOW_FALLBACK);
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
      if (SSID.indexOf(DEVICE_PREFIX) == 0) // Technically must start with a prefix
      {
        int deviceID = SSID.substring(String(DEVICE_PREFIX).length()).toInt();
        // Check the overwrite argument and only overwrite existing entries if true
        if (!StateManager::getPeerInfoMap().count(deviceID))
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
            StateManager::getPeerInfoMap()[deviceID].espnowPeerInfo = info;
            StateManager::getPeerInfoMap()[deviceID].handshakeResponse = false;
            StateManager::getPeerInfoMap()[deviceID].lastMsg = AF1Msg();
            StateManager::getMacToIDMap()[macToString(info.peer_addr)] = deviceID;
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
  if (StateManager::getPeerInfoMap().size())
  {
    for (std::map<int, af1_peer_info>::iterator it = StateManager::getPeerInfoMap().begin(); it != StateManager::getPeerInfoMap().end(); it++)
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

void ESPNowEnt::sendMsg(AF1Msg msg)
{
  // If recipients set is empty then send to all
  std::set<int> recipientIDs = msg.getRecipients().size() ? msg.getRecipients() : StateManager::getPeerIDs();
  for (std::set<int>::iterator it = recipientIDs.begin(); it != recipientIDs.end(); it++)
  {
    StateManager::getPeerInfoMap()[*it].mutex.lock();
    // Update last msg sent for this peer (now doing this even if sending fails)
    StateManager::getPeerInfoMap()[*it].lastMsg = msg;
    // Serial.println("Sending message to device ID " + String(*it) + " (MAC address " + macToString(peerInfoMap[*it].espnowPeerInfo.peer_addr) + ")");
    esp_err_t result = esp_now_send(StateManager::getPeerInfoMap()[*it].espnowPeerInfo.peer_addr, (uint8_t *)&msg, sizeof(msg));
    // Serial.print("Send Status: ");
    if (result != ESP_OK)
    {
      Serial.println("Send failed to device ID " + String(*it) + "; reason: ");
    }
    if (result == ESP_OK)
    {
      // Serial.println("Success");

      // Update the send count of that last msg
      StateManager::getPeerInfoMap()[*it].lastMsg.incrementSendCnt();
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
    StateManager::getPeerInfoMap()[*it].mutex.unlock();
    delay(DELAY_SEND);
  }
}

void ESPNowEnt::sendStateChangeMessages(int s)
{
  Serial.println("Pushing state change messages to the outbox");

  AF1Msg msg = AF1Msg();

  msg.setType(TYPE_CHANGE_STATE);
  msg.setSenderID(StateManager::getDeviceID());
  msg.setState(s);
  msg.setMaxRetries(DEFAULT_RETRIES);

  pushOutbox(msg);
}

void ESPNowEnt::sendHandshakeRequests(std::set<int> ids)
{
  Serial.println("Sending handshake requests");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_REQUEST);
  msg.setSenderID(StateManager::getDeviceID());
  msg.setState(STATE_HANDSHAKE);
  msg.setData(getMacAP());
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);

  for (std::set<int>::const_iterator it = ids.begin(); it != ids.end(); it++)
  {
    StateManager::getPeerInfoMap()[*it].handshakeRequest = true;
  }
}

void ESPNowEnt::receiveHandshakeRequest(AF1Msg m)
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
  StateManager::getPeerInfoMap()[m.getSenderID()].espnowPeerInfo = ei;
  StateManager::getPeerInfoMap()[m.getSenderID()].handshakeResponse = false;
  StateManager::getPeerInfoMap()[m.getSenderID()].lastMsg = AF1Msg();

  connectToPeers();
}

void ESPNowEnt::sendHandshakeResponses(std::set<int> ids)
{
  Serial.println("Sending handshake responses");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_RESPONSE);
  msg.setSenderID(StateManager::getDeviceID());
  msg.setState(STATE_HANDSHAKE);
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);
}

void ESPNowEnt::receiveHandshakeResponse(AF1Msg m)
{
  Serial.println("Receiving handshake response from ID " + String(m.getSenderID()));
  StateManager::getPeerInfoMap()[m.getSenderID()].handshakeResponse = true;
}

void ESPNowEnt::sendAllHandshakes()
{
  for (std::map<int, af1_peer_info>::const_iterator it = StateManager::getPeerInfoMap().begin(); it != StateManager::getPeerInfoMap().end() && !it->second.handshakeRequest; it++)
  {
    sendHandshakeRequests({it->first});
  }
}
