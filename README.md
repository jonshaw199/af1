# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: Websocket Client
- :globe_with_meridians: HTTP Client
- :handshake: ESP-Now P2P (Auto-Connect; Self-Healing)
- :arrows_counterclockwise: AP/STA Mode (Use websocket, HTTP, and ESP-Now concurrently)
- :stopwatch: Time Sync Devices/Tasks (without IR)
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
    set(Event("Blink", [](ECBArg a) {
      setBuiltinLED(a.cbCnt % 2); // Blink once per sec
    }, EVENT_TYPE_PERM, 500));
  }

  void preStateChange(int nextState) {
    setBuiltinLED(0); // Make sure LED is off before leaving state
  }
};

void setup()
{
  Serial.begin(115200);
  AF1::begin(DEVICE_ID); // Required
  AF1::registerStateEnt(STATE_BLINK, new Blink());
  AF1::registerStateEnt(STATE_MACARENA, new Macarena());
  AF1::registerStringHandler("mac", [](SHArg a) { AF1::setRequestedState(STATE_MACARENA); });
  AF1::setInitialState(STATE_BLINK);
}

void loop()
{
  AF1::update(); // Required
}
```

Until the dust settles, other examples can be found in [AF1 Light Show](https://github.com/jonshaw199/af1-light-show/blob/main/src/main.cpp).

## API Docs

### Event Scheduling

...
