#include <Bofu.h>

#define DEBUG true

#define RECEIVE_PIN 2 // Interrupt 0
#define TIMEOUT 6000

Bofu::Receive bofu = Bofu::Receive();

void setup() {
  Serial.begin(115200); // Used for error messages even with DEBUG set to false
  if (DEBUG) Serial.println("Starting up...");

  if (DEBUG) Serial.println("Init Bofu...");
  bofu.setPin(RECEIVE_PIN);
  bofu.setTimeout(TIMEOUT);
  bofu.startListening();
}

void loop()
{
  Bofu::Message m;

  if(bofu.available()) {
    m = bofu.readMessage();
    uint8_t checksum = m.getChecksum();
    uint32_t message = m.getData();

    Serial.println("----------------------------");
    Serial.print(  "Recieved Message: ");
    // To manually pad zeros
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