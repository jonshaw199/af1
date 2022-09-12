# AF1 - Arduino/ESP32 Framework

## Getting Started

### Features

- :trident: [State Management](#overview)
- :electric_plug: [Websocket Client](#overview)
- :handshake: ESP-Now P2P Communication
- :stopwatch: [Event Scheduling](#event-scheduling)
- :radio: Over-The-Air (OTA) firmware updates
- :keyboard: [Serial Input Handling](#overview)
- :leaves: Lightweight
- :heavy_plus_sign: Extendable
- :racing_car: Performant

### Overview

The AF1 core consists of a handful of pre-built "state entities" and one "state manager". A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class.

When creating new state entities, setup, loop, and message handling behavior from `Base` can be overridden as necessary. For example, this `Demo` class overrides `setup()` and `overrideInboxHandler()` in order to handle LED-related tasks:

**demo.cpp:**

```
#include <AF1.h>
#include "led/led.h"

class Demo : public Base
{
public:
  void setup()
  {
    Base::setup();
    JSLED::init();
  }

  void overrideInboxHandler()
  {
    setInboxMsgHandler([](AF1Msg m){
      Base::handleInboxMsg(m);
      switch (m.getType())
      {
      case TYPE_RUN_DATA:
        uint8_t b = m.getJson()["brightness"];
        JSLED::setBrightness(b);
      }
    });
  }
}
```

**main.cpp**

```
#include <Arduino.h>
#include <AF1.h>
#include "demo.cpp"

#define DEVICE_ID 1

enum user_defined_states
{
  STATE_DEMO
};

void setup()
{
  Serial.begin(115200);
  AF1::setup(DEVICE_ID); // REQUIRED
  AF1::registerWifiAP("ssid-here", "pass-here");
  Demo *d = new Demo();
  d->setWSClientInfo({"192.168.1.65", "/rc/demo5/ws", 3000, ""});
  AF1::registerStateEnt(STATE_DEMO, d);
  // Change to STATE_DEMO when "4" is entered into serial monitor
  AF1::registerStringHandler("4", [](){
    AF1::setRequestedState(STATE_DEMO);
  });
}

void loop()
{
  AF1::loop(); // REQUIRED
}

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

**API Docs To Do**

```
void AF1::registerStateEnt(int i, Base *s, String n);
void AF1::registerStringHandler(String s, string_input_handler h);
void AF1::registerWifiAP(String s, String p);
void AF1::registerWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd);

StateManager AF1::&getInstance();
void AF1::setup(int id);
void AF1::loop();
int AF1::getCurState();
int AF1::getPrevState();
void AF1::setRequestedState(int s);
int AF1::getRequestedState();
void AF1::handleUserInput(String s);
String AF1::stateToString(int s);
bool AF1::handleStateChange(int s);
const std::vector<wifi_ap_info> AF1::getWifiAPs();
void AF1::setInitialState(int s);
int AF1::getInitialState();
void AF1::setStateAfterHandshake(int s);
int AF1::getStateAfterHandshake();
void AF1::setPurgNext(int p, int n);
const std::map<int, Base *> AF1::&getStateEntMap();
int AF1::getDeviceID();
void AF1::setDefaultWSClientInfo(String host, String path, int port, String protocol);

// Inherited members from Base
```

## To Do

- Add state machine support

## To Explore

- Mesh network
- Nested state entities
- Long/infinite loops in stateEnt::loop() (basically hijacking the main loop and preventing requested state changes from being fulfilled the normal way; may be useful for other things)
