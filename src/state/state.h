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

#ifndef STATE_STATE_H_
#define STATE_STATE_H_

enum js_state
{
  STATE_NONE = 100,
  STATE_INIT = 101,
  STATE_PURG_OTA = 102,
  STATE_OTA = 103,
  STATE_IDLE = 104,
  STATE_PURG_RESTART = 105,
  STATE_RESTART = 106,
  STATE_HANDSHAKE = 107
};

#endif // STATE_STATE_H