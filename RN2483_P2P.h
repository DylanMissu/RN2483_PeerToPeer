#ifndef RN2483_P2P_H
#define RN2483_P2P_H

#include <Arduino.h>
#include "AES.h"

#define padedLength(bytes)          bytes + N_BLOCK - bytes % N_BLOCK
#define combineNibbles(MSN, LSN)    (MSN << 4) | (LSN)

#define AES_BITS 128

class RN2483_P2P {
    public:
        RN2483_P2P(Stream &usbSerial, Stream &loraSerial);
        void initLoRa();
        bool receiveMessage(void (*handleMessage)(const byte *payload));
        void transmitMessage(byte *bytes, const byte *targetAddress);
        void setPayloadLength(int Length);
        void setAesKey(const byte AESKey[AES_BITS/8]);
        void setAddress(const byte address[1]);

    private:
        byte nibble(char c);

    private:
        String str = "";
        int payloadLength = 10;
        byte deviceAddress[1] = {0x01};
        Stream *loraSerial;
        Stream *usbSerial;

    private:
        AES aes;
        byte key[AES_BITS/8] = {0x00};
        unsigned long long int my_iv = 0;

    private:
        char *InitCommandList[16] = {
            "sys reset",
            "sys get ver",  
            "mac pause",
            "radio set mod lora",
            "radio set freq 869100000",
            "radio set pwr 14",
            "radio set sf sf7",
            "radio set afcbw 41.7",
            "radio set rxbw 125",
            "radio set prlen 8",
            "radio set crc on",
            "radio set iqi off",
            "radio set cr 4/5",
            "radio set wdt 60000",
            "radio set sync 12",
            "radio set bw 125"
        };
};

#endif
