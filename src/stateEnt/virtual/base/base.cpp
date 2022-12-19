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

#include <vector>

#include "base.h"
#include "stateEnt/ota/ota.h"
#include "stateEnt/restart/restart.h"
#include "stateEnt/init/init.h"
#include "stateEnt/purg/purg.h"
#include "stateEnt/synctest/syncTest.h"

static int curState;
static int prevState;
static int requestedState;
static int initialState;
static String deviceID;
static Base *stateEnt;
static std::map<int, Base *> stateEntMap;

static std::map<String, string_input_handler> stringHandlerMap;
static std::vector<wifi_ap_info> wifiAPs;

static ws_client_info curWSClientInfo;
static ws_client_info defaultWSClientInfo;

std::map<String, af1_peer_info> Base::peerInfoMap;
std::map<String, String> Base::macToIDMap;

WiFiUDP Base::ntpUDP;
NTPClient Base::timeClient(ntpUDP);

unsigned long startMs;

static Box inbox;
static Box outbox;

static HTTPClient httpClient;
static WiFiMulti wifiMulti;

unsigned long Base::syncStartTime;

uint8_t Base::macAP[6];
uint8_t Base::macSTA[6];

WebSocketsClient Base::webSocketClient;

std::map<String, Event> Base::globalEventMap;

static bool detached;

#define MAX_PACKET_ID 5 // Can't store many unacked

static uint8_t nextPacketId;
static std::map<uint8_t, AF1Msg> unackedPackets;

Base::Base()
{
}

void Base::begin(String id)
{
  stateEntMap[STATE_INIT] = new Init();
  stateEntMap[STATE_OTA] = new OTA();
  stateEntMap[STATE_RESTART] = new Restart();
  stateEntMap[STATE_PURG] = new Purg<Base>();
  stateEntMap[STATE_IDLE_BASE] = new Base();
  stateEntMap[STATE_SYNC_TEST] = new SyncTest();

  addStringHandler(SHKEY_OTA, [](SHArg a)
                   { setRequestedState(STATE_OTA); });
  addStringHandler(SHKEY_RESTART, [](SHArg a)
                   { setRequestedState(STATE_RESTART); });
  addStringHandler(SHKEY_IDLE, [](SHArg a)
                   { setRequestedState(STATE_IDLE_BASE); });
  addStringHandler(SHKEY_SYNCTEST, [](SHArg a)
                   { setRequestedState(STATE_SYNC_TEST); });
  addStringHandler(SHKEY_HANDSHAKE, [](SHArg a)
                   { handleHandshakes(true); });
  addStringHandler(SHKEY_DETACH, [](SHArg a)
                   { detach(a.getValue().toInt()); });

  initialState = STATE_IDLE_BASE;
  defaultWSClientInfo = {"", "", 0, ""};
  curWSClientInfo = defaultWSClientInfo;

  deviceID = id;

  detached = false;

  int s = STATE_INIT;
  curState = s;
  requestedState = s;
  handleStateChange(s); // Let Init stateEnt handle everything
}

void Base::update()
{
  if (WiFi.status() == WL_CONNECTED && timeClient.isTimeSet())
  {
    timeClient.update();
  }

  inbox.handleMessages();
  outbox.handleMessages();

  // Handling user input
  if (Serial.available() > 0)
  {
    String s = Serial.readString();
    handleUserInput(s);
  }

  // State Events
  for (std::map<String, Event>::iterator it = stateEnt->eventMap.begin(); it != stateEnt->eventMap.end(); it++)
  {
    stateEnt->eventMap[it->first].cbIfTimeAndActive();
  }
  // Global Events
  for (std::map<String, Event>::iterator it = globalEventMap.begin(); it != globalEventMap.end(); it++)
  {
    globalEventMap[it->first].cbIfTimeAndActive();
  }

  webSocketClient.loop();

  // StateManager Loop - END
  // Handling this first instead of last; allows us to use init.loop() if we need it before switching to the requested state (or maybe we don't want to request a new state during init at all?)
  stateEnt->loop();

  // Check if state change requested
  int curState = getCurState();
  int requestedState = getRequestedState();
  if (curState != requestedState)
  {
    Serial.println("Handling state change request: " + stateToString(requestedState));
    stateEnt->preStateChange(requestedState);
    // Requested state may have changed between last and next function call
    handleStateChange(requestedState);
    Serial.println("State change complete");
  }
}

