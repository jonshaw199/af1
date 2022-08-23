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

#ifndef STATEENT_HANDSHAKE_MASTER_MASTERHANDSHAKE_H_
#define STATEENT_HANDSHAKE_MASTER_MASTERHANDSHAKE_H_

#include "stateEnt/virtual/espnowEnt/espnowEnt.h"

class MasterHandshake : public ESPNowEnt
{
  void setup();
  void loop();
};

#endif // STATEENT_HANDSHAKE_MASTER_MASTERHANDSHAKE_H_