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
  TYPE_NONE = 0,
  TYPE_HANDSHAKE_REQUEST = 1,
  TYPE_HANDSHAKE_RESPONSE = 2,
  TYPE_CHANGE_STATE = 3,
  TYPE_RUN_DATA = 4
};

typedef struct js_message
{
  // Calculated
  // int msgID;
  // Required, even for WS (single source of truth)
  int type;
  int senderID;
  int state;
  // State dependent and only used for espnow
  uint8_t data[100];
} js_message;

// This class is a wrapper around the js_message struct that actually gets sent using ESPNOW
// ... now also wraps json that gets sent using WS
class JSMessage
{
  js_message msg;
  DynamicJsonDocument json;
  std::set<int> recipients;
  int sendCnt;
  int retries;
  int maxRetries;

public:
  JSMessage();
  JSMessage(js_message m);
  JSMessage(DynamicJsonDocument d);
  void setRecipients(std::set<int> r);
  std::set<int> getRecipients();
  int incrementSendCnt();
  int getSendCnt();
  void setType(int t);
  int getType();
  void setState(int s);
  int getState();
  void setSenderID(int id);
  int getSenderID();
  void setMaxRetries(int m);
  int getMaxRetries();
  void setData(uint8_t *d);
  const uint8_t *getData();
  void setJson(DynamicJsonDocument d);
  DynamicJsonDocument getJson();
};

#endif // MESSAGE_MESSAGE_H_
