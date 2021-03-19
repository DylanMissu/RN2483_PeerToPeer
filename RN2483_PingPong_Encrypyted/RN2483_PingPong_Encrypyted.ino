#include "AES.h"
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB

RN2483_P2P peerToPeer;

// please change this to your custom AES-key or generate a random one with a true random number generator.
// both devices mush have the same key.
byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

byte address[1] = {0x01};
byte targetAddress[1] = {0x00};

String str;
bool firstLoop = true;
bool ledState = false;

byte payload[10] = {0};

void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  
  usbSerial.begin(57600);
  loraSerial.begin(57600);
  
  peerToPeer.initLoRa(usbSerial, loraSerial);
  peerToPeer.setAesKey(key);
}

void handleMessage(const byte *payLoad){
    int value = (payLoad[0] << 8) | payLoad[1];
    value += 1;
    payload[0] = (value >> 8) & 0xff;
    payload[1] = value & 0xff;
    
    usbSerial.println();
    usbSerial.println("value: ");
    usbSerial.println(value);
    
    delay(200);
    peerToPeer.transmitMessage(usbSerial, loraSerial, payload, targetAddress);
}

void loop() {
    if (firstLoop) {
        peerToPeer.transmitMessage(usbSerial, loraSerial, payload, targetAddress);
    }
    
    peerToPeer.receiveMessage(usbSerial, loraSerial, handleMessage);
    toggle_led();       
    firstLoop = false;
}

void lora_autobaud()
{
    String response = "";
    while (response=="")
    {
        delay(1000);
        loraSerial.write((byte)0x00);
        loraSerial.write(0x55);
        loraSerial.println();
        loraSerial.println("sys get ver");
        response = loraSerial.readStringUntil('\n');
    }
}

void toggle_led()
{
  analogWrite(12, 40*ledState);
  digitalWrite(13, !ledState);
  ledState = !ledState;
}
