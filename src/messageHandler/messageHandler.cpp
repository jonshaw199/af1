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

#include "messageHandler.h"
#include "pre.h"

MessageHandler::MessageHandler()
{
}

MessageHandler &MessageHandler::getInstance()
{
  static MessageHandler instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
  return instance;
}

const TSQueue<JSMessage> &MessageHandler::getOutbox()
{
  return getInstance().outbox;
}

const TSQueue<JSMessage> &MessageHandler::getInbox()
{
  return getInstance().inbox;
}

void MessageHandler::handleInboxMessages()
{
  getInstance().inbox.handleMessages();
}

void MessageHandler::handleOutboxMessages()
{
  getInstance().outbox.handleMessages();
}

void MessageHandler::setInboxMsgHandler(msg_handler h)
{
  getInstance().inbox.setMsgHandler(h);
}

void MessageHandler::setOutboxMsgHandler(msg_handler h)
{
  getInstance().outbox.setMsgHandler(h);
}

void MessageHandler::pushOutbox(JSMessage m)
{
  getInstance().outbox.enqueue(m);
}

void MessageHandler::pushInbox(JSMessage m)
{
  getInstance().inbox.enqueue(m);
}

void MessageHandler::loop()
{
  handleOutboxMessages();
  handleInboxMessages();
}
