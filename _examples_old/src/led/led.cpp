
#include "led.h"

JSLED::JSLED()
{
}

JSLED &JSLED::getInstance()
{
  static JSLED instance; // Guaranteed to be destroyed.
                         // Instantiated on first use.
  return instance;
}

void JSLED::init()
{
  if (CNT_A)
  {
    FastLED.addLeds<WS2812, LED_PIN_A, GRB>(getInstance().ledsA, CNT_A);
  }
  if (CNT_B)
  {
    FastLED.addLeds<WS2812, LED_PIN_B, GRB>(getInstance().ledsB, CNT_B);
  }
  FastLED.setBrightness(DEMO1_BRIGHTNESS);
}

void JSLED::fillColor(CRGB color)
{
  if (CNT_A)
  {
    fill_solid(getInstance().ledsA, CNT_A, color);
  }
  if (CNT_B)
  {
    fill_solid(getInstance().ledsB, CNT_B, color);
  }
  FastLED.show();
}

CRGB JSLED::getRandColor()
{
  CRGB options[] = {CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::White};
  int i = rand() % 4;
  return options[i];
}

const uint8_t JSLED::getBrightness()
{
  return getInstance().brightness;
}

void JSLED::setBrightness(uint8_t b)
{
  getInstance().brightness = b;
  FastLED.setBrightness(b);
}
