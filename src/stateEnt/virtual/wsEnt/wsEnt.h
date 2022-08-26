#ifndef STATEENT_VIRTUAL_WSENT_WSENT_H_
#define STATEENT_VIRTUAL_WSENT_WSENT_H_

#include <WebSocketClient.h>

#include "stateEnt/virtual/base/base.h"

class WSEnt : public Base
{
public:
  static WebSocketClient webSocketClient;
  // Use WiFiClient class to create TCP connections
  static WiFiClient client;
  void setup();
  void loop();
};

#endif