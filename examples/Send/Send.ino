#include <Bofu.h>

#define TRANSMIT_PIN 13
#define REPEAT_COMMAND 8
#define DEBUG true

Bofu::Transmit bofu = Bofu::Transmit();

Bofu::Message readMessage() {
  uint32_t message = 0;
  byte message_bytes[4];
  Serial.readBytes(message_bytes, 4);
  for(int i = 0; i < 4; i++) {
    message += static_cast<uint32_t>(message_bytes[3 - i]) << (i*8);
  }
  return Bofu::Message(message);
}

void setup() {
  Serial.begin(9600); // Used for error messages even with DEBUG set to false
  if (DEBUG) Serial.println("Starting up...");

  if (DEBUG) Serial.println("Init Bofu...");
  bofu.setRepeat(REPEAT_COMMAND);
  bofu.setPin(TRANSMIT_PIN);
}

void loop()
{
  if(Serial.available() >= 4) {
    Bofu::Message m = readMessage();

    Serial.print("Sending Message: ");
    Serial.print(m.getChecksum(),HEX);
    Serial.print(m.getData(), HEX);
    Serial.println("");
    bofu.sendMessage(m);
  }
}