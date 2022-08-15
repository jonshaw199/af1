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

#include "messageHandler.h"
#include "util/wifi/wifiUtil.h"
#include "util/stringify/stringify.h"

MessageHandler::MessageHandler()
{
}

MessageHandler &MessageHandler::getInstance()
{
  static MessageHandler instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
  return instance;
}

void MessageHandler::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
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
    int peerDeviceID = getInstance().macToIDMap[WifiUtil::macToString(mac_addr)];
    Serial.println("Delivery failed to peer ID " + String(peerDeviceID));
    getInstance().peerInfoMap[peerDeviceID].mutex.lock();
    // Check if there are more retries remaining and retry if so
    if (getInstance().peerInfoMap[peerDeviceID].lastMsg.getSendCnt() - 1 < getInstance().peerInfoMap[peerDeviceID].lastMsg.getMaxRetries())
    {
      Serial.println("Retrying send to device ID " + String(peerDeviceID));
      JSMessage msg = getInstance().peerInfoMap[peerDeviceID].lastMsg;
      msg.incrementSendCnt();
      msg.setRecipients({peerDeviceID}); // Only resending to 1 device!
      getInstance().outbox.enqueue(msg);
    }
    else
    {
      Serial.println("Max retries reached; not sending");
    }
    getInstance().peerInfoMap[peerDeviceID].mutex.unlock();
  }
}

void MessageHandler::onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Last Packet Recv from: ");
  Serial.println(macStr);
  js_message msg;
  memcpy(&msg, incomingData, sizeof(msg));
  JSMessage msgWrapper = msg;
  getInstance().inbox.enqueue(msgWrapper);
}

void MessageHandler::init()
{
  WifiUtil::setAPMode();
  WiFi.softAPmacAddress(getInstance().macAP);
  Serial.print("MAC AP: ");
  WifiUtil::printMac(getInstance().macAP);
  WifiUtil::setSTAMode();
  WiFi.macAddress(getInstance().macSTA);
  Serial.print("MAC STA: ");
  WifiUtil::printMac(getInstance().macSTA);
}

void MessageHandler::initEspNow()
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

void MessageHandler::scanForPeers(bool overwriteExisting)
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
        if (!getInstance().peerInfoMap.count(deviceID) || overwriteExisting)
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
            getInstance().peerInfoMap[deviceID].espnowPeerInfo = info;
            getInstance().peerInfoMap[deviceID].handshakeResponse = false;
            getInstance().peerInfoMap[deviceID].lastMsg = JSMessage();
            getInstance().macToIDMap[WifiUtil::macToString(info.peer_addr)] = deviceID;
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
}