void Base::setup()
{
  Serial.println("Base::setup()");
  resetEvents();
  activateEvents();
  startMs = millis();
  connectToWifi();
  connectToWS();
}

void Base::loop()
{
}

void Base::preStateChange(int s)
{
  Serial.print("Switching to ");
  Serial.print(stateEntMap.at(s)->getName());
  Serial.println(" state now.");
  deactivateEvents();

#if MASTER
  if (!detached)
  {
    sendStateChangeMessages(s);
  }
#endif

  setBuiltinLED(0); // to do
}

unsigned long Base::getElapsedMs()
{
  unsigned long nowMs = millis();
  return nowMs - startMs;
}

void Base::handleInboxMsg(AF1Msg &m)
{
  Serial.print("<");

#if PRINT_MSG_RECV
  m.print();
#endif

  switch (m.getType())
  {
  case TYPE_CHANGE_STATE:
  {
    Serial.println("State change request message in inbox");
    if (detached)
    {
      Serial.println("Detached; ignoring state change request message");
    }
    else
    {
      requestedState = m.getState();
    }
  }
  break;
  case TYPE_HANDSHAKE_REQUEST:
  {
    Serial.println("Handshake request message in inbox");
    receiveHandshakeRequest(m);
    sendHandshakeResponses({m.getSenderId()});
  }
  break;
  case TYPE_HANDSHAKE_RESPONSE:
  {
    Serial.println("Handshake response message in inbox");
    receiveHandshakeResponse(m);
    sendTimeSyncMsg({m.getSenderId()});
  }
  break;
  case TYPE_TIME_SYNC:
  {
    Serial.println("Time sync message in inbox");
    receiveTimeSyncMsg(m);
  }
  break;
  case TYPE_TIME_SYNC_RESPONSE:
  {
    Serial.println("Time sync response message in inbox");
    receiveTimeSyncMsg(m);
  }
  break;
  case TYPE_TIME_SYNC_START:
  {
    Serial.print("Received time: ");
    unsigned long t = m.json()["timeSyncStart"];
    Serial.println(t);
    syncStartTime = convertTime(m.getSenderId(), t);
    Serial.print("Converted time: ");
    Serial.println(syncStartTime);
    scheduleSyncStart();
  }
  break;
  case TYPE_MQTT_PUBLISH:
  {
    // Acks
    uint8_t q = m.json()["qos"];
    if (q == 1)
    {
      int p = m.json()["packetId"];
      AF1Msg res(TYPE_MQTT_PUBACK);
      res.json()["packetId"] = p;
      pushOutbox(res);
    }
    else if (q == 2)
    {
      int p = m.json()["packetId"];
      AF1Msg res(TYPE_MQTT_PUBREC);
      res.json()["packetId"] = p;
      pushOutbox(res);
    }
  }
  break;
  case TYPE_MQTT_PUBACK:
  case TYPE_MQTT_PUBCOMP:
  {
    uint8_t p = m.json()["packetId"];
    unackedPackets.erase(p);
  }
  break;
  case TYPE_MQTT_PUBREC:
  {
    uint8_t p = m.json()["packetId"];
    unackedPackets[p] = m;
    AF1Msg res(TYPE_MQTT_PUBREL);
    res.json()["packetId"] = p;
    pushOutbox(res);
  }
  break;
  case TYPE_MQTT_PUBREL:
  {
    int p = m.json()["packetId"];
    AF1Msg res(TYPE_MQTT_PUBCOMP);
    res.json()["packetId"] = p;
    pushOutbox(res);
  }
  break;
  }

#if IMPLICIT_STATE_CHANGE
#ifndef MASTER
  if (m.getState() != curState && m.getState() != requestedState && !detached)
  {
    Serial.println("Implicit state change to " + stateToString(m.getState()));
    requestedState = m.getState();
  }
#endif
#endif
}

