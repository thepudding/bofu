#include <Bofu.h>

#define DEBUG true

#define RECEIVE_PIN 29
#define TIMEOUT 6000

using Receive = Bofu::Receive;

void setup() {
  Serial.begin(115200); // Used for error messages even with DEBUG set to false
  if (DEBUG) Serial.println("Starting up...");

  if (DEBUG) Serial.println("Init Bofu...");
  Receive::setPin(RECEIVE_PIN);
  Receive::setTimeout(TIMEOUT);
  Receive::startListening();
}

void loop()
{
  Bofu::Message m;

  if(Receive::available()) {
    m = Receive::readMessage();
    uint8_t checksum = m.getChecksum();
    uint32_t message = m.getData();

    Serial.println("----------------------------");
    Serial.print(  "Recieved Message: ");
    // To manually pad zeros
    // from https://stackoverflow.com/a/62466324
    Serial.print(checksum>>4, HEX);
    Serial.print(checksum&0x0F,HEX);
    // only the last byte should contain zeros
    Serial.print(message>>28, HEX);
    Serial.print(message>>24&0x0F, HEX);
    Serial.println(message&0x00FFFFFF, HEX);

    Serial.print("Checksum: ");
    if(m.validateChecksum()) {
      Serial.println("pass");
    } else {
      Serial.println("fail");
      Serial.print("\tReceived: ");
      Serial.println(checksum, HEX);
      Serial.print("\tExpected: ");
      m.recomputeChecksum();
      Serial.println(m.getChecksum(), HEX);
    }

    Serial.print("Channel: ");
    Serial.print(Bofu::toString(m.getChannel()));
    Serial.print(", Command: ");
    Serial.println(Bofu::toString(m.getCommand()));
  }
}