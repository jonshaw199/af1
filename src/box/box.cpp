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

#include <Arduino.h>

#include "box/box.h"

bool dummyHandler(AF1Msg m)
{
  return false;
}

Box::Box()
{
  msgHandler = dummyHandler;
}

void Box::handleMessages()
{
  m.lock();
  if (!q.empty())
  {
    AF1Msg msg = q.front();
    q.pop();
    m.unlock();
    msgHandler(msg);
    handleMessages();
  }
  else
  {
    m.unlock();
  }
}

void Box::setMsgHandler(msg_handler m)
{
  msgHandler = m;
}
