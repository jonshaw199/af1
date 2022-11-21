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

#define AF1 Base

#ifndef DEVICE_PREFIX
#define DEVICE_PREFIX "AF1-"
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
#define RETRIES_PURG 5
#endif

#ifndef DELAY_PREPARE_WIFI
#define DELAY_PREPARE_WIFI 333
#endif

// WARNING! Some delay is necessary (3ms min?; try 1 and 2 sometime)
#ifndef DELAY_SEND
#define DELAY_SEND 3
#endif

#ifndef MS_PURG_DEFAULT
#define MS_PURG_DEFAULT 999
#endif

#ifndef MS_HANDSHAKE_INITIAL
#define MS_HANDSHAKE_INITIAL 9000
#endif

#ifndef MS_HANDSHAKE_LOOP
#define MS_HANDSHAKE_LOOP 30000
#endif

#ifndef MS_TIME_SYNC
#define MS_TIME_SYNC 30000
#endif

#ifndef MS_TIME_SYNC_SCHEDULE_START
#define MS_TIME_SYNC_SCHEDULE_START 2000
#endif

#ifndef MS_TIME_SYNC_START
#define MS_TIME_SYNC_START 3000
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

#define AF1_MSG_SIZE 225
#define AF1JsonDoc StaticJsonDocument<AF1_MSG_SIZE>

#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s

#define PREFS_NAMESPACE "af1"
#define PREFS_ID_KEY "id"
#define PREFS_WIFI_SSID_1_KEY "ssid1"
#define PREFS_WIFI_PASS_1_KEY "pass1"
#define PREFS_WIFI_SSID_2_KEY "ssid2"
#define PREFS_WIFI_PASS_2_KEY "pass2"
#define PREFS_WIFI_SSID_3_KEY "ssid3"
#define PREFS_WIFI_PASS_3_KEY "pass3"
#define PREFS_IP_KEY "ip"

#define SHKEY_CONFIG_START "c"

#define SHKEY_CONFIG "config"
#define SHKEY_OTA "ota"
#define SHKEY_RESTART "restart"
#define SHKEY_IDLE "idle"
#define SHKEY_SYNCTEST "synctest"

#define SHKEY_HANDSHAKE "hs"
#define SHKEY_DETACH "detach*"
