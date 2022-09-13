# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: [Websocket Client](#overview)
- :handshake: ESP-Now P2P
- :trident: [State Management](#overview)
- :stopwatch: [Event Scheduling](#event-scheduling)
- :radio: Over-The-Air (OTA) firmware updates
- :keyboard: [Serial Input Handling](#overview)
- :shipit: Mesh :soon:

### Overview

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class.

When creating new state entities, setup, loop, serialization/deserialization, and other behavior from `Base` can be overridden as necessary. As a non-trivial example, this `Demo1` class overrides many of the inherited methods in order to use ESP-Now for sending/receiving messages and then controlling and LED according to the messages:

```
#include <AF1.h>

#include "demo1.h"
#include "state.h"
#include "led/led.h"

typedef struct demo1_data
{
  int r;
  int g;
  int b;
} demo1_data;

class Demo1 : public Base
{
public:
  Demo1();
  void setup();
  void preStateChange(int s);
  void overrideInboxHandler();
  void serializeESPNow(AF1Msg &m);
  void deserializeESPNow(AF1Msg &m);
};

Demo1::Demo1()
{
#if MASTER
  intervalEventMap.insert(std::pair<String, IntervalEvent>("Demo1", IntervalEvent(MS_DEMO1_LOOP, [](IECBArg a)
                                                                                  {
  AF1Msg msg;
  msg.setState(STATE_DEMO1);
  msg.setType(TYPE_RUN_DATA);
  msg.getJson()["r"] = rand() % 250;
  msg.getJson()["g"] = rand() % 250;
  msg.getJson()["b"] = rand() % 250;
  pushOutbox(msg);

  return true; })));
#endif
}

void Demo1::setup()
{
  Base::setup();
  JSLED::init();
}

void Demo1::preStateChange(int s)
{
  Base::preStateChange(s);
  Serial.println("Turning off lights on the way out");
  JSLED::fillColor(CRGB::Black);
}

void Demo1::overrideInboxHandler()
{
  setInboxMsgHandler([](AF1Msg m)
                     {
    Base::handleInboxMsg(m);
    switch (m.getType()) {
    case TYPE_RUN_DATA:
      CRGB c(m.getJson()["r"], m.getJson()["g"], m.getJson()["b"]);
      JSLED::fillColor(c);
      break;
    } });
}

void Demo1::serializeESPNow(AF1Msg &m)
{
  demo1_data d = {m.getJson()["r"], m.getJson()["g"], m.getJson()["b"]};
  m.setData((uint8_t *)&d);
}

void Demo1::deserializeESPNow(AF1Msg &m)
{
  demo1_data d;
  memcpy(&d, m.getData(), sizeof(d));
  m.getJson()["r"] = d.r;
  m.getJson()["g"] = d.g;
  m.getJson()["b"] = d.b;
}
```

Once a class has been created, the state entity can be registered in `main`. Other basic configuration is handled here too such as configuring wifi or setting various defaults like initial state:

```
#include <Arduino.h>
#include <AF1.h>

#include "state.h"
#include "stateEnt/demo1/demo1.h"

void setup()
{
  Serial.begin(JS_BAUD);
  AF1::setup(JS_ID);
  AF1::registerWifiAP("js-guest", "B1g5lams!lol", JS_IP_A, JS_IP_B, JS_IP_C, JS_IP_D, 192, 168, 1, 254, 255, 255, 255, 0);
  AF1::registerStateEnt(STATE_DEMO1, new Demo1());
  AF1::registerStringHandler("1", []()
                             { AF1::setRequestedState(STATE_DEMO1); });
  AF1::setDefaultWSClientInfo({STRINGIFY(WS_HOST), STRINGIFY(WS_PATH), WS_PORT, STRINGIFY(WS_PROTOCOL)});
}

void loop()
{
  AF1::loop();
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

...

```
...
```

## Examples

Until the dust settles, examples can be found in other repos such as [AF1 Light Show](https://github.com/jonshaw199/af1-light-show).
