# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :electric_plug: [Websocket Client](#overview)
- :globe_with_meridians: HTTP Client
- :handshake: ESP-Now P2P
- :arrows_counterclockwise: AP/STA Mode (Use websocket, HTTP, and ESP-Now concurrently)
- :trident: [State Management](#overview)
- :stopwatch: [Event Scheduling](#event-scheduling)
- :radio: Over-The-Air (OTA) firmware updates
- :keyboard: [Serial Input Handling](#overview)

### Overview

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class.

When creating new state entities, setup, loop, serialization/deserialization, and other behavior from `Base` can be overridden as necessary.

This is the most trivial example:

```
#include <Arduino.h>
#include <AF1.h>

#define DEVICE_ID 1

enum af1_state_extended
{
  STATE_SANDBOX1
};

class Sandbox1 : public Base
{
public:
  /*
    virtual void setup();
    virtual void loop();
    virtual bool validateStateChange(int s);
    virtual void preStateChange(int s);
    virtual void overrideInboxHandler();
    virtual void overrideOutboxHandler();
    virtual void serializeESPNow(AF1Msg &m);
    virtual void deserializeESPNow(AF1Msg &m);
    virtual bool scanForESPNowPeers();
    virtual String getName();
  */

  String getName()
  {
    return "SANDBOX1";
  }
};

void setup()
{
  Serial.begin(115200);
  AF1::setup(DEVICE_ID); // Required

  /*
    static void registerStateEnt(int i, Base *s);
    static void registerStringHandler(String s, string_input_handler h);
    static void registerWifiAP(String s, String p);
    static void registerWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd);
    static void setInitialState(int s);
    static void setDefaultWSClientInfo(ws_client_info w);
  */

  AF1::registerStateEnt(STATE_SANDBOX1, new Sandbox1());
}

void loop()
{
  AF1::loop(); // Required
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
    intervalEventMap.insert(std::pair<String, IntervalEvent>("Demo2_1", IntervalEvent(3000, [](IECBArg a) {
      // Do something here every 3 seconds indefinitely
      return true;
    } /*, maxCbCnt */ ))); // Or assign maxCbCnt for 1-time or x-time events
  }
}
```

**Time Events (WiFi/NTP)**

```
class Demo2 : public Base
{
public:
  Demo2()
  {
    timeEventMap.insert(std::pair<String, TimeEvent>("Sandbox1_1", TimeEvent(1830354651000, [](TECBArg a) {
      // Do something here every 3 seconds starting on Saturday, January 1, 2028 7:50:51 AM GMT-08:00
      return true;
    }, /*intervalMs*/3000 /*, maxCbCnt */)));
  }
}

```
