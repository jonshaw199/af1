# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :trident: [State Management](#overview)
- :electric_plug: [Websocket Client](#overview)
- :handshake: ESP-Now P2P Communication
- :stopwatch: [Event Scheduling](#event-scheduling)
- :radio: Over-The-Air (OTA) firmware updates
- :keyboard: [Serial Input Handling](#overview)
- :shipit: Mesh Mode :soon:

### Overview

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class.

When creating new state entities, setup, loop, and message handling behavior from `Base` can be overridden as necessary.

...

```
...
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

## To Explore

- Nested state entities
- Long/infinite loops in stateEnt::loop() (basically hijacking the main loop and preventing requested state changes from being fulfilled the normal way; may be useful for other things)
