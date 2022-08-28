#include <ArduinoJson.h>

#include "wsEnt.h"
#include "pre.h"
#include "stateManager/stateManager.h"

WebSocketClient WSEnt::webSocketClient;
// Use WiFiClient class to create TCP connections
WiFiClient WSEnt::client;

char path[] = STRINGIFY(WS_PATH);
char host[] = STRINGIFY(WS_HOST);

void WSEnt::setup()
{
  Base::setup();

  connectToWifi();

  // Connect to the websocket server
  if (client.connect(STRINGIFY(WS_HOST), WS_PORT))
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("Connection failed.");
    while (1)
    {
      // Hang on failure
    }
  }

  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client))
  {
    Serial.println("Handshake successful");
  }
  else
  {
    Serial.println("Handshake failed.");
    while (1)
    {
      // Hang on failure
    }
  }
}

void WSEnt::loop()
{
  Base::loop();

  String data;

  if (client.connected())
  {
    webSocketClient.getData(data);
    if (data.length() > 0)
    {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, data);
      pushInbox(doc);
    }
  }
  else
  {
    Serial.println("Client disconnected.");
    while (1)
    {
      // Hang on disconnect.
    }
  }

  // wait to fully let the client disconnect
  // delay(3000);
}
