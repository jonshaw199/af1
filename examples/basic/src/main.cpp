
#include <AF1.h>

enum CustomStates
{
  STATE_BLINK,
  STATE_SOLID
};

class Blink : public Base
{
public:
  Blink()
  {
    addEvent(Event(
        "Blink-1", [](ECBArg a)
        {
          Serial.print(a.cbCnt);
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
    setBuiltinLED(1);
  }
};

void setup()
{
  Serial.setTimeout(1500);
  Serial.begin(115200);
  AF1::begin(JS_ID); // Device-specific; using build flags from platformio.ini
  AF1::addWifiAP(JSSSID, JSPASS, JS_IP_A, JS_IP_B, JS_IP_C, JS_IP_D, 192, 168, 1, 254, 255, 255, 255, 0);
  AF1::addStateEnt(STATE_BLINK, new Blink());
  AF1::addStateEnt(STATE_SOLID, new Solid());
  AF1::addStringHandler("blink", [](SHArg a)
                        { AF1::setRequestedState(STATE_BLINK); });
  AF1::addStringHandler("solid", [](SHArg a)
                        { AF1::setRequestedState(STATE_SOLID); });
  AF1::setInitialState(STATE_BLINK);
  // AF1::setDefaultWS(SERVER_IP, String("/?deviceId=") + String(JS_ID), SERVER_PORT);
}

void loop()
{
  AF1::update();
}
