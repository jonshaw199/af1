#ifndef STATEENT_DEMO2_MASTER_DEMO2MASTER_H_
#define STATEENT_DEMO2_MASTER_DEMO2MASTER_H_

#include <AF1.h>

class Demo2Master : public Base
{
  static void sendMsg(uint8_t b);
  static const float coefs[];
  static const unsigned long sceneMs;
  static float getCurCoef(unsigned long elapsedMs);
  static const uint8_t maxBrightness;

public:
  Demo2Master();
  void setup();
};

#endif // STATEENT_DEMO2_MASTER_DEMO2MASTER_H_