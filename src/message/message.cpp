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

JSMessage::JSMessage() : json(1024)
{
  msg = {};
  msg.state = STATE_NONE;
  msg.senderID = JS_ID;
  msg.type = TYPE_NONE;

  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  json["state"] = msg.state;
  json["type"] = msg.type;
  json["senderID"] = msg.senderID;
}

JSMessage::JSMessage(js_message m) : json(1024)
{
  msg = m;

  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  json["state"] = msg.state;
  json["type"] = msg.type;
  json["senderID"] = JS_ID;
}

JSMessage::JSMessage(DynamicJsonDocument d) : json(1024)
{
  msg = {};
  msg.state = d["state"];
  msg.senderID = JS_ID; // d["senderID"];
  msg.type = d["type"];

  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  json = d;
  json["senderID"] = msg.senderID;
}

std::set<int> JSMessage::getRecipients()
{
  return recipients;
}

void JSMessage::setRecipients(std::set<int> r)
{
  recipients = r;
}

int JSMessage::incrementSendCnt()
{
  return sendCnt++;
}

int JSMessage::getSendCnt()
{
  return sendCnt;
}

void JSMessage::setType(int t)
{
  msg.type = t;
  json["type"] = t;
}

int JSMessage::getType()
{
  return msg.type;
}

void JSMessage::setState(int s)
{
  msg.state = s;
  json["state"] = s;
}

int JSMessage::getState()
{
  return msg.state;
}

void JSMessage::setSenderID(int id)
{
  msg.senderID = id;
  json["senderID"] = id;
}

int JSMessage::getSenderID()
{
  return msg.senderID;
}

void JSMessage::setMaxRetries(int m)
{
  maxRetries = m;
}

int JSMessage::getMaxRetries()
{
  return maxRetries;
}

void JSMessage::setData(uint8_t *d)
{
  memcpy(msg.data, d, sizeof(msg.data));
}

const uint8_t *JSMessage::getData()
{
  return msg.data;
}

void JSMessage::setJson(DynamicJsonDocument d)
{
  json = d;
}

DynamicJsonDocument JSMessage::getJson()
{
  return json;
}

void JSMessage::print()
{
  String j;
  serializeJsonPretty(json, j);
  Serial.print("Message: ");
  Serial.println(j);
}
