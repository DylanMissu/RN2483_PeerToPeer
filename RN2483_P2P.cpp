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
        str = String("");
        while(str=="")
        {
            str = loraSerial->readStringUntil('\n');
        }
        if ( str.indexOf("radio_rx") == 0 )
        {
            usbSerial->println(str);
            str.remove(0,10);
            byte packetAddress = combineNibbles(nibble(str[0]), nibble(str[1]));
            if (packetAddress != deviceAddress[0]) {
                usbSerial->println("packet received but address does not match");
            } else {
                str.remove(0,2);
                int strLen = str.length() + 1;
                char charArray[strLen];
                str.toCharArray(charArray, strLen);
                byte receivedBytes[strLen];
            
                for (int i=0; i<strLen/2; i++){
                    receivedBytes[i] = combineNibbles(nibble(charArray[i*2]), nibble(charArray[i*2+1]));
                }
            
                byte iv [N_BLOCK] ;
                byte decryptedData [payloadLength];
    
                aes.get_IV(iv);
                aes.do_aes_decrypt(receivedBytes, strLen, decryptedData, key, 128, iv);
    
                usbSerial->print("decrypted: ");
                for (int i=0; i<payloadLength; i++){
                    usbSerial->print(decryptedData[i] >> 4, HEX);
                    usbSerial->print(decryptedData[i] & 0x0f, HEX);
                }
                usbSerial->println();

                (*handleMessage)(decryptedData);
            }
            return true;
        }
        else
        {
            usbSerial->println("Received nothing");
            return false;
        }
    }
    else
    {
        usbSerial->println("radio not going into receive mode");
        //initLoRa();
        return false;
        delay(1000);
    }
};

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

void RN2483_P2P::setAddress(const byte address[1]){
    for (int i=0; i<sizeof(address)/sizeof(address[0]); i++){
        deviceAddress[i] = address[i];
    }
};

void RN2483_P2P::transmitMessage(byte *bytes,const byte targetAddress[1]){
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
    
    for (int i=0; i<1; i++){
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
