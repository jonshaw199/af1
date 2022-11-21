
#include <Preferences.h>

#include "config.h"

static Preferences preferences;

bool overrideAutoProceed;

void Config::setup()
{
  // No base setup here

  // If config requred then do it now
  // Otherwise move on to init state
  if (preferences.begin(PREFS_NAMESPACE))
  {
    Serial.println("Loading config");
    DynamicJsonDocument c = loadConfig();
    if (hasRequiredConfigs(c))
    {
      // Config not necessary but should still be optional somehow
      printConfig(c);
      set(Event(
          EVENTKEY_CONFIG_AUTOPROCEED, [](ECBArg a)
          { setRequestedState(STATE_INIT); },
          EVENT_TYPE_TEMP, 3000));
      addStringHandler(SHKEY_CONFIG_START, [](SHArg a)
                            { unset(EVENTKEY_CONFIG_AUTOPROCEED); });
    }
    else
    {
      // Commence config
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

void Config::loop()
{
  // No base loop here
}

void Config::preStateChange(int s)
{
  removeStringHandler(SHKEY_CONFIG_START);
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
  return c;
}

void Config::printConfig(DynamicJsonDocument c)
{
  String pretty;
  serializeJsonPretty(c, pretty);
  Serial.print("Config: ");
  Serial.println(pretty);
  Serial.println();
}

bool Config::hasRequiredConfigs(DynamicJsonDocument c)
{
  String id = c["id"];
  return id.length();
}
