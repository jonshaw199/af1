# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: Websocket Client
- :globe_with_meridians: HTTP Client
- :handshake: ESP-Now P2P (Auto-Connect; Self-Healing)
- :arrows_counterclockwise: AP/STA Mode (Use websocket, HTTP, and ESP-Now concurrently)
- :stopwatch: Time Sync Devices/Tasks (no IR or line of sight required)
- :calendar: Event Scheduling
- :trident: State Management
- :radio: Over the air (ArduinoOTA) firmware updates

### Overview

A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class. When creating new state entities, setup, loop, serialization/deserialization, and other behavior from `Base` can be overridden as necessary. This is a trivial example:

```
#include <AF1.h>
#include "stateEnt/macarena.h" // ...
#define DEVICE_ID 1

enum custom_states
{
  STATE_BLINK,
  STATE_MACARENA
};

class Blink : public Base
{
public:
  Blink() {
    setIE(IntervalEvent("Blink", 500, [](IECBArg a) {
      setBuiltinLED(a.getCbCnt() % 2); // Blink once per sec
    }));
  }
};

void setup()
{
  Serial.begin(115200);
  AF1::begin(DEVICE_ID); // Required
  registerStateEnt(STATE_BLINK, new Blink());
  registerStateEnt(STATE_MACARENA, new Macarena());
  setInitialState(STATE_BLINK);
}

void loop()
{
  AF1::update(); // Required
}
```

Until the dust settles, other examples can be found in [AF1 Light Show](https://github.com/jonshaw199/af1-light-show/blob/main/src/main.cpp).

## API Docs

### Event Scheduling

**Interval Events**

```
class Demo2 : public Base
{
public:
  Demo2()
  {
    AF1::setIE(IntervalEvent("Demo2_1", 3000, [](IECBArg a) {
      // Do something here every 3 seconds indefinitely
    } /*, maxCbCnt */ )))); // Or assign maxCbCnt for 1-time or x-time events
  }
}
```

**Time Events (WiFi/NTP)**

```
class Demo3 : public Base
{
public:
  Demo3()
  {
    AF1::setTE(TimeEvent("Demo3_1", 1830354651000, [](TECBArg a) {
      // Do something here every 3 seconds starting on Saturday, January 1, 2028 7:50:51 AM GMT-08:00
    }, /*intervalMs*/3000 /*, maxCbCnt */))));
  }
}
```
