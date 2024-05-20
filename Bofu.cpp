#include "Bofu.h"

namespace Bofu {
  String toString(Command cmd) {
    switch (cmd) {
      case Command::UP_SINGLE: return "UP_SINGLE";
      case Command::UP_HOLD: return "UP_HOLD";
      case Command::DOWN_SINGLE: return "DOWN_SINGLE";
      case Command::DOWN_HOLD: return "DOWN_HOLD";
      case Command::STOP: return "STOP";
      case Command::PAIR: return "PAIR";
      case Command::LIMIT: return "LIMIT";
      case Command::CHANGE_DIR_UP: return "CHANGE_DIR_UP";
      case Command::CHANGE_DIR_DOWN: return "CHANGE_DIR_DOWN";
      default: return "UNKNOWN";
    }
  }
  String toString(Channel ch) {
    switch (ch) {
      case Channel::ONE: return "ONE";
      case Channel::TWO: return "TWO";
      case Channel::THREE: return "THREE";
      case Channel::FOUR: return "FOUR";
      case Channel::FIVE: return "FIVE";
      case Channel::ALL: return "ALL";
      default: return "UNKNOWN";
    }
  }

  Message::Message() {
    this->data = 0;
    this->checksum = 1;
  }
  Message::Message(uint32_t message) {
    this->data = message;
    recomputeChecksum();
  }
  Message::Message(uint32_t message, uint8_t checksum) {
    this-> data = message;
    this->checksum = checksum;
  }
  Message::Message(uint16_t remote_id, Channel channel, Command command) {
    this->data = 0;
    setRemoteID(remote_id);
    setChannel(channel);
    setCommand(command);
    recomputeChecksum();
  }

  uint32_t Message::getData() {
    return this->data;
  }
  uint16_t Message::getRemoteID(){
    // remoteID is lowest 16 bits
    return this->data&0xFFFF;
  }
  void Message::setRemoteID(uint16_t remote_id){
    this->data |= remote_id;
  }
  Channel Message::getChannel(){
    // channel is 3rd nibble
    return static_cast<Channel>((this->data & 0x000F0000) >> 16);
  }
  void Message::setChannel(Channel channel){
    this->data |= static_cast<uint32_t>(channel) << 16;
  }
  Command Message::getCommand(){
    // command code is nibble 4-6
    return static_cast<Command>((this->data & 0xFFF00000) >> 16);
  }
  void Message::setCommand(Command command){
    this->data |= static_cast<uint32_t>(command) << 16;
  }
  uint8_t Message::getChecksum(){
    return this->checksum;
  }
  void Message::recomputeChecksum(){
    this->checksum = Message::calculateChecksum(this->data);
  }
  bool Message::validateChecksum(){
    return this->checksum == Message::calculateChecksum(this->data);
  }

  uint8_t Message::calculateChecksum(uint32_t message){
    uint8_t sum = 0;
    for(int i = 0; i < 32; i += 8) {
      sum += (message >> i) & 0xFF;
    }
    return 1 - sum;
  }

  Transmit::Transmit() {
    this->pin = -1;
    this->repeat = 8;
  }

  void Transmit::setPin(int pin) {
    this->pin = pin;
  }

  void Transmit::setRepeat(int repeat) {
    this->repeat = repeat;
  }

  void Transmit::sendMessage(Message message) {
    pinMode(this->pin, OUTPUT);

    for(int i = 0; i < this->repeat; i++) {
      sendOnce(message);
    }

    send(LOW, RADIO_SILENCE);

    digitalWrite(this->pin, LOW);
  }

  void Transmit::send(uint8_t status, int delay) {
    digitalWrite(this->pin, status);
    //PORTB = PORTB D13high; // If you wish to use faster PORTB calls instead
    delayMicroseconds(delay);
  }

  void Transmit::sendBit(uint8_t bit) {
    send(LOW, PULSE);
    send(HIGH, PULSE);
    send(bit, PULSE);
  }

  void Transmit::sendOnce(Message message) {
    uint32_t data = message.getData();
    uint8_t checksum = message.getChecksum();

    agc();

    for(int i = 0; i < 32; i++) {
      sendBit((data >> i) & 1);
    }

    for(int i = 0; i < 8; i++) {
      sendBit((checksum >> i) & 1);
    }
  }

