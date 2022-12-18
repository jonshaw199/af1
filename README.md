# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: Websocket Client (w/ MQTT Support)
- :handshake: ESP-Now P2P (2-Way; Auto-Connect)
- :globe_with_meridians: HTTP Client
- :arrows_counterclockwise: AP/STA Mode (Use websocket, HTTP, and ESP-Now Concurrently)
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
#define DEVICE_ID "Some unique ID"

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
    virtual void preStateChange(int s);
    virtual msg_handler getInboxHandler();
    virtual msg_handler getOutboxHandler();
    virtual String getName();
    virtual bool doScanForPeersESPNow();
    virtual bool doConnectToWSServer();
    virtual void doSynced();
    virtual bool doSync();
    virtual void onConnectWSServer();
    virtual void onConnectWSServerFailed();
    virtual void onConnectWifi();
    virtual void onConnectWifiFailed();
    virtual void onConnectEspNowPeer(String peerId);
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

  // AF1::addWifiAP(String ssid, String password)
  // Static IP version also provided; multiple APs can be added
  // AF1::setDefaultWS(String host, String path, int port, String protocol}
  // State-specific websocket connections also supported using Base::setWS() instance method
  // MQTT support is built in as an option (see below for example)
}

void loop()
{
  AF1::update(); // Required
}
```

### Messaging

A message can be sent to peer ESP32s (auto-connect by default) and/or a websocket server with only a few lines of code:

```
AF1Msg msg();
msg.json()["hello"] = "world"; // All the power of ArduinoJSON at your fingertips
pushOutbox(msg);
```

Messages can be received by overriding `getInboxHandler()`:

```
msg_handler SomeSubclass::getInboxHandler()
{
  return [](AF1Msg &m)
  {
    Base::handleInboxMsg(m); // Or whatever superclass is being used
    String world = m.json()["hello"];
    Serial.print("Hello " + world);
  };
}
```

MQTT support is built in and can be used if desired. This is how an ESP32 can subscribe to a topic, perhaps in `onConnectWS()` (overridden from `Base`):

```
AF1Msg m(TYPE_MQTT_SUBSCRIBE);
m.json()["topic"] = "some/topic";
m.json()["qos"] = 0;
pushOutbox(m);
```

### Event Scheduling

One-time events can be scheduled as well as recurring ones. Events can be state-specific or global (always active). Start/end times can be set; NTP is an option.

...

## API Docs

...
