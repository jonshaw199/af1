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

#include "base.h"
#include "stateManager/stateManager.h"
#include "pre.h"

uint8_t Base::macAP[6];
uint8_t Base::macSTA[6];

Box Base::inbox;
Box Base::outbox;

HTTPClient Base::httpClient;

WiFiMulti Base::wifiMulti;

WebSocketClient Base::webSocketClient;
// Use WiFiClient class to create TCP connections
WiFiClient Base::client;

Base::Base()
{
  intervalEventMap.insert(std::pair<String, IntervalEvent>("Base_1", IntervalEvent(MS_HANDSHAKE_LOOP, [](IECBArg a)
                                                                                   {
    if (StateManager::getStateEntMap().at(StateManager::getCurState())->scanForESPNowPeers()) {
      handleHandshakes();
    } else {
      Serial.println("ESPNow peer scan denied in current state");
    }
    return true; })));
}

void Base::setup()
{
  Serial.println("Base::setup()");
  resetIntervalEvents();
  activateIntervalEvents();
  startMs = millis();
  connectToWifi();
  connectToWS();
}

void Base::loop()
{
  inbox.handleMessages([](AF1Msg &m)
                       { m.deserializeInnerMsgESPNow(); });
  outbox.handleMessages([](AF1Msg &m)
                        { m.serializeInnerMsgESPNow(); });

  // Handling user input
  if (Serial.available() > 0)
  {
    String s = Serial.readString();
    StateManager::handleUserInput(s);
  }

  // Interval events
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].cbIfTimeAndActive(getElapsedMs());
  }

  // Incoming websocket messages
  String data;
  if (client)
  {
    webSocketClient.getData(data);
    if (data.length() > 0)
    {
      Serial.print(".");
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, data);
      pushInbox(doc);
    }
  }
  else
  {
    // Serial.println("Client disconnected");
  }
}

bool Base::validateStateChange(int s)
{
  return true;
}

void Base::preStateChange(int s)
{
  Serial.print("Switching to ");
  Serial.print(StateManager::getStateEntMap().at(s)->getName());
  Serial.println(" state now.");
  deactivateIntervalEvents();

#if MASTER
  sendStateChangeMessages(s);
#endif
}

unsigned long Base::getElapsedMs()
{
  unsigned long nowMs = millis();
  return nowMs - startMs;
}

void Base::handleInboxMsg(AF1Msg m)
{
  Serial.print("<");

  m.deserializeInnerMsgESPNow();

#if PRINT_MSG_SEND
  m.print();
#endif

  switch (m.getType())
  {
  case TYPE_CHANGE_STATE:
    Serial.println("State change request message in inbox");
    StateManager::setRequestedState(m.getState());
    break;
  case TYPE_HANDSHAKE_REQUEST:
    Serial.println("Handshake request message in inbox");
    receiveHandshakeRequest(m);
    sendHandshakeResponses({m.getSenderID()});
    break;
  case TYPE_HANDSHAKE_RESPONSE:
    Serial.println("Handshake response message in inbox");
    receiveHandshakeResponse(m);
    break;
  }

#if IMPLICIT_STATE_CHANGE
#ifndef MASTER
  if (m.getState() != StateManager::getCurState() && m.getState() != StateManager::getRequestedState())
  {
    Serial.println("Implicit state change to " + StateManager::stateToString(m.getState()));
    StateManager::setRequestedState(m.getState());
  }
#endif
#endif
}

void Base::handleOutboxMsg(AF1Msg m)
{
  Serial.print(">");

  // Prepare msg for send
  m.serializeInnerMsgESPNow();

#if PRINT_MSG_SEND
  m.print();
#endif

  // ESPNow
  sendMsgESPNow(m);
  // Websocket
  sendMsgWS(m);
  // sendHTTP?
  // sendBluetooth?
}

void Base::overrideInboxHandler()
{
  setInboxMsgHandler(handleInboxMsg);
}

void Base::overrideOutboxHandler()
{
  setOutboxMsgHandler(handleOutboxMsg);
}

