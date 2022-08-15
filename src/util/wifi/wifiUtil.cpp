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

#include <WiFi.h>

#include "wifiUtil.h"
#include "util/stringify/stringify.h"

void WifiUtil::prepareWifi()
{
  WiFi.disconnect(true);
  delay(DELAY_PREPARE_WIFI);
}

void WifiUtil::setSTAMode()
{
  Serial.println("Setting wifi mode to STA");
  WiFi.mode(WIFI_STA);
}

void WifiUtil::setAPMode()
{
  Serial.println("Setting wifi mode to AP");
  WiFi.mode(WIFI_AP);
}

// Setup access point (aka open wifi network); this is used by slaves so master can find them
bool WifiUtil::broadcastAP()
{
  Serial.println("Broadcasting soft AP");
  String Prefix = STRINGIFY(DEVICE_PREFIX);
  String id = String(JS_ID);
  String SSID = Prefix + id;
  String Password = STRINGIFY(DEVICE_AP_PASS);
  return WiFi.softAP(SSID.c_str(), Password.c_str(), ESPNOW_CHANNEL, 0);
}

String WifiUtil::macToString(const uint8_t *m)
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

void WifiUtil::printMac(const uint8_t *m)
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
