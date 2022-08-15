
#ifndef LED_LED_H_
#define LED_LED_H_

#ifndef CNT_A
#define CNT_A 3 // Test strips
#endif
#ifndef CNT_B
#define CNT_B 3
#endif

#include <FastLED.h>

#include "state/state.h"

class LED
{
  LED();
  CRGB ledsA[CNT_A];
  CRGB ledsB[CNT_B];

public:
  // LED(); // Now singleton
  static LED &getInstance();
  static void init();
  static void fillColor(CRGB color);
  static CRGB getRandColor();
};

#endif // LED_LED_H_