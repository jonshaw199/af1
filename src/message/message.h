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

#ifndef MESSAGE_MESSAGE_H_
#define MESSAGE_MESSAGE_H_

#include <Arduino.h>
#include <set>

#include "state/state.h"

enum MessageType
{
  TYPE_NONE,
  TYPE_HANDSHAKE_REQUEST,
  TYPE_HANDSHAKE_RESPONSE,
  TYPE_CHANGE_STATE,
  TYPE_RUN_DATA
};

typedef struct js_espnow_message
{
  // Calculated
  // int msgID;
  // Required
  int type;
  int senderID;
  int state;
  // State dependent
  uint8_t data[100];
} js_espnow_message;

// This class is a wrapper around the js_espnow_message struct that actually gets sent using ESPNOW
class JSMessage
{
  js_espnow_message msg;
  std::set<int> recipients;
  int sendCnt;
  int retries;
  int maxRetries;

public:
  JSMessage();
  JSMessage(js_espnow_message m);
  void setRecipients(std::set<int> r);
  std::set<int> getRecipients();
  js_espnow_message asStruct();
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
};

#endif // MESSAGE_MESSAGE_H_