void Base::handleOutboxMsg(AF1Msg &m)
{
  Serial.print(">");
#if PRINT_MSG_SEND
  m.print();
#endif

  uint8_t q = m.json()["qos"];
  switch (m.getType())
  {
  case TYPE_MQTT_PUBLISH:
  case TYPE_MQTT_PUBREL:
    if (q)
    {
      m.json()["packetId"] = nextPacketId;
      unackedPackets[nextPacketId] = m;
      nextPacketId = (nextPacketId + 1) % MAX_PACKET_ID;
    }
    break;
  case TYPE_TIME_SYNC:
    m.json()["timeSyncTime"] = millis();
    break;
  }

  // ESPNow
  sendMsgESPNow(m);
  // Websocket
  sendMsgWS(m);
  // sendHTTP?
  // sendBluetooth?
}

msg_handler Base::getInboxHandler()
{
  return handleInboxMsg;
}

msg_handler Base::getOutboxHandler()
{
  return handleOutboxMsg;
}

String Base::getName()
{
  return "(unknown state name)";
}

void Base::resetEvents()
{
  for (std::map<String, Event>::iterator it = eventMap.begin(); it != eventMap.end(); it++)
  {
    eventMap[it->first].reset();
  }
  // Global
  for (std::map<String, Event>::iterator it = globalEventMap.begin(); it != globalEventMap.end(); it++)
  {
    if (globalEventMap[it->first].getStartTimeType() == START_STATE_MS)
    {
      globalEventMap[it->first].reset();
    }
  }
}

void Base::activateEvents()
{
  for (std::map<String, Event>::iterator it = eventMap.begin(); it != eventMap.end(); it++)
  {
    eventMap[it->first].activate();
  }
}

void Base::deactivateEvents()
{
  std::vector<String> delKeys;
  for (std::map<String, Event>::iterator it = eventMap.begin(); it != eventMap.end(); it++)
  {
    eventMap[it->first].deactivate();
    if (eventMap[it->first].getType() == EVENT_TYPE_TEMP)
    {
      delKeys.push_back(it->first);
    }
  }
  for (String s : delKeys)
  {
    Serial.print("Deleting temporary Event ");
    Serial.println(s);
    eventMap.erase(s);
  }
}

/*
  From WifiHandler
*/

// Setup access point (aka open wifi network)
bool Base::broadcastAP()
{
  Serial.println("Broadcasting soft AP");
  String Prefix = DEVICE_PREFIX;
  String id = deviceID;
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

  std::vector<wifi_ap_info> v = wifiAPs;

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
    IPAddress primaryDNS(8, 8, 8, 8);
    IPAddress secondaryDNS(8, 8, 4, 4);
    // Configures static IP address
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
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
    Serial.println("initializing timeClient");
    timeClient.begin();
    timeClient.update(); // Initial update needed before isTimeSet()?
    stateEnt->onConnectWifi();
  }
  else
  {
    stateEnt->onConnectWifiFailed();
  }
}

/*
  From MessageHandler
*/

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
#if SAFETY_CHECK_INBOX_OVERFLOW
  if (inbox.size() > 3)
  {
#if PRINT_MSG_RECEIVE
    Serial.println("Warning: inbox overflow approaching");
#endif
    // do something
  }
#endif

  inbox.enqueue(m);
}

/*
  New
*/

