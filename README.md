# AF1 - Arduino/ESP32 Framework

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

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a particular state, and new state entities can be created by extending the virtual `Base`, `WSEnt`, or `ESPNowEnt` classes, depending on the desired message handling mechanism (if any):

- `Base`: HTTP client or no wifi connectivity
  - `WSEnt`: Websocket client
  - `ESPNowEnt`: ESP-Now master/slave

When creating new state entities, setup, loop, and message handling behavior from the virtual classes can be overridden as necessary. As an example, this `Demo` class extends the framework's `WSEnt` class in order to receive websocket messages from a server for controlling an LED. Note that `setup()` and `overrideInboxHandler()` are inherited from `WSEnt`, but their behavior is overridden here in order to handle LED-related tasks:

**demo.cpp:**

```
#include <AF1.h>
#include "led/led.h"

class Demo : public WSEnt
{
public:
  void setup()
  {
    WSEnt::setup();
    JSLED::init();
  }

  void overrideInboxHandler()
  {
    setInboxMsgHandler([](JSMessage m){
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
}
```

**main.cpp**

```
#include <Arduino.h>
#include <AF1.h>
#include "demo.cpp"

#define DEVICE_ID 1

enum user_defined_states
{
  STATE_DEMO
};

void setup()
{
  Serial.begin(115200);
  AF1::setup(DEVICE_ID); // REQUIRED
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
