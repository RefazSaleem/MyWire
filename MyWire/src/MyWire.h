#ifndef MYWIRE_H
#define MYWIRE_H

#include <Arduino.h>

#define BAUD_RATE 152000

class MyWire {
private:
  uint32_t baudRate;
  uint32_t bitTime;
  uint8_t key;
  uint32_t lastData;
  uint8_t lastDevAddress;
  uint8_t lastDataPin;
  unsigned long lastSendTime;
  uint8_t senderAddress;
  uint32_t receivedData;

  void sendBit(bool bit, uint8_t dataPin);
  void sendBits(uint32_t data, uint8_t numBits, uint8_t dataPin);
  uint8_t encryptData(uint8_t data, uint8_t key);
  uint8_t calculateParity(uint32_t data, uint8_t numBits);
  bool waitForStartSequence(uint8_t dataPin);
  bool readBit(uint8_t dataPin);
  uint32_t readBits(uint8_t numBits, uint8_t dataPin);

public:
  MyWire();
  void begin();
  void send(uint8_t devAddress, uint32_t data, uint8_t dataPin);
  void resend();
  bool receive(uint8_t dataPin);
  uint8_t getSenderAddress();
  uint32_t getReceivedData();
  void requestResend(uint8_t senderAddress, uint8_t dataPin);
};

#endif
