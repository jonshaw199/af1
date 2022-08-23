#include "espnowEnt.h"
#include "espnowHandler/espnowHandler.h"
#include "messageHandler/messageHandler.h"

bool ESPNowEnt::handleInboxMsg(JSMessage m)
{
  switch (m.getType())
  {
  case TYPE_HANDSHAKE_REQUEST:
    Serial.println("Handshake request message in inbox");
    ESPNowHandler::receiveHandshakeRequest(m);
    ESPNowHandler::sendHandshakeResponses({m.getSenderID()});
    return true;
  case TYPE_HANDSHAKE_RESPONSE:
    Serial.println("Handshake response message in inbox");
    ESPNowHandler::receiveHandshakeResponse(m);
    true;
  }

  return Base::handleInboxMsg(m);
}

void ESPNowEnt::setInboxMessageHandler()
{
  MessageHandler::setInboxMsgHandler(handleInboxMsg);
}

bool ESPNowEnt::handleOutboxMsg(JSMessage m)
{
  ESPNowHandler::sendMsg(m);
  return true;
}

void ESPNowEnt::setOutboxMessageHandler()
{
  MessageHandler::setOutboxMsgHandler(handleOutboxMsg);
}
