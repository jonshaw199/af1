# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: Websocket Client
- :handshake: ESP-Now P2P (2-Way; Auto-Connect)
- :globe_with_meridians: HTTP Client
- :arrows_counterclockwise: AP/STA Mode (Use websocket, HTTP, and ESP-Now concurrently)
- :stopwatch: Time Sync Devices/Tasks
- :calendar: Event Scheduling
- :trident: State Management
- :radio: Over the air (ArduinoOTA) firmware updates

### Overview

A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class. When creating new state entities, setup, loop, message handling, and other behavior from `Base` can be overridden as necessary.

Until the dust settles, practical and up-to-date examples can be found in the [AF1-1](https://github.com/jonshaw199/af1-1/blob/main/firmware/lights/src/main.cpp) project. A more trivial example can be seen below:

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
    addEvent(Event("Blink", [](ECBArg a) {
      setBuiltinLED(a.cbCnt % 2); // Blink once per sec
    }, EVENT_TYPE_PERM, 500));
  }

  /*
    // Can be overridden
    virtual void setup();
    virtual void loop();
    virtual bool validateStateChange(int s);
    virtual void preStateChange(int s);
    virtual msg_handler getInboxHandler();
    virtual msg_handler getOutboxHandler();
    virtual String getName();
    virtual bool doScanForPeersESPNow();
    virtual bool doConnectToWSServer();
    virtual void doSynced();
    virtual bool doSync();
    virtual void onConnectWSServer();
  */

  void preStateChange(int nextState) {
    Base::preStateChange(nextState); // Usually need to call super methods; easy to forget
    setBuiltinLED(0); // Make sure LED is off before leaving state
  }
};

void setup()
{
  Serial.begin(115200);
  AF1::begin(DEVICE_ID); // Required
  AF1::addStateEnt(STATE_BLINK, new Blink());
  AF1::addStateEnt(STATE_MACARENA, new Macarena());
  AF1::addStringHandler("mac", [](SHArg a) { AF1::setRequestedState(STATE_MACARENA); });
  AF1::setInitialState(STATE_BLINK);
  // AF1::setDefaultWS(String host, String path, int port, String protocol}
  // State-specific websocket connections can also be used
}

void loop()
{
  AF1::update(); // Required
}
```

### Messaging

A message can be sent to peer devices (auto-connect by default) and/or a websocket server with only a few lines of code:

```
AF1Msg msg();
msg.json()["hello"] = "world"; // All the power of ArduinoJSON at your fingertips
pushOutbox(msg);
```

Messages can be received by overriding `getInboxHandler()`:

```

msg_handler SomeSubclass::getInboxHandler()
{
  return [](AF1Msg m)
  {
    Base::handleInboxMsg(m); // Or whatever superclass is being used
    String world = m.json()["hello"];
    Serial.print("Hello " + world);
  };
}
```

### Event Scheduling

One-time events can be scheduled as well as recurring ones. Events can be state-specific or global (always active). Start/end times can be set; NTP is an option.

...

## API Docs

...
