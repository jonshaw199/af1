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

#ifndef MESSAGEHANDLER_MESSAGEHANDLER_H_
#define MESSAGEHANDLER_MESSAGEHANDLER_H_

#include <map>
#include <queue>
#include <set>
#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include <mutex>

#include "message/message.h"
#include "box/box.h"

class MessageHandler
{
  MessageHandler(); // constructor
  Box inbox;
  Box outbox;

public:
  static MessageHandler &getInstance();
  static const TSQueue<JSMessage> &getOutbox(); // Read only
  static const TSQueue<JSMessage> &getInbox();  // Read only
  static void handleInboxMessages();
  static void handleOutboxMessages();
  static void setInboxMsgHandler(msg_handler h);
  static void setOutboxMsgHandler(msg_handler h);
  static void pushOutbox(JSMessage m);
  static void pushInbox(JSMessage m);
  static void loop();
};

#endif // MESSAGEHANDLER_MESSAGEHANDLER_H_
