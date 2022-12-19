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

A state entity defines the behavior for a state, and new state entities can be created by extending the `Base` class. When creating new state entities, setup, loop, message handling, and other behavior from `Base` can be overridden as necessary. Every time the state changes, the corresponding state entity's `setup()` method is called once. Then that state entity's `loop()` method is called repeatedly, consistent with the Arduino framework.

A boiled down example can be seen below. More practical examples can be found in the [AF1-1](https://github.com/jonshaw199/af1-1/blob/main/firmware/lights/src/main.cpp) project.

```
#include <AF1.h>

enum CustomStates
{
  STATE_BLINK,
  STATE_SOLID
};

class Blink : public Base
{
public:
  void setup()
  {
    Base::setup();
    addEvent(Event(
        "Blink-1", [](ECBArg a)
        {
          setBuiltinLED(a.cbCnt % 2); // Blink once per sec
        },
        EVENT_TYPE_PERM, 500));
  }
};

class Solid : public Base
{
public:
  void setup()
  {
    Base::setup();
    setBuiltinLED(1);
  }
};

void setup()
{
  Serial.setTimeout(1500);
  Serial.begin(115200);
  AF1::begin("unique-device-id-1"); // Device-specific ID; build flags might be useful for this (see basic example in examples directory)
  AF1::addWifiAP("ssid", "pass", 192, 168, 1, 123, 192, 168, 1, 254, 255, 255, 255, 0); // Static IP used here which is nice for OTA but not required; build flags might be useful for this too
  AF1::addStateEnt(STATE_BLINK, new Blink());
  AF1::addStateEnt(STATE_SOLID, new Solid());
  AF1::addStringHandler("blink", [](SHArg a)
                        { AF1::setRequestedState(STATE_BLINK); }); // Simply type "blink" in serial monitor for state change; convenient way of changing states in dev env
  AF1::addStringHandler("solid", [](SHArg a)
                        { AF1::setRequestedState(STATE_SOLID); });
  AF1::setInitialState(STATE_BLINK);
  // AF1::setDefaultWS(SERVER_IP, String("/?deviceId=") + String(DEVICE_ID), SERVER_PORT);
}

void loop()
{
  AF1::update();
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

To Do...

## API Docs

To Do...

```
static void begin(String id);
static void update();
static String macToString(const uint8_t *m);
static void printMac(const uint8_t *m);
static void pushOutbox(AF1Msg m);
static void pushInbox(AF1Msg m);
static AF1JsonDoc httpGet(String url);
static AF1JsonDoc httpPost(String url, AF1JsonDoc body);
static void setBuiltinLED(bool on);
static int getCurState();
static int getPrevState();
static void setRequestedState(int s);
static int getRequestedState();
static String stateToString(int s);
static void setInitialState(int s);
static int getInitialState();
static void setPurgNext(int p, int n);
static Base *getCurStateEnt();
static String getDeviceID();
static void setIntervalTime(String e, unsigned long t);
static void detach(bool detach);
static void setDefaultWS(String host, String path, int port, String protocol = "");
static void addEvent(Event e);
static void removeEvent(String eventName);
static void addStateEnt(int i, Base *s);
static void removeStateEnt(int i);
static void addStringHandler(String s, string_input_handler h);
static void removeStringHandler(String s);
static void addWifiAP(String s, String p);
static void addWifiAP(String s, String p, int a, int b, int c, int d, int ga, int gb, int gc, int gd, int sa, int sb, int sc, int sd);

static NTPClient timeClient;

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
virtual void onDisconnectWSServer();
virtual void onConnectWifi();
virtual void onConnectWifiFailed();
virtual void onConnectEspNowPeer(String peerId);

unsigned long getStartMs();
unsigned long getElapsedMs();
void setWS(String host, String path, int port, String protocol = "");
```

### Message Types

Custom message types can be used and can start with 0. The base AF1 message types start at 100 and are as follows:

```
enum MessageType
{
  TYPE_NONE = 100,
  TYPE_HANDSHAKE_REQUEST,
  TYPE_HANDSHAKE_RESPONSE,
  TYPE_CHANGE_STATE,
  TYPE_TIME_SYNC,
  TYPE_TIME_SYNC_RESPONSE,
  TYPE_TIME_SYNC_START,
  // MQTT
  TYPE_MQTT_SUBSCRIBE,
  TYPE_MQTT_SUBACK,
  TYPE_MQTT_UNSUBSCRIBE,
  TYPE_MQTT_UNSUBACK,
  TYPE_MQTT_PUBLISH,
  // QOS 1
  TYPE_MQTT_PUBACK,
  // QOS 2
  TYPE_MQTT_PUBREC,
  TYPE_MQTT_PUBREL,
  TYPE_MQTT_PUBCOMP,
};
```

### States

Custom states can and should be used, and can start with 0. The base AF1 states start at 100 and are as follows:

```
enum State
{
  STATE_INIT = 100,
  STATE_PURG,
  STATE_OTA,
  STATE_RESTART,
  STATE_IDLE_BASE,
  STATE_SYNC_TEST,
};
```
