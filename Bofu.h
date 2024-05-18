#ifndef BOFU_H
#define BOFU_H

#include <Arduino.h>
#include <stdint.h>

/**
 * @brief Protocol Definition
 * Pulse lengths, and command size
 */
#define AGC1_PULSE                   4885  // 216 samples
#define AGC2_PULSE                   2450  // 108 samples
#define AGC3_PULSE                   1700  // 75 samples
#define RADIO_SILENCE                5057  // 223 samples

#define PULSE                        340   // 15 samples

#define MESSAGE_BIT_LENGTH 41
#define MESSAGE_TIMINGS_LENGTH MESSAGE_BIT_LENGTH * 2 + 4
#define BUFFER_SIZE 20

namespace Bofu {
  enum class Command: uint16_t {
    UP_SINGLE       = 0x10C0,
    UP_HOLD         = 0x00C0,
    DOWN_SINGLE     = 0x1010,
    DOWN_HOLD       = 0x0010,
    STOP            = 0x0150,
    PAIR            = 0x1140,
    LIMIT           = 0x0120,
    CHANGE_DIR_UP   = 0x0190,
    CHANGE_DIR_DOWN = 0x01A0
  };

  enum class Channel: uint8_t {
    ONE   =0x01,
    TWO   =0x02,
    THREE =0x03,
    FOUR  =0x04,
    FIVE  =0x05,
    ALL   =0x0F
  };

  String toString(Command cmd);
  String toString(Channel ch);

  class Message {
    public:
      Message();
      Message(uint32_t message);
      Message(uint32_t message, uint8_t checksum);
      Message(uint16_t remote_id, Channel channel, Command command);

      uint32_t getData();
      uint16_t getRemoteID();
      void setRemoteID(uint16_t remote_id);
      Channel getChannel();
      void setChannel(Channel channel);
      Command getCommand();
      void setCommand(Command command);
      uint8_t getChecksum();
      void recomputeChecksum();
      bool validateChecksum();

      static uint8_t calculateChecksum(uint32_t message);
    private:
      uint32_t data;
      uint8_t checksum;
  };

  class Transmit {
    public:
      Transmit();

      void setPin(int pin);
      void setRepeat(int repeat);

      void sendMessage(Message message);

    private:
      int pin;
      int repeat;

      void send(PinStatus status, int delay);
      void sendBit(PinStatus bit);
      void sendOnce(Message message);
      void agc();
  };

  class MessageBuffer {
    public:
      MessageBuffer();
      void enqueue(Message message);
      Message dequeue();
      bool isEmpty();
      bool hasOverflowed();
    private:
      Message data[BUFFER_SIZE];
      int front;
      int back;
      bool overflow;
  };

  class Receive {
    public:
      Receive();

      void setPin(int pin);
      void setTimeout(unsigned int timeout);

      // DEBUG
      int getAGCCount();
      unsigned int * getTimings();
      int getTimingsLength();

      void startListening();
      void stopListening();

      bool available();
      Message readMessage();

    private:
      int pin;
      unsigned int timeout;

      unsigned int agc_count;

      Message parseTimings();
      static void handleInterrupt(Receive *caller);

      volatile int change_count;
      unsigned int timings[MESSAGE_TIMINGS_LENGTH];

      MessageBuffer messages;
  };
}
#endif