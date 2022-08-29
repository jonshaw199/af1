# AF1 - An Arduino extension framework for ESP32

## Getting Started

### Features

- :trident: State management
- :electric_plug: Websocket client
- :handshake: Peer to peer communication (ESPNOW)
- :stopwatch: Event scheduling (one-time or recurring)
- :keyboard: Serial input handling
- :radio: Over-The-Air (OTA) firmware updates
- :leaves: Lightweight
- :heavy_plus_sign: Extendable
- :racing_car: Performant

### Overview

Consumers of this framework use and extend "state entities". A state entity defines the behavior for a particular state, and new state entities can be created by extending the virtual `Base`, `WSEnt`, or `ESPNowEnt` classes (depending on the desired message handling mechanism, if any).

When creating new state entities, setup, loop, and message handling behavior from the virtual classes can be overridden as necessary. As an example, this `Demo` class extends the framework's `WSEnt` class in order to process websocket messages from a server related to LED brightness:

**demo.h:**

```
#include <AF1.h>

class Demo : public WSEnt
{
  void setup(); // override
  static bool handleInboxMsg(JSMessage m);
  void setInboxMessageHandler(); // override
};
```

**demo.cpp:**

```
#include "demo.h"
#include "led/led.h"

void Demo::setup()
{
  WSEnt::setup();
  JSLED::init();
}

bool Demo::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_RUN_DATA:
    uint8_t b = m.getJson()["brightness"];
    JSLED::setBrightness(b);
    return true;
  }

  return WSEnt::handleInboxMsg(m);
}

void Demo::setInboxMessageHandler()
{
  setInboxMsgHandler(handleInboxMsg);
}
```

**main.cpp**

```
#include <Arduino.h>
#include <AF1.h>
#include "stateEnt/demo/demo.h"

enum user_defined_states
{
  STATE_DEMO
};

void setStateDemo()
{
  AF1::setRequestedState(STATE_DEMO);
}

void setup()
{
  Serial.begin(115200);
  AF1::setup(); // REQUIRED
  AF1::registerStateEnt(STATE_DEMO, new Demo(), "STATE_DEMO");
  // The state is changed to STATE_DEMO when "4" is entered into the serial monitor
  AF1::registerStringHandler("4", setStateDemo);
}

void loop()
{
  AF1::loop(); // REQUIRED
}

```

**Build flags also need to be set for Wifi SSID/password, websocket configuration, etc. (see `library.json`)**

## API Docs

### Scheduling Events

```
class Demo2 : public Base
{
  int intervalMs = 3000;
  bool demoCb(IECBArg a)
  {
    // Do something here every 3 seconds indefinitely
    return true;
  }

public:
  Demo2()
  {
    intervalEvents.push_back(IntervalEvent(intervalMs, demoCb/*, maxCbCnt*/));
  }
}
```
