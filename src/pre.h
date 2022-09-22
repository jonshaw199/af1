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

#ifndef DEVICE_PREFIX
#define DEVICE_PREFIX "ESP32-AF1-"
#endif

#ifndef DEVICE_AP_PASS
#define DEVICE_AP_PASS "1234567890"
#endif

#ifndef PRINT_WIFI_SCAN_RESULTS
#define PRINT_WIFI_SCAN_RESULTS false
#endif

#ifndef PRINT_MSG_SEND
#define PRINT_MSG_SEND false
#endif

#ifndef PRINT_MSG_RECEIVE
#define PRINT_MGG_RECEIVE false
#endif

#ifndef ESPNOW_CHANNEL
#define ESPNOW_CHANNEL 0
#endif

#ifndef DEFAULT_RETRIES
#define DEFAULT_RETRIES 3
#endif

#ifndef RETRIES_PURG
#define RETRIES_PURG 10
#endif

#ifndef DELAY_PREPARE_WIFI
#define DELAY_PREPARE_WIFI 333
#endif

#ifndef DELAY_SEND
#define DELAY_SEND 0
#endif

#ifndef MS_PURG_DEFAULT
#define MS_PURG_DEFAULT 999
#endif

#ifndef MS_HANDSHAKE_LOOP
#define MS_HANDSHAKE_LOOP 30000
#endif

#ifndef MS_TIME_SYNC
#define MS_TIME_SYNC 0
#endif

#ifndef STATE_WS_FALLBACK
#define STATE_WS_FALLBACK STATE_IDLE_BASE
#endif

#ifndef STATE_ESPNOW_FALLBACK
#define STATE_ESPNOW_FALLBACK STATE_IDLE_BASE
#endif

#ifndef MODE_INITIAL
#define MODE_INITIAL MODE_BASIC
#endif

#ifndef SAFETY_CHECK_INBOX_OVERFLOW
#define SAFETY_CHECK_INBOX_OVERFLOW true
#endif

#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s
