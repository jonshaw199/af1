# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: [Websocket Client](#overview)
- :globe_with_meridians: HTTP Client
- :handshake: ESP-Now P2P
- :arrows_counterclockwise: AP/STA Mode (Use websocket, HTTP, and ESP-Now concurrently)
- :trident: [State Management](#overview)
- :stopwatch: [Event Scheduling](#event-scheduling)
- :radio: Over-The-Air (OTA) firmware updates
- :keyboard: [Serial Input Handling](#overview)

### Overview

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class.

When creating new state entities, setup, loop, serialization/deserialization, and other behavior from `Base` can be overridden as necessary.

This is the most trivial example:

```
#include <Arduino.h>
#include <AF1.h>

enum af1_state_extended
{
  STATE_SANDBOX1
};

class Sandbox1 : public Base
{
public:

  /*
    virtual void setup();
    virtual void loop();
    virtual bool validateStateChange(int s);
    virtual void preStateChange(int s);
    virtual void overrideInboxHandler();
    virtual void overrideOutboxHandler();
    virtual void serializeESPNow(AF1Msg &m);
    virtual void deserializeESPNow(AF1Msg &m);
    virtual bool scanForESPNowPeers();
    virtual String getName();
  */

  String getName()
  {
    return "SANDBOX1";
  }
};

void setup()
{
  AF1::setup(JS_ID);
  AF1::registerWifiAP("js-guest", "B1g5lams!lol");
  AF1::registerStateEnt(STATE_SANDBOX1, new Sandbox1());
}

void loop()
{
  AF1::loop();
}
```

Until the dust settles, other examples can be found in [AF1 Light Show](https://github.com/jonshaw199/af1-light-show/blob/main/src/main.cpp).

## API Docs

### Event Scheduling

```
class Demo2 : public Base
{
public:
  Demo2()
  {
    intervalEventMap.insert(std::pair<String, IntervalEvent>("Demo2_1", IntervalEvent(3000, [](IECBArg a) {
      // Do something here every 3 seconds indefinitely
      return true;
    } /*, maxCbCnt */ ))); // Or assign maxCbCnt for 1-time or x-time events
  }
}
```

...

```
...
```
