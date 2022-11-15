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
#include <pre.h>

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
  TYPE_TIME_SYNC_START,
  TYPE_INFO
};

class AF1Msg
{
  AF1JsonDoc data;
  std::set<int> recipients;
  int sendCnt;
  int retries;
  int maxRetries;

public:
  AF1Msg();
  AF1Msg(uint8_t type);
  AF1Msg(uint8_t type, uint8_t state);
  AF1Msg(AF1JsonDoc d);
  void setRecipients(std::set<int> r);
  std::set<int> getRecipients();
  int incrementSendCnt();
  int getSendCnt();
  void setType(uint8_t t);
  uint8_t getType();
  void setState(uint8_t s);
  uint8_t getState();
  void setSenderId(uint8_t id);
  uint8_t getSenderId();
  void setMaxRetries(int m);
  int getMaxRetries();
  void setData(AF1JsonDoc d);
  AF1JsonDoc &getData();
  void print();
};

#endif // MESSAGE_MESSAGE_H_
