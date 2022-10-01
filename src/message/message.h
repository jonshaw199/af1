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

#ifndef MESSAGE_MESSAGE_H_
#define MESSAGE_MESSAGE_H_

#include <Arduino.h>
#include <set>
#include <ArduinoJson.h>

#include "state/state.h"

enum MessageType
{
  TYPE_NONE = 100,
  TYPE_HANDSHAKE_REQUEST,
  TYPE_HANDSHAKE_RESPONSE,
  TYPE_CHANGE_STATE,
  TYPE_RUN_DATA,
  TYPE_RC_DATA,
  TYPE_TIME_SYNC,
  TYPE_TIME_SYNC_RESPONSE,
  TYPE_TIME_SYNC_START
};

enum TransportType
{
  TRANSPORT_ESPNOW,
  TRANSPORT_WEBSOCKET,
  TRANSPORT_NONE
};

typedef struct af1_msg
{
  // Calculated
  // int msgID;
  // Required, even for WS (single source of truth)
  uint8_t type;
  uint8_t senderID;
  uint8_t state;
  uint8_t transportType;
  // State dependent and only used for espnow
  uint8_t data[250];
} af1_msg;

typedef struct af1_time_sync_data
{
  unsigned long ms;
} af1_time_sync_data;

// This class is a wrapper around the af1_msg struct that actually gets sent using ESPNOW
// ... now also wraps json that gets sent using WS
class AF1Msg
{
  af1_msg msg;
  DynamicJsonDocument json;
  std::set<int> recipients;
  int sendCnt;
  int retries;
  int maxRetries;

public:
  AF1Msg();
  AF1Msg(af1_msg m);
  AF1Msg(DynamicJsonDocument d);
  void setRecipients(std::set<int> r);
  std::set<int> getRecipients();
  int incrementSendCnt();
  int getSendCnt();
  void setType(uint8_t t);
  uint8_t getType();
  void setState(uint8_t s);
  uint8_t getState();
  void setSenderID(uint8_t id);
  uint8_t getSenderID();
  uint8_t getTransportType();
  void setTransportType(uint8_t t);
  void setMaxRetries(int m);
  int getMaxRetries();
  void setData(uint8_t *d);
  const uint8_t *getData();

  void setJson(DynamicJsonDocument d);
  DynamicJsonDocument &getJson();

  af1_msg getInnerMsg();
  void deserializeInnerMsgESPNow();
  void serializeInnerMsgESPNow();

  void print();
};

#endif // MESSAGE_MESSAGE_H_
