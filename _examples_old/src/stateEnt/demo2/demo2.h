#ifndef STATEENT_DEMO2_DEMO2_H_
#define STATEENT_DEMO2_DEMO2_H_

#include <FastLED.h>

typedef struct demo2_data
{
  CRGB color = CRGB::Red;
  uint8_t brightness = 0;
} demo2_data;

#endif // STATEENT_DEMO2_DEMO2_H_