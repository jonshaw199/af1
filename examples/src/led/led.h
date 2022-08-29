
#ifndef LED_LED_H_
#define LED_LED_H_

#ifndef CNT_A
#ifdef LED_PIN_A
#define CNT_A 3 // Test strips
#else
#define CNT_A 0
#define LED_PIN_A 0
#endif
#endif

#ifndef CNT_B
#ifdef LED_PIN_B
#define CNT_B 3
#else
#define CNT_B 0
#define LED_PIN_B 0
#endif
#endif

#include <FastLED.h>

#include <AF1.h>

class JSLED
{
  JSLED();
  CRGB ledsA[CNT_A];
  CRGB ledsB[CNT_B];
  uint8_t brightness = 0;
  const uint8_t MAX_BRIGHTNESS = 255;

public:
  // LED(); // Now singleton
  static JSLED &getInstance();
  static void init();
  static void fillColor(CRGB color);
  static CRGB getRandColor();
  static const uint8_t getBrightness();
  static void setBrightness(uint8_t b);
};

#endif // LED_LED_H_