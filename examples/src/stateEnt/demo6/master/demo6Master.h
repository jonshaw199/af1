#ifndef STATEENT_DEMO6_MASTER_DEMO6MASTER_H_
#define STATEENT_DEMO6_MASTER_DEMO6MASTER_H_

#include <AF1.h>

class Demo6Master : public Base
{
  static void sendMsg(uint8_t b);
  static const float coefs[];
  static const unsigned long sceneMs;
  static float getCurCoef(unsigned long elapsedMs);
  static const uint8_t maxBrightness;

public:
  Demo6Master();
  void setup();
};

#endif // STATEENT_DEMO6_MASTER_DEMO6MASTER_H_