String Base::getName()
{
  return "(unknown state)";
}

void Base::resetIntervalEvents()
{
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].reset();
  }
}

void Base::activateIntervalEvents()
{
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].activate();
  }
}

void Base::deactivateIntervalEvents()
{
  for (std::map<String, IntervalEvent>::iterator it = intervalEventMap.begin(); it != intervalEventMap.end(); it++)
  {
    intervalEventMap[it->first].deactivate();
  }
}

/*
  From WifiHandler
*/

// Setup access point (aka open wifi network); this is used by scanForESPNowPeers() (or whatever its called)
/*
  Unused now? How?
*/
bool Base::broadcastAP()
{
  Serial.println("Broadcasting soft AP");
  String Prefix = DEVICE_PREFIX;
  String id = String(StateManager::getDeviceID());
  String SSID = Prefix + id;
  String Password = DEVICE_AP_PASS;
  return WiFi.softAP(SSID.c_str(), Password.c_str(), ESPNOW_CHANNEL, 0);
}

String Base::macToString(const uint8_t *m)
{
  char buffer[33];
  String result = "";
  for (int i = 0; i < 6; i++)
  {
    itoa(m[i], buffer, 16);
    result += buffer;
  }
  return result;
}

void Base::printMac(const uint8_t *m)
{
  Serial.print("{");
  for (int i = 0; i < 6; i++)
  {
    Serial.print("0x");
    Serial.print(m[i], HEX);
    if (i < 5)
      Serial.print(',');
  }
  Serial.println("}");
}

uint8_t *Base::getMacSTA()
{
  return macSTA;
}

uint8_t *Base::getMacAP()
{
  return macAP;
}

void Base::connectToWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Already connected to wifi");
    return;
  }

  /*
    To do: figure out how to handle static IP and WifiMulti properly
    Currently the static IP must be the same for all APs
  */

  std::vector<wifi_ap_info> v = StateManager::getWifiAPs();

  if (!v.size())
  {
    return;
  }

  if (v.size() && v[0].staticIP[0] >= 0)
  {
    // Set your Static IP address
    IPAddress local_IP(v[0].staticIP[0], v[0].staticIP[1], v[0].staticIP[2], v[0].staticIP[3]);
    // Set your Gateway IP address
    IPAddress gateway(v[0].gatewayIP[0], v[0].gatewayIP[1], v[0].gatewayIP[2], v[0].gatewayIP[3]);
    IPAddress subnet(v[0].subnetIP[0], v[0].subnetIP[1], v[0].subnetIP[2], v[0].subnetIP[3]);
    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet))
    {
      Serial.println("Static IP Failed to configure");
    }
    else
    {
      Serial.println("Static IP config success");
    }
  }

  // Connect to Wi-Fi network with SSID and password
  for (wifi_ap_info i : v)
  {
    wifiMulti.addAP(i.ssid.c_str(), i.pass.c_str());
  }

  Serial.println("Connecting to wifi");

  if (wifiMulti.run() == WL_CONNECTED)
  {
    Serial.println("Wifi connection successful");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
  }
}

/*
  From MessageHandler
*/

const TSQueue<AF1Msg> &Base::getOutbox()
{
  return outbox;
}

const TSQueue<AF1Msg> &Base::getInbox()
{
  return inbox;
}

void Base::setInboxMsgHandler(msg_handler h)
{
  inbox.setMsgHandler(h);
}

void Base::setOutboxMsgHandler(msg_handler h)
{
  outbox.setMsgHandler(h);
}

void Base::pushOutbox(AF1Msg m)
{
  outbox.enqueue(m);
}

void Base::pushInbox(AF1Msg m)
{
  inbox.enqueue(m);
}

/*
  New
*/

