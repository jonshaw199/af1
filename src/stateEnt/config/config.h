#ifndef STATEENT_CONFIG_CONFIG_H_
#define STATEENT_CONFIG_CONFIG_H_

#include <AF1.h>

class Config : public Base
{
  DynamicJsonDocument loadConfig();
  void printConfig(DynamicJsonDocument c);
public:
  void setup();
};

#endif