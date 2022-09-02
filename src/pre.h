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

#define AF1 StateManager

#ifndef INITIAL_STATE
#define INITIAL_STATE INITIAL_STATE_DEF
#endif

#ifndef STATE_AFTER_HANDSHAKE
#define STATE_AFTER_HANDSHAKE STATE_AFTER_HANDSHAKE_DEF
#endif

#ifndef WS_HOST
#define WS_HOST WS_HOST_DEF
#endif

#ifndef WS_PATH
#define WS_PATH WS_PATH_DEF
#endif

#ifndef WS_PORT
#define WS_PORT WS_PORT_DEF
#endif

#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s
