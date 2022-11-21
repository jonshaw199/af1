
#include <Preferences.h>

#include "config.h"

static Preferences preferences;

void Config::setup()
{
  // If config requred then do it now
  // Otherwise move on to init state
  if (preferences.begin(PREFS_NAMESPACE))
  {
    Serial.println("Loading config");
    DynamicJsonDocument c = loadConfig();
    String id = c[PREFS_ID_KEY];
    if (id.length())
    {
      // Config not necessary but should still be optional somehow
      printConfig(c);
      set(Event(
          "Config_AutoProceed", [](ECBArg a)
          { setRequestedState(STATE_INIT); },
          EVENT_TYPE_TEMP, 3000));
    }
    else
    {
      // Commence initial config
      Serial.println("Initial config needed");
    }
  }
  else
  {
    Serial.println("Failed to open prefs");
    while (1)
      delay(1000); // Halt
  }
}

DynamicJsonDocument Config::loadConfig()
{
  DynamicJsonDocument c(1024);
  c[PREFS_ID_KEY] = preferences.getString(PREFS_ID_KEY);
  c[PREFS_WIFI_SSID_1_KEY] = preferences.getString(PREFS_WIFI_SSID_1_KEY);
  c[PREFS_WIFI_PASS_1_KEY] = preferences.getString(PREFS_WIFI_PASS_1_KEY);
  c[PREFS_WIFI_SSID_2_KEY] = preferences.getString(PREFS_WIFI_SSID_2_KEY);
  c[PREFS_WIFI_PASS_2_KEY] = preferences.getString(PREFS_WIFI_PASS_2_KEY);
  c[PREFS_WIFI_SSID_3_KEY] = preferences.getString(PREFS_WIFI_SSID_3_KEY);
  c[PREFS_WIFI_PASS_3_KEY] = preferences.getString(PREFS_WIFI_PASS_3_KEY);
  c[PREFS_IP_KEY] = preferences.getString(PREFS_IP_KEY);
}

void Config::printConfig(DynamicJsonDocument c)
{
  String pretty;
  serializeJsonPretty(c, pretty);
  Serial.print("Config: ");
  Serial.println(pretty);
  Serial.println();
}
