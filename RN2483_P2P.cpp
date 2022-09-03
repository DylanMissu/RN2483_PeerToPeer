#include <Arduino.h>
#include "AES.h"
#include "RN2483_P2P.h"

RN2483_P2P::RN2483_P2P(Stream &usbSerial, Stream &loraSerial){
    this->usbSerial = &usbSerial;
    this->loraSerial = &loraSerial;
};

void RN2483_P2P::initLoRa(){
    for (int i=0; i<16; i++){
        loraSerial->println(InitCommandList[i]);
        str = loraSerial->readStringUntil('\n');
        usbSerial->println(str);
    }
    aes.set_IV(my_iv);
};

bool RN2483_P2P::receiveMessage(void (*handleMessage)(const byte *payload)){
    loraSerial->println("radio rx 0"); //wait for 60 seconds to receive
    
    str = loraSerial->readStringUntil('\n');

    if ( str.indexOf("ok") == 0 )
    {
        handleIncommingMessage(handleMessage);
    }
    else
    {
        usbSerial->println("radio not going into receive mode");
        //initLoRa();
        return false;
        delay(1000);
    }
};

int RN2483_P2P::handleIncommingMessage(void (*handleMessage)(const byte *payload)){
    str = String("");
    while(str=="")
    {
        str = loraSerial->readStringUntil('\n');
    }
    if ( str.indexOf("radio_rx") == 0 )
    {
        // print received cipher to terminal
        usbSerial->println(str); 

        // remove "radio_rx " from response
        str.remove(0,10);

        // convert received hex string into a byte array
        int packetLength = (str.length()-1)/2;
        byte packet[packetLength];
        hexStringToByteArray(str, packet, packetLength);

        // check if the address is correct
        bool addressCorrect = false;
        for (int i=0; i<ADRESS_SIZE; i++){
            addressCorrect += (packet[i] == deviceAddress[i]);
        }

        // decrypt message if the adress is correct
        if (!addressCorrect) {
            usbSerial->println("packet received but address does not match");
        } else {
            decryptMessage(packet, handleMessage, packetLength);
        }
        return 0;
    }
    else
    {
        usbSerial->println("Received nothing");
        return -1;
    }
}

void RN2483_P2P::hexStringToByteArray(String hex, byte *decoded, int numBytes){
    int nibbles = hex.length() - 1;
    int bytes = 0;
    
    if (nibbles%2 == 1){
        decoded = {0x00};
    }

    for (int i=0; i<nibbles; i++){
        if (!isHexadecimalDigit(hex[i])){
            decoded = {0x00};
        }
    }

    for (int i=0; i<numBytes; i++){
        decoded[i] = combineNibbles(nibble(hex[i*2]), nibble(hex[i*2+1]));
    }
}

void RN2483_P2P::decryptMessage(const byte *packet, void (*handleMessage)(const byte *payload), const int packetLength){
    byte cipher[packetLength - ADRESS_SIZE];

    for (int i=0; i<packetLength - ADRESS_SIZE; i++){
        cipher[i] = packet[i+ADRESS_SIZE];
    }

    byte iv [N_BLOCK];
    byte decryptedData [payloadLength];

    aes.get_IV(iv);
    aes.do_aes_decrypt(cipher, packetLength - ADRESS_SIZE, decryptedData, key, 128, iv);

    usbSerial->print("decrypted: ");
    for (int i=0; i<payloadLength; i++){
        usbSerial->print(decryptedData[i] >> 4, HEX);
        usbSerial->print(decryptedData[i] & 0x0f, HEX);
    }
    usbSerial->println();

    (*handleMessage)(decryptedData);
}

byte RN2483_P2P::nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
};

void RN2483_P2P::setPayloadLength(int Length) {
    payloadLength = Length;
};

void RN2483_P2P::setAesKey(const byte AESKey[AES_BITS/8]){
    for (int i=0; i<AES_BITS/8; i++){
        key[i] = AESKey[i];
    }
};

void RN2483_P2P::setAddress(const byte address[ADRESS_SIZE]){
    for (int i=0; i<ADRESS_SIZE; i++){
        deviceAddress[i] = address[i];
    }
};

void RN2483_P2P::transmitMessage(byte *bytes,const byte targetAddress[ADRESS_SIZE]){
    int packetLength = payloadLength;
    
    byte iv [N_BLOCK];
    byte cipher [padedLength(packetLength)];

    // encrypt data with AES
    aes.get_IV(iv);
    aes.do_aes_encrypt(bytes, packetLength, cipher, key, AES_BITS, iv);

    // send data
    loraSerial->println("mac pause");
    str = loraSerial->readStringUntil('\n');
    
    loraSerial->print("radio tx ");
    
    for (int i=0; i<ADRESS_SIZE; i++){
        loraSerial->print(targetAddress[i] >> 4, HEX);
        loraSerial->print(targetAddress[i] & 0x0f, HEX);
    }
    for (int i=0; i<sizeof(cipher)/sizeof(cipher[0]); i++){
        loraSerial->print(cipher[i] >> 4, HEX);
        loraSerial->print(cipher[i] & 0x0f, HEX);
    }
    
    loraSerial->println();
    
    str = loraSerial->readStringUntil('\n');
    str = loraSerial->readStringUntil('\n');
};
