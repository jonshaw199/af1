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
  TYPE_TIME_SYNC,
  TYPE_TIME_SYNC_RESPONSE,
  TYPE_TIME_SYNC_START,
  // MQTT
  TYPE_MQTT_SUBSCRIBE,
  TYPE_MQTT_SUBACK,
  TYPE_MQTT_UNSUBSCRIBE,
  TYPE_MQTT_UNSUBACK,
  TYPE_MQTT_PUBLISH,
  // QOS 1
  TYPE_MQTT_PUBACK,
  // QOS 2
  TYPE_MQTT_PUBREC,
  TYPE_MQTT_PUBREL,
  TYPE_MQTT_PUBCOMP,
};

class AF1Msg
{
  AF1JsonDoc jsonDoc;
  uint8_t *raw;
  int rawLen;
  bool isTxt;
  std::set<String> recipients;
  int sendCnt;
  int retries;
  int maxRetries;

public:
  AF1Msg();
  AF1Msg(uint8_t type);
  AF1Msg(JsonDocument &d);
  AF1Msg(uint8_t *raw, int rawLen, bool isTxt = false);
  AF1Msg(uint8_t type, uint8_t *raw, int rawLen, bool isTxt = false);
  AF1Msg(const AF1Msg &m); // Copy
  AF1Msg(AF1Msg &&m);      // Move
  ~AF1Msg();
  AF1Msg &operator=(const AF1Msg &m); // Copy
  AF1Msg &operator=(AF1Msg &&m);      // Move

  JsonDocument &json();
  const uint8_t *getRaw();
  int getRawLen();
  bool getIsTxt();
  uint8_t getType();
  uint8_t getState();
  String getSenderId();

  void setRecipients(std::set<String> r);
  std::set<String> getRecipients();
  int incrementSendCnt();
  int getSendCnt();
  void setMaxRetries(int m);
  int getMaxRetries();
  int getRetries();
  void print() const;
};

#endif // MESSAGE_MESSAGE_H_
