#ifndef STATEENT_DEMO3_DEMO3_H_
#define STATEENT_DEMO3_DEMO3_H_

#include <FastLED.h>

typedef struct demo3_data
{
  CRGB color = CRGB::Red;
  uint8_t brightness = 0;
} demo3_data;

#endif // STATEENT_DEMO3_DEMO3_H_