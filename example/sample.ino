#include "MyWire.h"

MyWire myWire;

void setup() {
  Serial.begin(9600);
  myWire.begin();
}

void loop() {
  uint8_t dev_addr = 0b000111;
  uint8_t pin = 4;

  myWire.send(dev_addr, 0xAAAA, pin);
  delay(1000);
  myWire.resend();
  if (myWire.receive(pin)) {
    uint8_t senderAddress = myWire.getSenderAddress();
    uint32_t receivedData = myWire.getReceivedData();
    Serial.print("Received data from address: ");
    Serial.print(senderAddress, BIN);
    Serial.print(" Data: ");
    Serial.println(receivedData, BIN);
  } else {
    myWire.resend();
  }
}
