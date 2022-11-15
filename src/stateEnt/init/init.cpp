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

#include <WiFi.h>

#include "init.h"
#include "pre.h"

Init::Init()
{
  set(Event(
      "Global_ESPHandshake", [](ECBArg a)
      {
    if (getCurStateEnt()->doScanForPeersESPNow())
    {
      handleHandshakes();
    }
    else
    {
      Serial.println("ESPNow peer scan denied in current state");
    } },
      EVENT_TYPE_GLOBAL, MS_HANDSHAKE_LOOP, 0, 0, START_DEVICE_MS));

#if MASTER
  set(Event(
      "Global_SendSyncStartTime", [](ECBArg a)
      {
    if (getCurStateEnt()->doSync())
    {
      syncStartTime = millis() + (unsigned long)MS_TIME_SYNC_START;
      AF1Msg msg(TYPE_TIME_SYNC_START);
      msg.json()["timeSyncStart"] = syncStartTime;
      pushOutbox(msg);
      Serial.println("Scheduling sync start");
      scheduleSyncStart();
    } },
      EVENT_TYPE_GLOBAL, MS_TIME_SYNC_SCHEDULE_START, 1));
#endif
}

void Init::setup()
{
  Serial.println();
  Serial.println("*********");
  Serial.print("JS ID: ");
  Serial.println(getDeviceID());

  // WifiHandler init
  WiFi.mode(WIFI_MODE_APSTA);
  broadcastAP(); // For AP config like SSID
  WiFi.softAPmacAddress(macAP);
  Serial.print("MAC AP: ");
  printMac(macAP);
  WiFi.macAddress(macSTA);
  Serial.print("MAC STA: ");
  printMac(macSTA);

  Serial.println("*********");
  Serial.println();

  Base::setup();

  initEspNow();

#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif
  // setupDisplay();
}

void Init::loop()
{
  setRequestedState(getInitialState());
}

String Init::getName()
{
  return "STATE_INIT";
}
