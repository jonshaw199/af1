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

#ifndef STATEENT_VIRTUAL_SYNC_SYNC_H_
#define STATEENT_VIRTUAL_SYNC_SYNC_H_

#include <AF1.h>

typedef struct sync_data
{
  unsigned long ms;
} sync_data;

class STArg
{
  const IECBArg iecbArg;

public:
  STArg(IECBArg a);
  IECBArg getIECBArg();
};

class Sync : public Base
{
  unsigned long startTime;
  static void scheduleStart();

protected:
public:
  Sync();
  static Sync *getInstance();
  virtual void doSynced(STArg a);
  msg_handler getInboxHandler();
  bool doScanForPeersESPNow();
  void preStateChange(int s);
};

#endif