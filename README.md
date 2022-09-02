# AF1 - An Arduino/ESP32 Framework

## Getting Started

### Features

- :trident: [State Management](#overview)
- :electric_plug: [Websocket Client](#overview)
- :handshake: ESP-Now P2P Communication
- :stopwatch: [Event Scheduling](#event-scheduling)
- :radio: Over-The-Air (OTA) firmware updates
- :keyboard: [Serial Input Handling](#overview)
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

void Demo::setInboxMessageHandler()
{
  setInboxMsgHandler([](JSMessage m) {
    switch (m.getType())
    {
    case TYPE_RUN_DATA:
      uint8_t b = m.getJson()["brightness"];
      JSLED::setBrightness(b);
      return true;
    }

    return WSEnt::handleInboxMsg(m);
  });
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

void setup()
{
  Serial.begin(115200);
  AF1::setup(); // REQUIRED
  AF1::registerWifiAP("ssid-here", "pass-here");
  AF1::registerWSServer("192.168.1.123", "/", 3000);
  AF1::registerStateEnt(STATE_DEMO, new Demo(), "STATE_DEMO");
  AF1::registerStringHandler("4", [](){
    AF1::setRequestedState(STATE_DEMO);
  });
}

void loop()
{
  AF1::loop(); // REQUIRED
}

```

## API Docs

### Event Scheduling

```
class Demo2 : public Base
{
  int intervalMs = 3000;

public:
  Demo2()
  {
    intervalEvents.push_back(IntervalEvent(intervalMs, [](IECBArg a) {
      // Do something here every 3 seconds indefinitely
      return true;
    } /*, maxCbCnt*/));
  }
}
```