DynamicJsonDocument Base::httpFetch(String url)
{
  DynamicJsonDocument result(1024);
  connectToWifi();
  if ((WiFi.status() == WL_CONNECTED))
  {
    httpClient.begin(url);
    Serial.println("Making HTTP GET request...");
    Serial.print("URL: ");
    Serial.println(url);
    int httpCode = httpClient.GET();
    if (httpCode > 0)
    {
      String resBody = httpClient.getString();
      deserializeJson(result, resBody);
      Serial.println(resBody);
    }
    else
    {
      Serial.println("HTTP GET error");
    }
    httpClient.end();
  }
  return result;
}

DynamicJsonDocument Base::httpPost(String url, DynamicJsonDocument body)
{
  DynamicJsonDocument result(1024);
  connectToWifi();
  if ((WiFi.status() == WL_CONNECTED))
  {
    httpClient.begin(url);
    httpClient.addHeader("Content-Type", "application/json");
    String serializedBody;
    serializeJson(body, serializedBody);
    Serial.println("Making HTTP POST request...");
    Serial.print("URL: ");
    Serial.println(url);
    Serial.print("Body: ");
    Serial.println(serializedBody);
    int httpCode = httpClient.POST(serializedBody);
    if (httpCode > 0)
    {
      String resBody = httpClient.getString();
      deserializeJson(result, resBody);
      Serial.println(resBody);
    }
    else
    {
      Serial.print("HTTP POST error: ");
      Serial.println(httpClient.errorToString(httpCode));
    }
    httpClient.end();
  }
  return result;
}

void Base::setBuiltinLED(bool on)
{
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, on);
#endif
}

/*
  From ESPNowEnt
*/

void Base::onESPNowDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
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

void Base::onESPNowDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
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

void Base::initEspNow()
{
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESP-NOW Init Success");
    esp_now_register_recv_cb(onESPNowDataRecv);
    esp_now_register_send_cb(onESPNowDataSent);
  }
  else
  {
    Serial.println("ESP-Now init failed");
  }
}

bool Base::handleHandshakes()
{
  scanForPeers();
  connectToPeers();
  sendAllHandshakes();
  return true;
}

int8_t Base::scanForPeers()
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

void Base::connectToPeers()
{
  // Try to connect if not connected already
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

void Base::sendMsgESPNow(AF1Msg msg)
{
  msg.setTransportType(TRANSPORT_ESPNOW);

  // If recipients set is empty then send to all
  std::set<int> recipientIDs = msg.getRecipients().size() ? msg.getRecipients() : StateManager::getPeerIDs();

  if (!recipientIDs.size())
  {
#if PRINT_MSG_SEND
    Serial.println("No ESPNow peers; unable to send message");
#endif
  }

  for (std::set<int>::iterator it = recipientIDs.begin(); it != recipientIDs.end(); it++)
  {
    StateManager::getPeerInfoMap()[*it].mutex.lock();
    // Update last msg sent for this peer (now doing this even if sending fails)
    StateManager::getPeerInfoMap()[*it].lastMsg = msg;
    // Serial.println("Sending message to device ID " + String(*it) + " (MAC address " + macToString(peerInfoMap[*it].espnowPeerInfo.peer_addr) + ")");
    af1_msg m = msg.getInnerMsg();
    esp_err_t result = esp_now_send(StateManager::getPeerInfoMap()[*it].espnowPeerInfo.peer_addr, (uint8_t *)&m, sizeof(m));
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

void Base::serializeESPNow(AF1Msg &m)
{
#if PRINT_MSG_SEND
  Serial.println("Base::serializeESPNow()");
#endif
}

void Base::deserializeESPNow(AF1Msg &m)
{
#if PRINT_MSG_RECEIVE
  Serial.println("Base::deserializeESPNow()");
#endif
}

bool Base::scanForESPNowPeers()
{
  return true;
}

void Base::sendStateChangeMessages(int s)
{
  Serial.println("Pushing state change messages to the outbox");

  AF1Msg msg = AF1Msg();

  msg.setType(TYPE_CHANGE_STATE);
  msg.setSenderID(StateManager::getDeviceID());
  msg.setState(s);
  msg.setMaxRetries(DEFAULT_RETRIES);

  pushOutbox(msg);
}

void Base::sendHandshakeRequests(std::set<int> ids)
{
  Serial.println("Sending handshake requests");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_REQUEST);
  msg.setSenderID(StateManager::getDeviceID());
  msg.setState(StateManager::getCurState()); // msg.setState(STATE_HANDSHAKE);
  msg.setData(getMacAP());
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);

  for (std::set<int>::const_iterator it = ids.begin(); it != ids.end(); it++)
  {
    StateManager::getPeerInfoMap()[*it].handshakeRequest = true;
  }
}

void Base::receiveHandshakeRequest(AF1Msg m)
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

void Base::sendHandshakeResponses(std::set<int> ids)
{
  Serial.println("Sending handshake responses");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_RESPONSE);
  msg.setSenderID(StateManager::getDeviceID());
  msg.setState(StateManager::getCurState()); // msg.setState(STATE_HANDSHAKE);
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);
}

