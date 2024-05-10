#include <Bofu.h>

#define TRANSMIT_PIN 13
#define REPEAT_COMMAND 8
#define DEBUG true

uint32_t message = 0;

Bofu::Transmit bofu = Bofu::Transmit();

void readMessage() {
  message = 0;
  byte message_bytes[4];
  Serial.readBytes(message_bytes, 4);
  for(int i = 0; i < 4; i++) {
    message += message_bytes[3 - i] << (i*8);
  }
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
    readMessage();

    Serial.print("Sending Message: ");
    Serial.print(message, HEX);
    Serial.println("");
    bofu.sendMessage(message);
  }
}