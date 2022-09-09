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

#define DEVICE_PREFIX "JS"
#define DEVICE_AP_PASS "1234567890"
#define PRINT_WIFI_SCAN_RESULTS false
#define PRINT_MSG_SEND false
#define PRINT_MGG_RECEIVE false

#define ESPNOW_CHANNEL 0
#define SLAVE_CNT 4
#define DEFAULT_RETRIES 3
#define RETRIES_PURG 10

#define DELAY_PREPARE_WIFI 333
#define DELAY_SEND 2
#define MS_PURG_DEFAULT 999
#define MS_MASTER_HANDSHAKE_LOOP 333

#define STATE_WS_FALLBACK STATE_IDLE_BASE
#define STATE_ESPNOW_FALLBACK STATE_IDLE_BASE

#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s
