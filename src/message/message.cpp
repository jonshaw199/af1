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
#include "stateEnt/virtual/base/base.h"

AF1Msg::AF1Msg()
{
  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;

  data["state"] = Base::getCurState();
  data["type"] = TYPE_NONE;
  data["senderId"] = Base::getDeviceID();
}

AF1Msg::AF1Msg(uint8_t t) : AF1Msg() {
  data["type"] = t;
}

AF1Msg::AF1Msg(uint8_t t, uint8_t s) : AF1Msg() {
  data["type"] = t;
  data["state"] = s;
}

AF1Msg::AF1Msg(AF1JsonDoc d) : AF1Msg() {
  data = d;
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

void AF1Msg::setType(uint8_t t)
{
  data["type"] = t;
}

uint8_t AF1Msg::getType()
{
  return data["type"];
}

void AF1Msg::setState(uint8_t s)
{
  data["state"] = s;
}

uint8_t AF1Msg::getState()
{
  return data["state"];
}

void AF1Msg::setSenderId(uint8_t id)
{
  data["senderId"] = id;
}

uint8_t AF1Msg::getSenderId()
{
  return data["senderId"];
}

void AF1Msg::setMaxRetries(int m)
{
  maxRetries = m;
}

int AF1Msg::getMaxRetries()
{
  return maxRetries;
}

AF1JsonDoc &AF1Msg::json()
{
  return data;
}

void AF1Msg::print()
{
  String j;
  serializeJsonPretty(data, j);
  Serial.print("Message: ");
  Serial.println(j);
}