  void Transmit::agc() {
    send(HIGH, AGC1_PULSE);
    send(LOW, AGC2_PULSE);
    send(HIGH, AGC3_PULSE);
  }

  MessageBuffer::MessageBuffer() {
    this->front = 0;
    this->back = 0;
    this->overflow = false;
  }
  void MessageBuffer::enqueue(Message message){
    this->data[this->back++] = message;
    this->back %= BUFFER_SIZE;
    this->overflow = this->back == this->front;
  }
  /**
   * @brief Dequeue a message from the buffer. Does not check for an empty state
   *
   * @return Message
   */
  Message MessageBuffer::dequeue(){
    Message out;
    out = this->data[this->front++];
    this->front %= BUFFER_SIZE;
    return out;
  }
  bool MessageBuffer::isEmpty(){
    return this->front == this->back && !this->overflow;
  }
  bool MessageBuffer::hasOverflowed(){
    return this->overflow;
  }

  int Receive::pin = -1;
  unsigned int Receive::timeout = 10000;
  unsigned int Receive::agc_count = 0;
  MessageBuffer Receive::messages = MessageBuffer();
  unsigned int Receive::timings[MESSAGE_TIMINGS_LENGTH];
  volatile int Receive::change_count;

  void Receive::setPin(int pin) {
    Receive::pin = pin;
  }

  void Receive::setTimeout(unsigned int timeout) {
    Receive::timeout = timeout;
  }

  int Receive::getAGCCount() {
    return Receive::agc_count;
  }

  unsigned int * Receive::getTimings() {
    return Receive::timings;
  }

  int Receive::getTimingsLength() {
    return Receive::change_count;
  }

  void Receive::startListening() {
    attachInterrupt(digitalPinToInterrupt(Receive::pin),Receive::handleInterrupt,CHANGE);
  }

  void Receive::stopListening() {
    detachInterrupt(digitalPinToInterrupt(Receive::pin));
  }

  bool Receive::available() {
    return !Receive::messages.isEmpty();
  }

  Message Receive::readMessage() {
    return messages.dequeue();
  }

  Message Receive::parseTimings() {
    uint32_t data = 0;
    uint8_t checksum = 0;

    // 32 is hardcoded message length
    for(int i = 4, j = 0; j < 32; i += 2, j++) {
      data |= (timings[i] > PULSE + PULSE / 2) << j;
    }
    for(int i = 68, j = 0; j < 8; i += 2, j++) {
      checksum |= (timings[i] > PULSE + PULSE / 2) << j;
    }
    return Message(data, checksum);
  }
  // Some code from https://github.com/sui77/rc-switch/
  // Timings from https://github.com/akirjavainen/markisol
  // Interrupt must be static for pointers to work, so we must manually pass the
  // Receiver object rather than using the implicit `this`.
  void Receive::handleInterrupt() {
    // In the first interrupt, change_count will be zero, and the duration will
    // be time - 0, so almost certainly time out.
    static unsigned int change_count = 0;
    static unsigned long last_time = 0;

    const long time = micros();
    const unsigned int duration = time - last_time;

    last_time = time;
    Receive::change_count = change_count;

    // Check Timeout
    if(duration > Receive::timeout) {
      // If signal is high, the timeout was a low pulse. we'll check against the
      // first pulse length, so count = 1. If it's gone low, resetting to zero
      // is the default state.
      change_count = digitalRead(Receive::pin);
      return;
    }

    // Check sync pulse
    switch (change_count) {
      case 0:
        change_count++;
        return;
      case 1:
        if(duration < 4500 || 6000 < duration) {
          change_count = 0;
          return;
        }
        break;
      case 2:
        if(duration < 2300 || 2600 < duration) {
          change_count = 1;
          return;
        }
        break;
      case 3:
        if(duration < 1100 || 1900 < duration) {
          change_count = 0;
          return;
        } else {
          Receive::agc_count++;
        }
        break;
    }
    // Record next timing
    Receive::timings[change_count - 1] = duration;

    // Check message end.
    if(change_count == MESSAGE_TIMINGS_LENGTH) {
        Receive::messages.enqueue(Receive::parseTimings());
        change_count = 0;
    } else {
      change_count++;
    }
  }
}