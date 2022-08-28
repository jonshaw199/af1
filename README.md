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

Consumers of this framework can use and extend "state entities". A state entity defines the behavior for a particular state, and consumers can define their own states by extending the virtual `Base`, `WSEnt`, or `ESPNowEnt` classes (depending on the desired message handling mechanism, if any).

When creating new state entities, setup, loop, and message handling behavior from the virtual classes can be overridden as necessary. As an example, this Demo class extends the framework's `WSEnt` class so it can 2-way communicate with a server as a websocket client, but it also needs to perform some additional, custom logic during setup (printing to the serial monitor):

demo.h:

```
class Demo4 : public WSEnt
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
