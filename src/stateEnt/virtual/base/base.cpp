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
#include "pre.h"

// StateManager - BEGIN (END kinda got lost down a ways)

// #include <set>

// #include "stateManager.h"
#include "modeEnt/basic/basic.h"
#include "stateEnt/ota/ota.h"
#include "stateEnt/restart/restart.h"
#include "stateEnt/init/init.h"
#include "stateEnt/purg/purg.h"
#include "stateEnt/synctest/syncTest.h"

static int curState;
static int prevState;
static int requestedState;
static int initialState;
static int deviceID;
static Base *stateEnt;
static std::map<int, Base *> stateEntMap;

static int curMode;
static int prevMode;
static int requestedMode;
static int initialMode;
static ModeBase *modeEnt;
static std::map<int, ModeBase *> modeEntMap;

static std::map<String, string_input_handler> stringHandlerMap;
static std::vector<wifi_ap_info> wifiAPs;

static ws_client_info curWSClientInfo;
static ws_client_info defaultWSClientInfo;

// From espnowHandler
static std::map<int, af1_peer_info> peerInfoMap;
static std::map<String, int> macToIDMap;

WiFiUDP Base::ntpUDP;
NTPClient Base::timeClient(ntpUDP);

unsigned long startMs;

static Box inbox;
static Box outbox;

static HTTPClient httpClient;
static WiFiMulti wifiMulti;

unsigned long syncStartTime;

uint8_t Base::macAP[6];
uint8_t Base::macSTA[6];

static WebSocketClient webSocketClient;
static WiFiClient client; // Use WiFiClient class to create TCP connections

Base::Base()
{
  eventMap["Base_ESPHandshake"] = Event(
      "Base_ESPHandshake", [](ECBArg a)
      {
                                                          if (stateEnt->doScanForPeersESPNow())
                                                          {
                                                            handleHandshakes();
                                                          }
                                                          else
                                                          {
                                                            Serial.println("ESPNow peer scan denied in current state");
                                                          } },
      false, MS_HANDSHAKE_LOOP);

  syncStartTime = 0;

#if MASTER
  eventMap["Base_SendSyncStartTime"] = Event(
      "Base_SendSyncStartTime", [](ECBArg a)
      {
        if (stateEnt->doSync()) {
          stateEnt->setSyncStartTime(millis() + (unsigned long)MS_TIME_SYNC_START);

          AF1Msg msg;
          msg.setState(curState);
          msg.setType(TYPE_TIME_SYNC_START);
          sync_data d;
          d.ms = stateEnt->getSyncStartTime();
          msg.setData((uint8_t *)&d);
          pushOutbox(msg);
          scheduleSyncStart();
        } },
      false, MS_TIME_SYNC_SCHEDULE_START, 1);
#endif
}

void Base::begin(uint8_t id)
{
  modeEntMap[MODE_BASIC] = new Basic();

  initialMode = MODE_INITIAL;

  stateEntMap[STATE_INIT] = new Init();
  stateEntMap[STATE_OTA] = new OTA();
  stateEntMap[STATE_RESTART] = new Restart();
  stateEntMap[STATE_PURG] = new Purg<Base>();
  stateEntMap[STATE_IDLE_BASE] = new Base();
  stateEntMap[STATE_SYNC_TEST] = new SyncTest();

  registerStringHandler("ota", [](SHArg a)
                        { setRequestedState(STATE_OTA); });
  registerStringHandler("restart", [](SHArg a)
                        { setRequestedState(STATE_RESTART); });
  registerStringHandler("idle", [](SHArg a)
                        { setRequestedState(STATE_IDLE_BASE); });
  registerStringHandler("synctest", [](SHArg a)
                        { setRequestedState(STATE_SYNC_TEST); });
  registerStringHandler("hs", [](SHArg a)
                        { stateEnt->handleHandshakes(true); });

  initialState = STATE_IDLE_BASE;
  defaultWSClientInfo = {"", "", 0, ""};
  curWSClientInfo = defaultWSClientInfo;

  deviceID = id;

  int m = initialMode;
  curMode = m;
  requestedMode = m;
  if (handleModeChange(m))
  {
    int s = STATE_INIT;
    curState = s;
    requestedState = s;
    handleStateChange(s); // Let Init stateEnt handle everything
  }
}

