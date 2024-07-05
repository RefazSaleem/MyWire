#include "MyWire.h"

MyWire::MyWire() {
  baudRate = BAUD_RATE;
  lastData = 0;
  lastDevAddress = 0;
  lastDataPin = 0;
  lastSendTime = 0;
  senderAddress = 0;
  receivedData = 0;
}

void MyWire::begin() {
  bitTime = 1000000 / baudRate;
  randomSeed(analogRead(A0));
  key = random(0, 8);
}

void MyWire::sendBit(bool bit, uint8_t dataPin) {
  digitalWrite(dataPin, bit);
  unsigned long start = micros();
  while (micros() - start < bitTime);
}

void MyWire::sendBits(uint32_t data, uint8_t numBits, uint8_t dataPin) {
  uint8_t consecutiveOnes = 0;
  for (int i = numBits - 1; i >= 0; i--) {
    bool bit = bitRead(data, i);
    sendBit(bit, dataPin);

    if (bit) {
      consecutiveOnes++;
      if (consecutiveOnes == 4) {
        sendBit(0, dataPin);
        consecutiveOnes = 0;
      }
    } else {
      consecutiveOnes = 0;
    }
  }
}

uint8_t MyWire::encryptData(uint8_t data, uint8_t key) {
  return data ^ key;
}

uint8_t MyWire::calculateParity(uint32_t data, uint8_t numBits) {
  uint8_t parity = 0;
  for (int i = 0; i < numBits; i++) {
    parity ^= bitRead(data, i);
  }
  return parity;
}

void MyWire::send(uint8_t devAddress, uint32_t data, uint8_t dataPin) {
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, HIGH);

  uint8_t dataSize = 0;
  uint32_t tempData = data;
  while (tempData > 0) {
    dataSize++;
    tempData >>= 1;
  }

  uint32_t encryptedData = (dataSize <= 8) ? encryptData(data, key) : data;
  sendBits(0b1111, 4, dataPin);
  sendBits(key, 3, dataPin);
  sendBits(devAddress, 6, dataPin);
  sendBits(dataSize, 8, dataPin);
  sendBit(1, dataPin);
  sendBits(encryptedData, dataSize, dataPin);
  uint8_t parity = calculateParity(encryptedData, dataSize);
  sendBits(parity, 2, dataPin);
  sendBits(0b000, 3, dataPin);
  digitalWrite(dataPin, HIGH);

  lastData = data;
  lastDevAddress = devAddress;
  lastDataPin = dataPin;
  lastSendTime = millis();
}

void MyWire::resend() {
  if (millis() - lastSendTime <= 10000) {
    send(lastDevAddress, lastData, lastDataPin);
  }
}

bool MyWire::waitForStartSequence(uint8_t dataPin) {
  uint8_t sequence = 0;
  while (true) {
    sequence = (sequence << 1) | digitalRead(dataPin);
    delayMicroseconds(bitTime);
    if ((sequence & 0b1111) == 0b1111) {
      return true;
    }
  }
  return false;
}

bool MyWire::readBit(uint8_t dataPin) {
  bool bit = digitalRead(dataPin);
  delayMicroseconds(bitTime);
  return bit;
}

uint32_t MyWire::readBits(uint8_t numBits, uint8_t dataPin) {
  uint32_t data = 0;
  uint8_t consecutiveOnes = 0;
  for (int i = 0; i < numBits; i++) {
    bool bit = readBit(dataPin);
    if (bit) {
      consecutiveOnes++;
      if (consecutiveOnes == 4) {
        readBit(dataPin);
        consecutiveOnes = 0;
      }
    } else {
      consecutiveOnes = 0;
    }
    data = (data << 1) | bit;
  }
  return data;
}

bool MyWire::receive(uint8_t dataPin) {
  pinMode(dataPin, INPUT);

  if (!waitForStartSequence(dataPin)) {
    return false;
  }

  uint8_t receivedKey = readBits(3, dataPin);
  senderAddress = readBits(6, dataPin);
  uint8_t dataSize = readBits(8, dataPin);
  receivedData = readBits(dataSize, dataPin);

  if (dataSize <= 8) {
    receivedData = encryptData(receivedData, receivedKey);
  }

  uint8_t parity = readBits(2, dataPin);

  for (int i = 0; i < 3; i++) {
    if (readBit(dataPin) != LOW) {
      return false;
    }
  }

  if (parity != calculateParity(receivedData, dataSize)) {
    requestResend(senderAddress, dataPin);
    return false;
  }

  return true;
}

void MyWire::requestResend(uint8_t senderAddress, uint8_t dataPin) {
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, HIGH);

  sendBits(0b0000, 4, dataPin);
  sendBits(senderAddress, 6, dataPin);

  digitalWrite(dataPin, HIGH);
}

uint8_t MyWire::getSenderAddress() {
  return senderAddress;
}

uint32_t MyWire::getReceivedData() {
  return receivedData;
}
