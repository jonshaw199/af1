#include "wifiHandler.h"
#include "pre.h"

WifiHandler::WifiHandler()
{
}

WifiHandler &WifiHandler::getInstance()
{
  static WifiHandler instance;
  return instance;
}

void WifiHandler::init()
{
  setAPMode();
  WiFi.softAPmacAddress(getInstance().macAP);
  Serial.print("MAC AP: ");
  printMac(getInstance().macAP);
  setSTAMode();
  WiFi.macAddress(getInstance().macSTA);
  Serial.print("MAC STA: ");
  printMac(getInstance().macSTA);
}

void WifiHandler::prepareWifi()
{
  WiFi.disconnect(true);
  delay(DELAY_PREPARE_WIFI);
}

void WifiHandler::setSTAMode()
{
  Serial.println("Setting wifi mode to STA");
  WiFi.mode(WIFI_STA);
}

void WifiHandler::setAPMode()
{
  Serial.println("Setting wifi mode to AP");
  WiFi.mode(WIFI_AP);
}

// Setup access point (aka open wifi network); this is used by slaves so master can find them
bool WifiHandler::broadcastAP()
{
  Serial.println("Broadcasting soft AP");
  String Prefix = STRINGIFY(DEVICE_PREFIX);
  String id = String(JS_ID);
  String SSID = Prefix + id;
  String Password = STRINGIFY(DEVICE_AP_PASS);
  return WiFi.softAP(SSID.c_str(), Password.c_str(), ESPNOW_CHANNEL, 0);
}

String WifiHandler::macToString(const uint8_t *m)
{
  char buffer[33];
  String result = "";
  for (int i = 0; i < 6; i++)
  {
    itoa(m[i], buffer, 16);
    result += buffer;
  }
  return result;
}

void WifiHandler::printMac(const uint8_t *m)
{
  Serial.print("{");
  for (int i = 0; i < 6; i++)
  {
    Serial.print("0x");
    Serial.print(m[i], HEX);
    if (i < 5)
      Serial.print(',');
  }
  Serial.println("}");
}

uint8_t *WifiHandler::getMacSTA()
{
  return getInstance().macSTA;
}

uint8_t *WifiHandler::getMacAP()
{
  return getInstance().macAP;
}