void Base::update()
{
  // Check if mode change requested and proceed if modeEnt->validateStateChange() says its ok
  int curMode = getCurMode();
  int requestedMode = getRequestedMode();
  if (curMode != requestedMode)
  {
    Serial.println("Handling mode change request: " + modeToString(requestedMode));
    if (modeEnt->validateModeChange(requestedMode))
    {
      modeEnt->preModeChange(requestedMode);
      // Requested mode may have changed between last and next function call
      handleModeChange(requestedMode);
      Serial.println("Mode change complete");
    }
    else
    {
      Serial.println("Mode change rejected by validateStateChange");
    }
  }

  if (modeEnt->loop())
  {
    // StateManager Loop - BEGIN

    if (timeClient.isTimeSet())
    {
      timeClient.update();
    }

    inbox.handleMessages([](AF1Msg &m)
                         { m.deserializeInnerMsgESPNow(); });
    outbox.handleMessages([](AF1Msg &m)
                          { 
                          if (m.getType() == TYPE_TIME_SYNC) {
                            setTimeSyncMsgTime(m);
                          }
                          m.serializeInnerMsgESPNow(); });

    // Handling user input
    if (Serial.available() > 0)
    {
      String s = Serial.readString();
      handleUserInput(s);
    }

    // Events
    for (std::map<String, Event>::iterator it = stateEnt->eventMap.begin(); it != stateEnt->eventMap.end(); it++)
    {
      if (timeClient.isTimeSet())
      {
        // ...
      }
      stateEnt->eventMap[it->first].cbIfTimeAndActive(stateEnt->getElapsedMs());
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

    // StateManager Loop - END
    // Handling this first instead of last; allows us to use init.loop() if we need it before switching to the requested state (or maybe we don't want to request a new state during init at all?)
    stateEnt->loop();

    // Check if state change requested and proceed if stateEnt->validateStateChange() says its ok
    int curState = getCurState();
    int requestedState = getRequestedState();
    if (curState != requestedState)
    {
      Serial.println("Handling state change request: " + stateToString(requestedState));
      if (stateEnt->validateStateChange(requestedState))
      {
        stateEnt->preStateChange(requestedState);
        // Requested state may have changed between last and next function call
        handleStateChange(requestedState);
        Serial.println("State change complete");
      }
      else
      {
        Serial.println("State change rejected by validateStateChange");
      }
    }
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

bool Base::validateStateChange(int s)
{
  return true;
}

void Base::preStateChange(int s)
{
  Serial.print("Switching to ");
  Serial.print(stateEntMap.at(s)->getName());
  Serial.println(" state now.");
  deactivateEvents();

#if MASTER
  sendStateChangeMessages(s);
#endif

  if (doSync())
  {
    setBuiltinLED(0);
  }
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
    requestedState = m.getState();
    break;
  case TYPE_HANDSHAKE_REQUEST:
    Serial.println("Handshake request message in inbox");
    receiveHandshakeRequest(m);
    sendHandshakeResponses({m.getSenderID()});
    break;
  case TYPE_HANDSHAKE_RESPONSE:
    Serial.println("Handshake response message in inbox");
    receiveHandshakeResponse(m);
    sendTimeSyncMsg({m.getSenderID()});
    break;
  case TYPE_TIME_SYNC:
    Serial.println("Time sync message in inbox");
    receiveTimeSyncMsg(m);
    break;
  case TYPE_TIME_SYNC_RESPONSE:
    Serial.println("Time sync response message in inbox");
    receiveTimeSyncMsg(m);
    break;
  case TYPE_TIME_SYNC_START:
    sync_data d;
    memcpy(&d, m.getData(), sizeof(d));
    Serial.print("Received time: ");
    Serial.println(d.ms);
    stateEnt->setSyncStartTime(convertTime(m.getSenderID(), d.ms));
    Serial.print("Converted time: ");
    Serial.println(stateEnt->getSyncStartTime());
    scheduleSyncStart();
    break;
  }

#if IMPLICIT_STATE_CHANGE
#ifndef MASTER
  if (m.getState() != curState && m.getState() != requestedState)
  {
    Serial.println("Implicit state change to " + stateToString(m.getState()));
    requestedState = m.getState();
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
    if (eventMap[it->first].getTemporary())
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
  String id = String(deviceID);
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
    if (m.getType() == TYPE_RUN_DATA)
    {
      Serial.print("X");
#if PRINT_MSG_RECEIVE
      Serial.println("Skipping TYPE_RUN_DATA message in favor of higher priority messages");
#endif
      return;
    }
  }
#endif

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
    int peerDeviceID = getMacToIDMap()[macToString(mac_addr)];
#if PRINT_MSG_SEND
    Serial.println("Delivery failed to peer ID " + String(peerDeviceID));
#else
    Serial.print("X");
#endif
    peerInfoMap[peerDeviceID].mutex.lock();
    // Check if there are more retries remaining and retry if so
    if (peerInfoMap[peerDeviceID].lastMsg.getSendCnt() - 1 < peerInfoMap[peerDeviceID].lastMsg.getMaxRetries())
    {
#if PRINT_MSG_SEND
      Serial.println("Retrying send to device ID " + String(peerDeviceID));
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
        int deviceID = SSID.substring(String(DEVICE_PREFIX).length()).toInt();
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
  if (peerInfoMap.size())
  {
    for (std::map<int, af1_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
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
  msg.setTransportType(TRANSPORT_ESPNOW);

  // If recipients set is empty then send to all
  std::set<int> recipientIDs = msg.getRecipients().size() ? msg.getRecipients() : getPeerIDs();

  if (!recipientIDs.size())
  {
#if PRINT_MSG_SEND
    Serial.println("No ESPNow peers; unable to send message");
#endif
  }

  for (std::set<int>::iterator it = recipientIDs.begin(); it != recipientIDs.end() && peerInfoMap.count(*it); it++)
  {
    peerInfoMap[*it].mutex.lock();
    // Update last msg sent for this peer (now doing this even if sending fails)
    peerInfoMap[*it].lastMsg = msg;
    // Serial.println("Sending message to device ID " + String(*it) + " (MAC address " + macToString(peerInfoMap[*it].espnowPeerInfo.peer_addr) + ")");
    af1_msg m = msg.getInnerMsg();
    esp_err_t result = esp_now_send(peerInfoMap[*it].espnowPeerInfo.peer_addr, (uint8_t *)&m, sizeof(m));
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

bool Base::doScanForPeersESPNow()
{
  return !doSync();
}

void Base::sendStateChangeMessages(int s)
{
  Serial.println("Pushing state change messages to the outbox");

  AF1Msg msg = AF1Msg();

  msg.setType(TYPE_CHANGE_STATE);
  msg.setSenderID(deviceID);
  msg.setState(s);
  msg.setMaxRetries(DEFAULT_RETRIES);

  pushOutbox(msg);
}

void Base::sendHandshakeRequests(std::set<int> ids)
{
  Serial.println("Pushing handshake requests to outbox");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_REQUEST);
  msg.setSenderID(deviceID);
  msg.setState(curState); // msg.setState(STATE_HANDSHAKE);
  msg.setData(getMacAP());
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);

  for (std::set<int>::const_iterator it = ids.begin(); it != ids.end(); it++)
  {
    peerInfoMap[*it].handshakeRequest = true;
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
  peerInfoMap[m.getSenderID()].espnowPeerInfo = ei;
  peerInfoMap[m.getSenderID()].handshakeResponse = false;
  peerInfoMap[m.getSenderID()].lastMsg = AF1Msg();
  peerInfoMap[m.getSenderID()].otherTimeSync = 0;
  peerInfoMap[m.getSenderID()].thisTimeSync = 0;

  connectToPeers();
}

void Base::sendHandshakeResponses(std::set<int> ids)
{
  Serial.println("Pushing handshake responses to outbox");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(TYPE_HANDSHAKE_RESPONSE);
  msg.setSenderID(deviceID);
  msg.setState(curState); // msg.setState(STATE_HANDSHAKE);
  // Set wrapper
  msg.setRecipients(ids);

  pushOutbox(msg);
}

void Base::receiveHandshakeResponse(AF1Msg m)
{
  Serial.println("Receiving handshake response from ID " + String(m.getSenderID()));
  peerInfoMap[m.getSenderID()].handshakeResponse = true;
}

void Base::sendAllHandshakes(bool resend)
{
  for (std::map<int, af1_peer_info>::const_iterator it = peerInfoMap.begin(); it != peerInfoMap.end() && (!it->second.handshakeRequest || resend); it++)
  {
    sendHandshakeRequests({it->first});
  }
}

void Base::sendTimeSyncMsg(std::set<int> ids, bool isResponse)
{
  Serial.print("Pushing time sync ");
  Serial.print(isResponse ? "response " : "");
  Serial.println("messages to outbox");

  AF1Msg msg = AF1Msg();

  // Set struct
  msg.setType(isResponse ? TYPE_TIME_SYNC_RESPONSE : TYPE_TIME_SYNC);
  msg.setSenderID(deviceID);
  msg.setState(curState);
  setTimeSyncMsgTime(msg); // Not sure if this is really necessary; time is now set right before sending which is better
  // Set wrapper
  msg.setRecipients(ids);
  msg.setMaxRetries(DEFAULT_RETRIES);

  pushOutbox(msg);
}

void Base::receiveTimeSyncMsg(AF1Msg m)
{
  Serial.print("Receiving time sync ");
  Serial.print(m.getType() == TYPE_TIME_SYNC_RESPONSE ? "response" : "");
  Serial.print(" msg from ID ");
  Serial.println(m.getSenderID());

  if (peerInfoMap.count(m.getSenderID()))
  {
    af1_time_sync_data d;
    memcpy(&d, m.getData(), sizeof(d));
    peerInfoMap[m.getSenderID()].otherTimeSync = d.ms;
    unsigned long thisMs = millis();
    peerInfoMap[m.getSenderID()].thisTimeSync = thisMs;
    Serial.print("Other device time: ");
    Serial.print(peerInfoMap[m.getSenderID()].otherTimeSync);
    Serial.print("; This device time: ");
    Serial.println(peerInfoMap[m.getSenderID()].thisTimeSync);

    if (m.getType() == TYPE_TIME_SYNC)
    {
      sendTimeSyncMsg({m.getSenderID()}, true);
    }
  }
  else
  {
    Serial.println("Time sync message rejected; need to handshake first");
  }
}

void Base::sendAllTimeSyncMessages()
{
  for (std::map<int, af1_peer_info>::const_iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
  {
    sendTimeSyncMsg({it->first});
  }
}

void Base::setTimeSyncMsgTime(AF1Msg &m)
{
  af1_time_sync_data d;
  d.ms = millis();
  m.setData((uint8_t *)&d);
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
      if (client && (i == curWSClientInfo || !curWSClientInfo) && i == defaultWSClientInfo)
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
          Serial.println("WS handshake successful (WS connection complete)");
          curWSClientInfo = i;
          stateEnt->onConnectWSServer();
        }
        else
        {
          Serial.println("WS handshake failed (WS connection failed halfway)");
        }
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

void Base::sendMsgInfo(std::set<int> recipients)
{
  AF1Msg msg;
  msg.setState(getCurState());
  msg.setType(TYPE_INFO);
  msg.getJson()["info"] = getCurStateEnt()->getInfo();
  msg.setRecipients(recipients);
  pushOutbox(msg);
}

std::map<String, Event> &Base::getEventMap()
{
  return eventMap;
}

unsigned long Base::getStartMs()
{
  return startMs;
}

void Base::scheduleSyncStart()
{
  unsigned long s = stateEnt->getSyncStartTime();

  Serial.println("Scheduling");
  Serial.print("Current time: ");
  Serial.print(millis());
  Serial.print("; Start time: ");
  Serial.print(s);
  Serial.print("; diff: ");
  Serial.println(s - millis());

  unsigned long dif = s - millis();
  unsigned long startMs = dif + stateEnt->getElapsedMs();

  set(Event(
      "Sync_ScheduleSyncStart", [](ECBArg a)
      {
    Serial.println("Starting");
    stateEnt->doSynced(); },
      true, 0, 1, startMs));
}

void Base::doSynced()
{
  set(Event(
      "Base_SyncStart",
      [](ECBArg a)
      { setBuiltinLED(a.cbCnt % 2); },
      true, 300));
}

void Base::setSyncStartTime(unsigned long s)
{
  syncStartTime = s;
}

unsigned long Base::getSyncStartTime()
{
  return syncStartTime;
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

void Base::registerStateEnt(int i, Base *s)
{
  stateEntMap[i] = s;
}

void Base::registerStringHandler(String s, string_input_handler h)
{
  stringHandlerMap[s] = h;
}

void Base::registerWifiAP(String s, String p)
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

void Base::registerWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd)
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

int Base::getDeviceID()
{
  return deviceID;
}

void Base::setDefaultWSClientInfo(ws_client_info w)
{
  defaultWSClientInfo = w;
}

ws_client_info Base::getDefaultWSClientInfo()
{
  return defaultWSClientInfo;
}

void Base::onConnectWSServer()
{
}

DynamicJsonDocument Base::getInfo()
{
  DynamicJsonDocument info(1024);
  info["id"] = deviceID;
#ifdef ESP32
  info["esp32"] = true; // Should always be true
#endif
  return info;
}

std::set<int> Base::getPeerIDs()
{
  std::set<int> result;
  for (std::map<int, af1_peer_info>::iterator it = peerInfoMap.begin(); it != peerInfoMap.end(); it++)
  {
    result.insert(it->first);
  }
  return result;
}

std::map<int, af1_peer_info> &Base::getPeerInfoMap()
{
  return peerInfoMap;
}

std::map<String, int> &Base::getMacToIDMap()
{
  return macToIDMap;
}

const std::map<int, Base *> &Base::getStateEntMap()
{
  return stateEntMap;
}

Base *Base::getCurStateEnt()
{
  return stateEntMap[curState];
}

int Base::getCurMode()
{
  return curMode;
}

int Base::getPrevMode()
{
  return prevMode;
}

void Base::setRequestedMode(int m)
{
  requestedMode = m;
}

int Base::getRequestedMode()
{
  return requestedMode;
}

void Base::setInitialMode(int m)
{
  initialMode = m;
}

int Base::getInitialMode()
{
  return initialMode;
}

bool Base::handleModeChange(int m)
{
  if (modeEntMap.count(m))
  {
    prevMode = curMode;
    curMode = m;
    modeEnt = modeEntMap[m];
    return modeEnt->setup();
  }
  else
  {
    return false;
  }
}

String Base::modeToString(int s)
{
  if (modeEntMap.count(s))
  {
    return modeEntMap[s]->getName();
  }
  return "Unknown mode name";
}

void Base::setCurWSClientInfo(ws_client_info i)
{
  curWSClientInfo = i;
}

ws_client_info Base::getCurWSClientInfo()
{
  return curWSClientInfo;
}

unsigned long Base::convertTime(int id, unsigned long t)
{
  if (peerInfoMap.count(id))
  {
    unsigned long dif = t - peerInfoMap[id].otherTimeSync;
    return peerInfoMap[id].thisTimeSync + dif;
  }
  return 0;
}

void Base::setIEIntervalMs(String e, unsigned long m)
{
  if (stateEnt->getEventMap().count(e))
  {
    stateEnt->getEventMap().at(e).setIntervalMs(m, stateEnt->getElapsedMs());
  }
}

void Base::set(Event e)
{
  stateEnt->getEventMap()[e.getName()] = e;
}

// StateManager - END
