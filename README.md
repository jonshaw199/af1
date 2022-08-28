### Features

- :trident: State management
- :electric_plug: Websocket client
- :handshake: Peer to peer communication (ESPNOW)
- :stopwatch: Timed event handling (one-time or recurring)
- :keyboard: Serial input handling
- :radio: Over-The-Air (OTA) firmware updates
- :leaves: Lightweight
- :heavy_plus_sign: Extendable

### Overview

Consumers of this framework use and extend "state entities". A state entity defines the behavior for a particular state, and new states can be defined by extending the virtual `Base`, `WSEnt`, or `ESPNowEnt` classes (depending on the desired message handling mechanism, if any).

When creating new state entities, setup, loop, and message handling behavior from the virtual classes can be overridden as necessary. As an example, this `Demo` class extends the framework's `WSEnt` class to use a websocket, but it also needs to perform some additional, custom logic during setup (printing to the serial monitor):

demo.h:

```
class Demo : public WSEnt
{
  void setup();
};
```

demo.cpp:

```
void Demo::setup()
{
  WSEnt::setup();
  Serial.println("Demo-specific logic here");
}
```
