
#include "demo2Master.h"
#include "state.h"
#include "led/led.h"
#include "stateEnt/demo2/demo2.h"

const float Demo2Master::coefs[] = {0, .00001, .0001, .0003, .0005, .001, .003, .005, .01, .03, .05, .1, .15, .3, .5, .7, .9, 1, 1, 1, .9, .7, .5, .3, .15, .1, .05, .03, .01, .005, .003, .001, .0005, 0};
const unsigned long Demo2Master::sceneMs = 7000;
const uint8_t Demo2Master::maxBrightness = 150;

Demo2Master::Demo2Master()
{
    intervalEventMap.insert(std::pair<String, IntervalEvent>("Demo2Master_1", IntervalEvent(MS_DEMO2_LOOP, [](IECBArg a)
                                         {
    uint8_t brightness = getCurCoef(a.getElapsedMs()) * maxBrightness;
    sendMsg(brightness);
    return true; }));
}

void Demo2Master::setup()
{
  sendMsg(0);
}

void Demo2Master::sendMsg(uint8_t b)
{
  JSMessage msg;
  msg.setState(STATE_DEMO1);
  msg.setType(TYPE_RUN_DATA);
  demo2_data d;
  d.brightness = b;
  msg.setData((uint8_t *)&d);
  pushOutbox(msg);
}

// Smooths out transitions from one coef to the next
float Demo2Master::getCurCoef(unsigned long elapsedMs)
{
  float curSceneMs = elapsedMs % sceneMs;
  float curSceneRatio = curSceneMs / sceneMs;
  float coefArrIdxExact = curSceneRatio * sizeof(coefs) / sizeof(coefs[0]);
  int coefArrIdxTrunc = coefArrIdxExact;
  float coefArrIdxRem = coefArrIdxExact - coefArrIdxTrunc;

  float coefA = coefs[coefArrIdxTrunc];
  float coefB = coefs[coefArrIdxTrunc + 1];
  float min = std::min(coefA, coefB);
  float max = std::max(coefA, coefB);
  float dif = max - min;

  float rem = coefArrIdxRem * dif;
  float result = min == coefA ? min + rem : max - rem;
  return result;
}
