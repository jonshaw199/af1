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
  rawLen = 0;
  isTxt = false;
  raw = NULL;
  jsonDoc["state"] = Base::getCurState();
  jsonDoc["type"] = TYPE_NONE;
  jsonDoc["senderId"] = Base::getDeviceID();
}

AF1Msg::AF1Msg(uint8_t t) : AF1Msg()
{
  jsonDoc["type"] = t;
}

AF1Msg::AF1Msg(JsonDocument &d) : AF1Msg()
{
  jsonDoc = d;
}

AF1Msg::AF1Msg(uint8_t *r, int l, bool t)
{
  recipients = {};
  sendCnt = 0;
  retries = 0;
  maxRetries = 0;
  rawLen = l ? l + 1 : 0;
  isTxt = t;
  raw = new uint8_t[l + 1];
  memcpy(raw, r, l);
  raw[l] = 0;
}

AF1Msg::AF1Msg(const AF1Msg &m)
{
  recipients = m.recipients;
  sendCnt = m.sendCnt;
  retries = m.retries;
  maxRetries = m.maxRetries;
  rawLen = m.rawLen;
  isTxt = m.isTxt;
  jsonDoc = m.jsonDoc;
  if (m.raw != NULL)
  {
    raw = new uint8_t[m.rawLen];
    memcpy(raw, m.raw, m.rawLen);
  }
  else
  {
    raw = NULL;
  }
}

AF1Msg::~AF1Msg()
{
  if (raw != NULL)
  {
    delete[] raw;
    raw = NULL;
  }
}

AF1Msg &AF1Msg::operator=(const AF1Msg &other)
{
  AF1Msg temp(other);
  std::swap(*this, temp);
  return *this;
}

JsonDocument &AF1Msg::json()
{
  return jsonDoc;
}

const uint8_t *AF1Msg::getRaw()
{
  return raw;
}

int AF1Msg::getRawLen()
{
  return rawLen;
}

bool AF1Msg::getIsTxt()
{
  return isTxt;
}

uint8_t AF1Msg::getType()
{
  return jsonDoc["type"];
}

uint8_t AF1Msg::getState()
{
  return jsonDoc["state"];
}

String AF1Msg::getSenderId()
{
  return jsonDoc["senderId"];
}

std::set<String> AF1Msg::getRecipients()
{
  return recipients;
}

void AF1Msg::setRecipients(std::set<String> r)
{
  recipients = r;
  for (std::set<String>::iterator it = r.begin(); it != r.end(); it++)
  {
    recipients.insert(*it);
  }
}

int AF1Msg::incrementSendCnt()
{
  return sendCnt++;
}

int AF1Msg::getSendCnt()
{
  return sendCnt;
}

void AF1Msg::setMaxRetries(int m)
{
  maxRetries = m;
}

int AF1Msg::getMaxRetries()
{
  return maxRetries;
}

int AF1Msg::getRetries()
{
  return retries;
}

void AF1Msg::print()
{
  Serial.print("Message: ");
  if (raw != NULL)
  {
    if (isTxt)
    {
      Serial.println((char *)raw);
    }
    else
    {
      Serial.print("length");
      Serial.println(rawLen);
    }
  }
  else
  {
    String j;
    serializeJsonPretty(jsonDoc, j);
    Serial.println(j);
  }
}
