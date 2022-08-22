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

#ifndef FRAMEWORK_FRAMEWORK_H_
#define FRAMEWORK_FRAMEWORK_H_

#include "stateEnt/virtual/base/base.h"

#include "stateManager/stateManager.h"
#include "messageHandler/messageHandler.h"
#include "flags.h"

class Framework
{
public:
  static void setup();
  static void loop();
  static void registerStateEnt(int i, Base *s, String n);
  static void registerStringHandler(String s, string_input_handler h);
};

#endif // FRAMEWORK_FRAMEWORK_H_