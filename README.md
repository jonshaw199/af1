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

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class.

When creating new state entities, setup, loop, serialization/deserialization, and other behavior from `Base` can be overridden as necessary.

This is a trivial example:

```
#include <Arduino.h>
#include <AF1.h>

#define DEVICE_ID 1

enum custom_states
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
    virtual msg_handler getInboxHandler();
    virtual msg_handler getOutboxHandler();
    virtual void serializeESPNow(AF1Msg &m);
    virtual void deserializeESPNow(AF1Msg &m);
    virtual bool doScanForPeersESPNow();
    virtual bool doConnectToWSServer();
    virtual bool doSync();
    virtual void doSynced();
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
    static void registerWifiAP(String s, String p, int ipA, int ipB, int ipC, int ipD, int gatewayIpA, int gatewayIpB, int gatewayIpC, int gatewayIpD, int subnetIpA, int subnetIpB, int subnetIpC, int subnetIpD);
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
    intervalEventMap["Demo2_1"] = IntervalEvent("Demo2_1", 3000, [](IECBArg a) {
      // Do something here every 3 seconds indefinitely
      return true;
    } /*, maxCbCnt */ ))); // Or assign maxCbCnt for 1-time or x-time events
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
    timeEventMap["Demo3_1"] = TimeEvent("Demo3_1", 1830354651000, [](TECBArg a) {
      // Do something here every 3 seconds starting on Saturday, January 1, 2028 7:50:51 AM GMT-08:00
      return true;
    }, /*intervalMs*/3000 /*, maxCbCnt */)));
  }
}
```
