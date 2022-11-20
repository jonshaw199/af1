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

A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class. When creating new state entities, setup, loop, message handling, and other behavior from `Base` can be overridden as necessary. This is a trivial example:

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

Until the dust settles, other examples can be found in the [AF1-1](https://github.com/jonshaw199/af1-1/blob/main/firmware/src/main.cpp) project.

### Messaging

A message can be sent to peer devices (auto-connect by default) and/or a websocket server with only a few lines of code:

```
AF1Msg msg();
msg.json()["hello"] = "world"; // All the power of ArduinoJSON at your fingertips
pushOutbox(msg);
```

Messages can be received by overriding `getInboxHandler()` (example from the [AF1-1](https://github.com/jonshaw199/af1-1/blob/main/firmware/src/main.cpp) project mentioned above):

```
#define TYPE_MOTION 0 // Custom types can be created; really just unique, arbitrary numbers

msg_handler LightShowBase::getInboxHandler()
{
  return [](AF1Msg m)
  {
    Base::handleInboxMsg(m); // Or whatever superclass is being used

    switch (m.getType())
    {
    case TYPE_MOTION:
      Serial.print("Notification msg received: motion ");
      if (m.json()["motion"] == true)
      {
        Serial.println(" begin...");
#ifdef VS1053_CS_PIN
        playFile("/track001.mp3");
#endif
      }
      else
      {
        Serial.println(" end.");
      }
      break;
    }
  };
}
```

### Event Scheduling

One-time events can be scheduled as well as recurring ones. Events can be state-specific or global (always active). Start/end times can be set; NTP is an option.

...

## API Docs

...
