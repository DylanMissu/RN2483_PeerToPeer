#include "AES.h"
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB

RN2483_P2P peerToPeer(usbSerial, loraSerial);

// please change this to your custom AES-key or generate a random one with a true random number generator.
// this is used for encryption, so both devices mush have the same key.
const byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

const byte targetAddress[1] = {0x00};
const byte deviceAddress[1] = {0x04};

bool ledState = false;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  
  usbSerial.begin(57600);
  loraSerial.begin(57600);
  
  peerToPeer.initLoRa();
  peerToPeer.setAesKey(key);
  peerToPeer.setAddress(deviceAddress);

  byte tempPayload[10] = {0};
  peerToPeer.transmitMessage(tempPayload, targetAddress);
}

void handleMessage(const byte *payload)
{
    byte tempPayload[sizeof(payload)] = {0};
    
    //combine the two first bytes into an 16 bit integer
    uint16_t value = (payload[0] << 8) | payload[1];

    //increment it by one
    value += 1;

    //split again into two bytes
    tempPayload[0] = (value >> 8) & 0xff;
    tempPayload[1] = value & 0xff;

    // print value to the console
    usbSerial.println();
    usbSerial.print("value: ");
    usbSerial.println(value);
    
    // send result back to the other device
    delay(200);
    peerToPeer.transmitMessage(tempPayload, targetAddress);
    toggle_led();
}

void loop() 
{
    bool stat = peerToPeer.receiveMessage(handleMessage);
    
    if (stat){
        byte tempPayload[10] = {0};
        peerToPeer.transmitMessage(tempPayload, targetAddress);
    }
}

void toggle_led()
{
  analogWrite(12, 40*ledState);
  digitalWrite(13, !ledState);
  ledState = !ledState;
}