void Base::receiveHandshakeResponse(AF1Msg m)
{
  Serial.println("Receiving handshake response from ID " + String(m.getSenderID()));
  StateManager::getPeerInfoMap()[m.getSenderID()].handshakeResponse = true;
}

void Base::sendAllHandshakes()
{
  for (std::map<int, af1_peer_info>::const_iterator it = StateManager::getPeerInfoMap().begin(); it != StateManager::getPeerInfoMap().end() && !it->second.handshakeRequest; it++)
  {
    sendHandshakeRequests({it->first});
  }
}

// From WSEnt

void Base::setWSClientInfo(ws_client_info w)
{
  Serial.print("Setting websocket client info: ");
  Serial.println(w.toString());
  wsClientInfo.host = w.host;
  wsClientInfo.path = w.path;
  wsClientInfo.port = w.port;
  wsClientInfo.protocol = w.protocol;
}

ws_client_info Base::getWSClientInfo()
{
  return wsClientInfo;
}

void Base::sendMsgWS(AF1Msg m)
{
  m.setTransportType(TRANSPORT_WEBSOCKET);
  if (client)
  {
    String s;
    serializeJson(m.getJson(), s);
#if PRINT_MSG_SEND
    Serial.println("Sending websocket message");
#endif
    webSocketClient.sendData(s);
  }
  else
  {
#if PRINT_MSG_SEND
    Serial.println("Websocket client not connected; unable to send message");
#endif
  }
}

void Base::connectToWS()
{
  // Only attempting to connect to websocket if wifi is connected first
  if (WiFi.status() == WL_CONNECTED)
  {
    ws_client_info i = StateManager::getStateEntMap().at(StateManager::getCurState())->wsClientInfo;
    if (!i.host.length() && StateManager::getDefaultWSClientInfo().host.length())
    {
      i = StateManager::getDefaultWSClientInfo();
    }

    Serial.print("connectToWS(): checking WS connection: ");
    Serial.println(i.toString());

    if (!i.host.length())
    {
      Serial.println("Invalid WS info; skipping");
    }
    else if (client && i == StateManager::getCurWSClientInfo())
    {
      Serial.println("Already connected to websocket");
    }
    else
    {
      // Connect to the websocket server
      if (client.connect(i.host.c_str(), i.port))
      {
        Serial.println("Connected to websocket server");
      }
      else
      {
        Serial.println("Connection to websocket server failed");
      }

      int lenH = i.host.length() + 1;
      int lenP = i.path.length() + 1;
      int lenPr = i.protocol.length() + 1;
      char h[lenH];
      char p[lenP];
      char pr[lenPr];
      i.host.toCharArray(h, lenH);
      i.path.toCharArray(p, lenP);
      i.protocol.toCharArray(pr, lenPr);
      webSocketClient.host = h;
      webSocketClient.path = p;
      if (i.protocol.length())
      {
        webSocketClient.protocol = pr;
      }

      if (webSocketClient.handshake(client))
      {
        Serial.println("Handshake successful");
        StateManager::setCurWSClientInfo(i);
      }
      else
      {
        Serial.println("Handshake failed");
      }
    }
  }
}
