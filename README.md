### Features

- :trident: State management
- :electric_plug: Websocket client
- :handshake: Peer to peer communication (ESPNOW)
- :stopwatch: Timed event handling (one-time or recurring)
- :keyboard: Serial input handling
- :radio: Over-The-Air (OTA) firmware updates
- :leaves: Lightweight
- :heavy_plus_sign: Extendable
- :racing_car: Performant

### Overview

Consumers of this framework use and extend "state entities". A state entity defines the behavior for a particular state, and new state entities can be created by extending the virtual `Base`, `WSEnt`, or `ESPNowEnt` classes (depending on the desired message handling mechanism, if any).

When creating new state entities, setup, loop, and message handling behavior from the virtual classes can be overridden as necessary. As an example, this `Demo` class extends the framework's `WSEnt` class in order to process websocket messages from a server related to LED brightness:

demo.h:

```
#include <framework.h>

class Demo : public WSEnt
{
  void setup(); // override
  static bool handleInboxMsg(JSMessage m); // override
  void setInboxMessageHandler(); // override
};
```

demo.cpp:

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

### Getting Started

main.cpp

```

#include <Arduino.h>
#include <framework.h>
#include "stateEnt/demo/demo.h"

// User-defined states
enum js_state_extended
{
  STATE_DEMO
};

void setStateDemo()
{
  StateManager::getInstance().setRequestedState(STATE_DEMO);
}

void setup()
{
  Serial.begin(115200);
  Framework::setup(); // REQUIRED
  Framework::registerStateEnt(STATE_DEMO, new Demo(), "STATE_DEMO");
  // The state is changed to STATE_DEMO when "4" is entered into the serial monitor
  Framework::registerStringHandler("4", setStateDemo);
}

void loop()
{
  Framework::loop(); // REQUIRED
}

```