void MessageHandler::connectToPeers()
{
  // Check if each slave is already connected to master
  // If not, then try to connect
  if (getInstance().peerInfoMap.size())
  {
    for (std::map<int, js_peer_info>::iterator it = getInstance().peerInfoMap.begin(); it != getInstance().peerInfoMap.end(); it++)
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

void MessageHandler::sendMsg(JSMessage msg)
{
  // If recipients set is empty then send to all
  std::set<int> recipientIDs = msg.getRecipients().size() ? msg.getRecipients() : getPeerIDs();
  for (std::set<int>::iterator it = recipientIDs.begin(); it != recipientIDs.end(); it++)
  {
    getInstance().peerInfoMap[*it].mutex.lock();
    // Update last msg sent for this peer (now doing this even if sending fails)
    getInstance().peerInfoMap[*it].lastMsg = msg;
    // Serial.println("Sending message to device ID " + String(*it) + " (MAC address " + WifiUtil::macToString(getInstance().peerInfoMap[*it].espnowPeerInfo.peer_addr) + ")");
    esp_err_t result = esp_now_send(getInstance().peerInfoMap[*it].espnowPeerInfo.peer_addr, (uint8_t *)&msg, sizeof(msg));
    // Serial.print("Send Status: ");
    if (result != ESP_OK)
    {
      Serial.println("Send failed to device ID " + String(*it) + "; reason: ");
    }
    if (result == ESP_OK)
    {
      // Serial.println("Success");

      // Update the send count of that last msg
      getInstance().peerInfoMap[*it].lastMsg.incrementSendCnt();
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
    getInstance().peerInfoMap[*it].mutex.unlock();
    delay(DELAY_SEND);
  }
}

void MessageHandler::sendHandshakeRequests(std::set<int> ids)
{
  Serial.println("Sending handshake requests");

  JSMessage msg = JSMessage();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_REQUEST);
  msg.setSenderID(JS_ID);
  msg.setState(STATE_HANDSHAKE);
  msg.setSenderAPMac(getInstance().macAP);
  // Set wrapper
  msg.setRecipients(ids);

  getInstance().outbox.enqueue(msg);

  for (std::set<int>::const_iterator it = ids.begin(); it != ids.end(); it++)
  {
    getInstance().peerInfoMap[*it].handshakeRequest = true;
  }
}

void MessageHandler::receiveHandshakeRequest(JSMessage m)
{
  Serial.println("Receiving handshake request from ID " + String(m.getSenderID()));
  Serial.print("Mac: ");
  WifiUtil::printMac(m.getSenderAPMac());

  esp_now_peer_info_t ei;
  memset(&ei, 0, sizeof(ei));
  memcpy(&ei.peer_addr, m.getSenderAPMac(), 6);
  ei.channel = ESPNOW_CHANNEL;
  ei.encrypt = 0; // No encryption
  ei.ifidx = WIFI_IF_AP;
  getInstance().peerInfoMap[m.getSenderID()].espnowPeerInfo = ei;
  getInstance().peerInfoMap[m.getSenderID()].handshakeResponse = false;
  getInstance().peerInfoMap[m.getSenderID()].lastMsg = JSMessage();

  connectToPeers();
}

void MessageHandler::sendHandshakeResponses(std::set<int> ids)
{
  Serial.println("Sending handshake responses");

  JSMessage msg = JSMessage();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_RESPONSE);
  msg.setSenderID(JS_ID);
  msg.setState(STATE_HANDSHAKE);
  // Set wrapper
  msg.setRecipients(ids);

  getInstance().outbox.enqueue(msg);
}

void MessageHandler::receiveHandshakeResponse(JSMessage m)
{
  Serial.println("Receiving handshake response from ID " + String(m.getSenderID()));
  getInstance().peerInfoMap[m.getSenderID()].handshakeResponse = true;
}

const std::map<int, js_peer_info> &MessageHandler::getPeerInfoMap()
{
  return getInstance().peerInfoMap;
}

void MessageHandler::sendAllHandshakes()
{
  for (std::map<int, js_peer_info>::const_iterator it = getInstance().peerInfoMap.begin(); it != getInstance().peerInfoMap.end() && !it->second.handshakeRequest; it++)
  {
    sendHandshakeRequests({it->first});
  }
}

std::set<int> MessageHandler::getPeerIDs()
{
  std::set<int> result;
  for (std::map<int, js_peer_info>::iterator it = getInstance().peerInfoMap.begin(); it != getInstance().peerInfoMap.end(); it++)
  {
    result.insert(it->first);
  }
  return result;
}

const TSQueue<JSMessage> &MessageHandler::getOutbox()
{
  return getInstance().outbox;
}

const TSQueue<JSMessage> &MessageHandler::getInbox()
{
  return getInstance().inbox;
}

void MessageHandler::handleInboxMessages()
{
  getInstance().inbox.handleMessages();
}

void MessageHandler::handleOutboxMessages()
{
  getInstance().outbox.handleMessages();
}

void MessageHandler::setInboxMsgHandler(msg_handler h)
{
  getInstance().inbox.setMsgHandler(h);
}

void MessageHandler::setOutboxMsgHandler(msg_handler h)
{
  getInstance().outbox.setMsgHandler(h);
}

void MessageHandler::pushOutbox(JSMessage m)
{
  getInstance().outbox.enqueue(m);
}

void MessageHandler::loop()
{
  handleOutboxMessages();
  handleInboxMessages();
}
