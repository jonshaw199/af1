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

#include "message.h"
#include "stateManager/stateManager.h"

AF1Msg::AF1Msg() : json(1024)
{
  msg = {};
  msg.state = STATE_IDLE_BASE;
  msg.senderID = StateManager::getDeviceID();
  msg.type = TYPE_NONE;
  msg.transportType = TRANSPORT_NONE;

  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  json["state"] = msg.state;
  json["type"] = msg.type;
  json["senderID"] = msg.senderID;
  json["transportType"] = msg.transportType;
}

AF1Msg::AF1Msg(af1_msg m) : json(1024)
{
  msg = m;

  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  json["state"] = msg.state;
  json["type"] = msg.type;
  json["senderID"] = msg.senderID;
  json["transportType"] = msg.transportType;
}

AF1Msg::AF1Msg(DynamicJsonDocument d) : json(1024)
{
  msg = {};
  msg.state = d["state"];
  msg.senderID = d["senderID"];
  msg.type = d["type"];
  msg.transportType = d["transportType"];

  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  json = d;
}

std::set<int> AF1Msg::getRecipients()
{
  return recipients;
}

void AF1Msg::setRecipients(std::set<int> r)
{
  recipients = r;
}

int AF1Msg::incrementSendCnt()
{
  return sendCnt++;
}

int AF1Msg::getSendCnt()
{
  return sendCnt;
}

void AF1Msg::setType(int t)
{
  msg.type = t;
  json["type"] = t;
}

int AF1Msg::getType()
{
  return msg.type;
}

void AF1Msg::setState(int s)
{
  msg.state = s;
  json["state"] = s;
}

int AF1Msg::getState()
{
  return msg.state;
}

void AF1Msg::setSenderID(int id)
{
  msg.senderID = id;
  json["senderID"] = id;
}

int AF1Msg::getSenderID()
{
  return msg.senderID;
}

void AF1Msg::setMaxRetries(int m)
{
  maxRetries = m;
}

int AF1Msg::getMaxRetries()
{
  return maxRetries;
}

void AF1Msg::setTransportType(int t)
{
  msg.transportType = t;
  json["transportType"] = t;
}

int AF1Msg::getTransportType()
{
  return msg.transportType;
}

void AF1Msg::setData(uint8_t *d)
{
  memcpy(msg.data, d, sizeof(msg.data));
}

const uint8_t *AF1Msg::getData()
{
  return msg.data;
}

void AF1Msg::setJson(DynamicJsonDocument d)
{
  json = d;
}

DynamicJsonDocument &AF1Msg::getJson()
{
  return json;
}

void AF1Msg::print()
{
  String j;
  serializeJsonPretty(json, j);
  Serial.print("Message: ");
  Serial.println(j);
}

af1_msg AF1Msg::getInnerMsg()
{
  return msg;
}

void AF1Msg::deserializeInnerMsgESPNow()
{
  if (StateManager::getStateEntMap().count(msg.state))
  {
    StateManager::getStateEntMap().at(msg.state)->deserializeESPNow(*this);
  }
  else
  {
#if PRINT_MSG_RECEIVE
    Serial.println("AF1Msg::deserializeInnerMsg(): state doesnt exist");
#endif
  }
}

void AF1Msg::serializeInnerMsgESPNow()
{
  if (StateManager::getStateEntMap().count(msg.state))
  {
    StateManager::getStateEntMap().at(msg.state)->serializeESPNow(*this);
  }
  else
  {
#if PRINT_MSG_SEND
    Serial.println("AF1Msg::serializeInnerMsg(): state doesnt exist");
#endif
  }
}