StaticJsonDocument<2048> Base::httpGet(String url)
{
  StaticJsonDocument<2048> result;
  connectToWifi();
  if (WiFi.status() == WL_CONNECTED)
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

StaticJsonDocument<2048> Base::httpPost(String url, JsonDocument &body)
{
  StaticJsonDocument<2048> result;
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
    String peerDeviceID = macToIDMap[macToString(mac_addr)];
#if PRINT_MSG_SEND
    Serial.println("Delivery failed to peer ID " + deviceID);
#else
    Serial.print("X");
#endif
    peerInfoMap[peerDeviceID].mutex.lock();
    // Check if there are more retries remaining and retry if so
    if (peerInfoMap[peerDeviceID].lastMsg.getSendCnt() - 1 < peerInfoMap[peerDeviceID].lastMsg.getMaxRetries())
    {
#if PRINT_MSG_SEND
      Serial.println("Retrying send to device ID " + peerDeviceID);
#endif
      AF1Msg msg = peerInfoMap[peerDeviceID].lastMsg;
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
    peerInfoMap[peerDeviceID].mutex.unlock();
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
  AF1JsonDoc data;
  deserializeJson(data, incomingData);
  AF1Msg msg(data);
  pushInbox(msg);
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

void Base::handleHandshakes(bool resend)
{
  scanForPeersESPNow();
  connectToPeers();
  sendAllHandshakes(resend);
}

int8_t Base::scanForPeersESPNow()
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
      // Check if the current network is one of our peers
      if (SSID.indexOf(DEVICE_PREFIX) == 0) // Technically must start with a prefix
      {
        String deviceID = SSID.substring(String(DEVICE_PREFIX).length());
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
          // Get BSSID (MAC Address) of the peer
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
            peerInfoMap[deviceID].lastMsg = AF1Msg();
            peerInfoMap[deviceID].otherTimeSync = 0;
            peerInfoMap[deviceID].thisTimeSync = 0;
            macToIDMap[macToString(info.peer_addr)] = deviceID;
            Serial.println("Saved peer info for device ID " + deviceID);
          }
        }
        else
        {
          Serial.println("Peer info already collected for device ID " + deviceID);
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
  if (peerInfoMap.size())
  {
    for (std::map<String, af1_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
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
        // Peer not connected; attempt to connect
        esp_err_t connectStatus = esp_now_add_peer(&it->second.espnowPeerInfo);
        // IMPORTANT: Should the peer/peerInfo be removed from the map if pairing failed? TO DO
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
    // No peers found in scan
    Serial.println("No peers found to connect");
  }
}

void Base::sendMsgESPNow(AF1Msg msg)
{
  // If recipients set is empty then send to all
  std::set<String> recipientIDs = msg.getRecipients().size() ? msg.getRecipients() : getPeerIDs();

  if (!recipientIDs.size())
  {
#if PRINT_MSG_SEND
    Serial.println("No ESPNow peers; unable to send message");
#endif
  }

  for (std::set<String>::iterator it = recipientIDs.begin(); it != recipientIDs.end() && peerInfoMap.count(*it); it++)
  {
    peerInfoMap[*it].mutex.lock();
    // Update last msg sent for this peer (now doing this even if sending fails)
    peerInfoMap[*it].lastMsg = msg;
    // Serial.println("Sending message to device ID " + String(*it) + " (MAC address " + macToString(peerInfoMap[*it].espnowPeerInfo.peer_addr) + ")");
    /*
    af1_msg m = msg.getInnerMsg();
    esp_err_t result = esp_now_send(peerInfoMap[*it].espnowPeerInfo.peer_addr, (uint8_t *)&m, sizeof(m));
    */
    String s;
    serializeJson(msg.json(), s);
    esp_err_t result = esp_now_send(peerInfoMap[*it].espnowPeerInfo.peer_addr, (uint8_t *)s.c_str(), AF1_MSG_SIZE);

    // Serial.print("Send Status: ");
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

bool Base::doScanForPeersESPNow()
{
  return !doSync();
}

void Base::sendStateChangeMessages(int s)
{
  Serial.println("Pushing state change messages to the outbox");

  AF1Msg msg = AF1Msg(TYPE_CHANGE_STATE);
  msg.json()["state"] = s;
  msg.setMaxRetries(DEFAULT_RETRIES);
  pushOutbox(msg);
}

void Base::sendHandshakeRequests(std::set<String> ids)
{
  Serial.println("Pushing handshake requests to outbox");

  AF1Msg msg = AF1Msg(TYPE_HANDSHAKE_REQUEST);

  JsonArray macJson = msg.json().createNestedArray("mac");
  copyArray(macAP, macJson);
  msg.setRecipients(ids);
  pushOutbox(msg);

  for (std::set<String>::const_iterator it = ids.begin(); it != ids.end(); it++)
  {
    peerInfoMap[*it].handshakeRequest = true;
  }
}

void Base::receiveHandshakeRequest(AF1Msg m)
{
  Serial.println("Receiving handshake request from ID " + String(m.getSenderId()));

  esp_now_peer_info_t ei;
  memset(&ei, 0, sizeof(ei));
  copyArray(m.json()["mac"], ei.peer_addr);
  // memcpy(&ei.peer_addr, (uint8_t *) m.json()["mac"], 6);
  ei.channel = ESPNOW_CHANNEL;
  ei.encrypt = 0; // No encryption
  ei.ifidx = WIFI_IF_AP;
  peerInfoMap[m.getSenderId()].espnowPeerInfo = ei;
  peerInfoMap[m.getSenderId()].handshakeResponse = false;
  peerInfoMap[m.getSenderId()].lastMsg = AF1Msg();
  peerInfoMap[m.getSenderId()].otherTimeSync = 0;
  peerInfoMap[m.getSenderId()].thisTimeSync = 0;

  connectToPeers();
}

void Base::sendHandshakeResponses(std::set<String> ids)
{
  Serial.println("Pushing handshake responses to outbox");
  AF1Msg msg = AF1Msg(TYPE_HANDSHAKE_RESPONSE);
  msg.setRecipients(ids);
  pushOutbox(msg);
}

void Base::receiveHandshakeResponse(AF1Msg m)
{
  Serial.println("Receiving handshake response from ID " + m.getSenderId());
  peerInfoMap[m.getSenderId()].handshakeResponse = true;
  stateEnt->onConnectEspNowPeer(m.getSenderId());
}

void Base::sendAllHandshakes(bool resend)
{
  for (std::map<String, af1_peer_info>::const_iterator it = peerInfoMap.begin(); it != peerInfoMap.end() && (!it->second.handshakeRequest || resend); it++)
  {
    sendHandshakeRequests({it->first});
  }
}

void Base::sendTimeSyncMsg(std::set<String> ids, bool isResponse)
{
  Serial.print("Pushing time sync ");
  Serial.print(isResponse ? "response " : "");
  Serial.println("messages to outbox");

  AF1Msg msg = AF1Msg(isResponse ? TYPE_TIME_SYNC_RESPONSE : TYPE_TIME_SYNC);
  msg.json()["timeSyncTime"] = millis(); // Not sure if this is really necessary; time is now set right before sending which is better
  msg.setRecipients(ids);
  msg.setMaxRetries(DEFAULT_RETRIES);

  pushOutbox(msg);
}

void Base::receiveTimeSyncMsg(AF1Msg m)
{
  Serial.print("Receiving time sync ");
  Serial.print(m.getType() == TYPE_TIME_SYNC_RESPONSE ? "response" : "");
  Serial.print(" msg from ID ");
  Serial.println(m.getSenderId());

  if (peerInfoMap.count(m.getSenderId()))
  {
    peerInfoMap[m.getSenderId()].otherTimeSync = m.json()["timeSyncTime"];
    unsigned long thisMs = millis();
    peerInfoMap[m.getSenderId()].thisTimeSync = thisMs;
    Serial.print("Other device time: ");
    Serial.print(peerInfoMap[m.getSenderId()].otherTimeSync);
    Serial.print("; This device time: ");
    Serial.println(peerInfoMap[m.getSenderId()].thisTimeSync);

    if (m.getType() == TYPE_TIME_SYNC)
    {
      sendTimeSyncMsg({m.getSenderId()}, true);
    }
  }
  else
  {
    Serial.println("Time sync message rejected; need to handshake first");
  }
}

void Base::sendAllTimeSyncMessages()
{
  for (std::map<String, af1_peer_info>::const_iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
  {
    sendTimeSyncMsg({it->first});
  }
}

// From WSEnt

void Base::setWS(String host, String path, int port, String protocol, unsigned long reconnectMs)
{
  wsClientInfo.host = host;
  wsClientInfo.path = path;
  wsClientInfo.port = port;
  wsClientInfo.protocol = protocol;
  wsClientInfo.reconnectMs = reconnectMs;
}

void Base::sendMsgWS(AF1Msg m)
{
  if (webSocketClient.isConnected())
  {
    String s;
    serializeJson(m.json(), s);
#if PRINT_MSG_SEND
    Serial.println("Sending websocket message");
#endif
    webSocketClient.sendTXT(s);
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
    ws_client_info i = stateEnt->wsClientInfo;
    if (!i && defaultWSClientInfo)
    {
      i = defaultWSClientInfo;
    }

    if (!stateEnt->doConnectToWSServer())
    {
      Serial.println("Websocket connections disabled in this state");
    }
    else if (i)
    {
      Serial.print("connectToWS(): checking WS connection: ");
      Serial.println(i.toString());
      if (webSocketClient.isConnected() && (i == curWSClientInfo || !curWSClientInfo) && i == defaultWSClientInfo)
      {
        Serial.println("Already connected to websocket");
      }
      else
      {
        // Connect to the websocket server
        webSocketClient.begin(i.host, i.port, i.path);
        webSocketClient.onEvent(handleWebSocketEvent);
        // use HTTP Basic Authorization this is optional remove if not needed
        // webSocketClient.setAuthorization("user", "Password");
        // try every 5000 again if connection has failed
        webSocketClient.setReconnectInterval(i.reconnectMs);
      }
    }
    else
    {
      Serial.println("No websocket info provided for this state; skipping.");
    }
  }
}

bool Base::doConnectToWSServer()
{
  return !doSync();
}

unsigned long Base::getStartMs()
{
  return startMs;
}

void Base::scheduleSyncStart()
{
  unsigned long s = syncStartTime;

  Serial.println("Scheduling");
  Serial.print("Current time: ");
  Serial.print(millis());
  Serial.print("; Start time: ");
  Serial.print(s);
  Serial.print("; diff: ");
  Serial.println(s - millis());

  unsigned long dif = s - millis();
  unsigned long startMs = dif + stateEnt->getElapsedMs();

  addEvent(Event(
      EVENTKEY_SCHEDULE_SYNC_START, [](ECBArg a)
      {
    Serial.println("Starting");
    stateEnt->doSynced(); },
      EVENT_TYPE_TEMP, 0, 1, startMs));
}

void Base::doSynced()
{
  addEvent(Event(
      EVENTKEY_SYNC_START,
      [](ECBArg a)
      { setBuiltinLED(a.cbCnt % 2); },
      EVENT_TYPE_TEMP, 300));
}

bool Base::doSync()
{
  return false;
}

// StateManager - BEGIN

int Base::getCurState()
{
  return curState;
}

int Base::getPrevState()
{
  return prevState;
}

void Base::setRequestedState(int s)
{
  if (stateEntMap.find(s) == stateEntMap.end())
  {
    Serial.print("Requested state ");
    Serial.print(s);
    Serial.println(" is not recognized, not setting.");
  }
  else if (curState == s)
  {
    Serial.print("Already in requested state ");
    Serial.println(s);
  }
  else
  {
    Serial.print("Setting requested state: ");
    Serial.print(stateEntMap[s]->getName());
    Serial.print(" (");
    Serial.print(s);
    Serial.println(")");
    requestedState = s;
  }
}

int Base::getRequestedState()
{
  return requestedState;
}

void Base::handleUserInput(String s)
{
  String s2 = s;
  s2.toLowerCase();
  int wildIndex = s2.indexOf("*");
  if (wildIndex >= 0 && stringHandlerMap.count(s2.substring(0, wildIndex + 1)))
  {
    stringHandlerMap[s2.substring(0, wildIndex + 1)](SHArg(s2, s2.substring(wildIndex + 1)));
  }
  else if (stringHandlerMap.count(s2))
  {
    stringHandlerMap[s2](SHArg(s2));
  }
  else
  {
    Serial.print("String input not recognized: ");
    Serial.println(s2);
  }
}

String Base::stateToString(int s)
{
  if (stateEntMap.count(s))
  {
    return stateEntMap[s]->getName();
  }
  return "Unknown state name";
}

bool Base::handleStateChange(int s)
{
  if (stateEntMap.count(s))
  {
    prevState = curState;
    curState = s;

    stateEnt = stateEntMap[s];
    stateEnt->setup();
    stateEnt->setInboxMsgHandler(stateEnt->getInboxHandler());
    stateEnt->setOutboxMsgHandler(stateEnt->getOutboxHandler());

    return true;
  }
  else
  {
    return false;
  }
}

void Base::addStateEnt(int i, Base *s)
{
  stateEntMap[i] = s;
}

void Base::removeStateEnt(int i)
{
  stateEntMap.erase(i);
}

void Base::addStringHandler(String s, string_input_handler h)
{
  stringHandlerMap[s] = h;
}

void Base::removeStringHandler(String s)
{
  stringHandlerMap.erase(s);
}

void Base::addWifiAP(String s, String p)
{
  wifi_ap_info i;
  i.ssid = s;
  i.pass = p;
  i.staticIP[0] = -1;
  i.staticIP[1] = -1;
  i.staticIP[2] = -1;
  i.staticIP[3] = -1;
  i.gatewayIP[0] = -1;
  i.gatewayIP[1] = -1;
  i.gatewayIP[2] = -1;
  i.gatewayIP[3] = -1;
  i.subnetIP[0] = -1;
  i.subnetIP[1] = -1;
  i.subnetIP[2] = -1;
  i.subnetIP[3] = -1;
  wifiAPs.push_back(i);
}

void Base::addWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd)
{
  wifi_ap_info i;
  i.ssid = s;
  i.pass = p;
  i.staticIP[0] = a;
  i.staticIP[1] = b;
  i.staticIP[2] = c;
  i.staticIP[3] = d;
  i.gatewayIP[0] = ga;
  i.gatewayIP[1] = gb;
  i.gatewayIP[2] = gc;
  i.gatewayIP[3] = gd;
  i.subnetIP[0] = sa;
  i.subnetIP[1] = sb;
  i.subnetIP[2] = sc;
  i.subnetIP[3] = sd;
  wifiAPs.push_back(i);
}

const std::vector<wifi_ap_info> Base::getWifiAPs()
{
  return wifiAPs;
}

void Base::setInitialState(int s)
{
  initialState = s;
}

int Base::getInitialState()
{
  return initialState;
}

void Base::setPurgNext(int p, int n)
{
  (static_cast<Purg<Base> *>(stateEntMap[p]))->setNext(n);
}

String Base::getDeviceID()
{
  return deviceID;
}

void Base::setDefaultWS(String host, String path, int port, String protocol, unsigned long reconnectMs)
{
  defaultWSClientInfo.host = host;
  defaultWSClientInfo.path = path;
  defaultWSClientInfo.port = port;
  defaultWSClientInfo.protocol = protocol;
  defaultWSClientInfo.reconnectMs = reconnectMs;
}

void Base::onConnectWSServer()
{
}

void Base::onDisconnectWSServer()
{
}

void Base::onConnectWifi()
{
}

void Base::onConnectWifiFailed()
{
}

void Base::onConnectEspNowPeer(String p)
{
}

std::set<String> Base::getPeerIDs()
{
  std::set<String> result;
  for (std::map<String, af1_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
  {
    result.insert(it->first);
  }
  return result;
}

Base *Base::getCurStateEnt()
{
  return stateEntMap[curState];
}

unsigned long Base::convertTime(String id, unsigned long t)
{
  if (peerInfoMap.count(id))
  {
    unsigned long dif = t - peerInfoMap[id].otherTimeSync;
    return peerInfoMap[id].thisTimeSync + dif;
  }
  return 0;
}

void Base::setIntervalTime(String e, unsigned long t)
{
  if (stateEnt->eventMap.count(e))
  {
    stateEnt->eventMap.at(e).setIntervalTime(t, stateEnt->getElapsedMs());
  }
}

void Base::addEvent(Event e)
{
  if (e.getType() == EVENT_TYPE_GLOBAL)
  {
    globalEventMap[e.getName()] = e;
  }
  else
  {
    stateEnt->eventMap[e.getName()] = e;
  }
}

void Base::removeEvent(String e)
{
  globalEventMap.erase(e);
  stateEnt->eventMap.erase(e);
}

void Base::detach(bool d)
{
  Serial.print("Detaching: ");
  Serial.println(d);
  detached = d;
}

void Base::hexdump(const void *mem, uint32_t len, uint8_t cols)
{
  const uint8_t *src = (const uint8_t *)mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++)
  {
    if (i % cols == 0)
    {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void Base::handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
  {
    Serial.printf("[WSc] Disconnected!\n");
    stateEnt->onDisconnectWSServer();
  }
  break;
  case WStype_CONNECTED:
  {
    Serial.printf("[WSc] Connected to url: %s\n", payload);
    // send message to server when Connected
    // webSocketClient.sendTXT("Connected");
    delay(2000);
    stateEnt->onConnectWSServer();
  }
  break;
  case WStype_TEXT:
  {
    Serial.printf("[WSc] get text: %s\n", payload);
    // send message to server
    // webSocket.sendTXT("message here");
    Serial.print(".");

    // Can't assume the payload is valid JSON here; big TO DO

    String payloadStr((char *)payload);
    AF1JsonDoc doc;
    deserializeJson(doc, payloadStr);
    pushInbox(doc);
  }
  break;
  case WStype_BIN:
  {
    Serial.printf("[WSc] get binary length: %u\n", length);
    hexdump(payload, length);
    // send data to server
    // webSocket.sendBIN(payload, length);

    // to do
  }
  break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}
