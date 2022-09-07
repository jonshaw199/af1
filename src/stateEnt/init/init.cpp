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
#include "stateManager/stateManager.h"
#include "pre.h"

void Init::setup()
{
  Serial.println();
  Serial.println("*********");
  Serial.print("JS ID: ");
  Serial.println(StateManager::getDeviceID());

  // WifiHandler init
  setAPMode();
  WiFi.softAPmacAddress(macAP);
  Serial.print("MAC AP: ");
  printMac(macAP);
  setSTAMode();
  WiFi.macAddress(macSTA);
  Serial.print("MAC STA: ");
  printMac(macSTA);

  Serial.println("*********");
  Serial.println();

  Base::setup();
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
#endif
  // setupDisplay();
}

void Init::loop()
{
  StateManager::setRequestedState(StateManager::getInitialState());
}
