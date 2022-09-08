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

#ifndef BOX_BOX_H_
#define BOX_BOX_H_

#include "tsQueue/tsQueue.h"
#include "message/message.h"

typedef bool (*msg_handler)(AF1Msg m);

class Box : public TSQueue<AF1Msg>
{
  msg_handler msgHandler;

public:
  Box();
  void handleMessages();
  void setMsgHandler(msg_handler h);
};

#endif // BOX_BOX_H_
