# RN2483_PingPong
 
 This is an example on how to use the RN2483 LoRaWan modem to acheive a relatively secure peer to peer communication with another RN2483.
 
 This Arduino library is made for those who have a RN2483 on an existing board. Microchip doesn't advertise the use of peer to peer communication with their RN2483 chips because they were specifically desgined for the LoRaWAN protocol stack. If you are a developer who wants to implement peer to peer communication it is recomended that you directly use the SX12xx lora modems from the start.
 
 This library exploits the limited peer to peer possibilities of the RN2483 to implement secure communication with another RN2483. The connection is stateless so you can broadcast a packet and the device with the correct key and adress will decode and handle the packet.
 
 This library is still in early development. Feel free to add issues or create pull requests if you want to see features added.
 
 planned features:
 * CRC
 * Handshaking
 * Compatibility with other lora modems
 * replay attack prevention
 
 Here is an example sketch:
 
 ```C++
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB

// 128bit aes key
// Both devices mush have the same AES-key.
const byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// set the address of the device where you want te send a packet to
const byte targetAddress[1] = {0x00};

// the adress of this device (The adresses can be the same. It won't loop back)
const byte deviceAddress[1] = {0x00};

RN2483_P2P peerToPeer(usbSerial, loraSerial);

void setup() {

    usbSerial.begin(115200);
    loraSerial.begin(57600);
    loraSerial.setTimeout(1000);
    lora_autobaud();

    usbSerial.println("initializing...");
    
    peerToPeer.initLoRa();
    peerToPeer.setAesKey(key);
    peerToPeer.setAddress(deviceAddress);
}


void loop() 
{
    // uncomment for the transmitter
    //byte payload[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    //peerToPeer.transmitMessage(payload, targetAddress);

    // uncomment for the receiver
    peerToPeer.receiveMessage(handleMessage);
}

void handleMessage(const byte *payload)
{
    // handle the received payload
    
    for (int i=0; i < 10; i++){
        // print received payload to the console
        usbSerial.println( (int) payload[i] );
    }
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
 ```
 The code where this library is build upon is [this Code example.](https://github.com/jpmeijers/RN2483-Arduino-p2p-examples) <br>
 The encryption library used is [this AES library.](https://github.com/bigfighter/arduino-AES)
 
 The code was tested on a SODAQ ExpLoRer with an RN2483A with firmware version 1.0.4 but should work on all boards with the RN2483.
