
#include "led.h"

LED::LED()
{
}

LED &LED::getInstance()
{
  static LED instance; // Guaranteed to be destroyed.
                       // Instantiated on first use.
  return instance;
}

void LED::init()
{
  if (CNT_A)
  {
    FastLED.addLeds<WS2812, LED_PIN_A, GRB>(getInstance().ledsA, CNT_A);
  }
  if (CNT_B)
  {
    FastLED.addLeds<WS2812, LED_PIN_B, GRB>(getInstance().ledsB, CNT_B);
  }
  FastLED.setBrightness(LED_BRIGHTNESS_LOW);
}

void LED::fillColor(CRGB color)
{
  Serial.println("Filling color");
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

CRGB LED::getRandColor()
{
  CRGB options[] = {CRGB::Red, CRGB::Blue, CRGB::Green, CRGB::White};
  int i = rand() % 4;
  return options[i];
}