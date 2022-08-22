#ifndef WIFIHANDLER_WIFIHANDLER_H_
#define WIFIHANDLER_WIFIHANDLER_H_

#include <Arduino.h>
#include <WiFi.h>

class WifiHandler
{
  WifiHandler(); // Private constructor
  uint8_t macAP[6];
  uint8_t macSTA[6];

public:
  static WifiHandler &getInstance(); // Public singleton instance getter
  static void init();
  static void prepareWifi();
  static void setSTAMode();
  static void setAPMode();
  static bool broadcastAP();
  static String macToString(const uint8_t *m);
  static void printMac(const uint8_t *m);
  static uint8_t *getMacSTA();
  static uint8_t *getMacAP();
};

#endif // WIFIHANDLER_WIFIHANDLER_H_
