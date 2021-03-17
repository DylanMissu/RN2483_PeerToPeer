#include "AES.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB

#define padedLength(bytes)      bytes + N_BLOCK - bytes % N_BLOCK
    
#define payloadLength           10 //bytes


AES aes ;

// please change this to your AES-key or generate a random one with a true random number generator.
// both devices mush have the same key.
byte *key = (unsigned char*)"0123456789012345";
unsigned long long int my_iv = 0;

String str;
bool firstLoop = true;
bool ledState = false;

byte payload[payloadLength] = {0};

void setup() {
  //output LED pins
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  
  usbSerial.begin(57600);
  
  loraSerial.begin(57600);
  loraSerial.setTimeout(1000);
  lora_autobaud();

  aes.set_IV(my_iv);

  usbSerial.println("Initing LoRa");
  resetLora();
  
  usbSerial.println("starting loop");
}

void loop() {
    if (firstLoop) {
        transmit(payload);
    }
    //SerialUSB.println("waiting for a message");
    loraSerial.println("radio rx 0"); //wait for 60 seconds to receive

    str = loraSerial.readStringUntil('\n');
    if ( str.indexOf("ok") == 0 )
    {
        str = String("");
        while(str=="")
        {
            str = loraSerial.readStringUntil('\n');
        }
        if ( str.indexOf("radio_rx") == 0 )
        {
            usbSerial.println(str);
            str.remove(0,10);
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

            usbSerial.print("decrypted: ");
            for (int i=0; i<payloadLength; i++){
                usbSerial.print(decryptedData[i] >> 4, HEX);
                usbSerial.print(decryptedData[i] & 0x0f, HEX);
            }
            usbSerial.println();

            int value = (decryptedData[0] << 8) | decryptedData[1];
            value += 1;
            decryptedData[0] = (value >> 8) & 0xff;
            decryptedData[1] = value & 0xff;
            
            usbSerial.println();
            usbSerial.println("value: ");
            usbSerial.println(value);
            
            delay(200);
            transmit(decryptedData);
            toggle_led();
        }
        else
        {
            usbSerial.println("Received nothing");
        }
    }
    else
    {
        usbSerial.println("radio not going into receive mode");
        delay(1000);
    }
    firstLoop = false;
}

byte combineNibbles(byte MSN, byte LSN){
    return (MSN << 4) | (LSN);
}

void transmit(byte *bytes){
    int packetLength = payloadLength;
    
    byte iv [N_BLOCK];
    byte cipher [padedLength(packetLength)];

    // encrypt data with AES
    aes.get_IV(iv);
    aes.do_aes_encrypt(bytes, packetLength, cipher, key, 128, iv);

    // send data
    loraSerial.println("mac pause");
    str = loraSerial.readStringUntil('\n');
    
    loraSerial.print("radio tx ");
    for (int i=0; i<sizeof(cipher); i++){
        loraSerial.print(cipher[i] >> 4, HEX);
        loraSerial.print(cipher[i] & 0x0f, HEX);
    }
    loraSerial.println();
    
    str = loraSerial.readStringUntil('\n');
    str = loraSerial.readStringUntil('\n');
}

byte nibble(char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';

  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;

  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;

  return 0;  // Not a valid hexadecimal character
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

/*
 * This function blocks until the word "ok\n" is received on the UART,
 * or until a timeout of 3*5 seconds.
 */
int wait_for_ok()
{
  str = loraSerial.readStringUntil('\n');
  if ( str.indexOf("ok") == 0 ) {
    return 1;
  }
  else return 0;
}

void resetLora(){
    loraSerial.println("sys reset");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);

    loraSerial.println("sys get ver");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("mac pause");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set mod lora");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set freq 869100000");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set pwr 14");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set sf sf7");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set afcbw 41.7");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set rxbw 125");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set prlen 8");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set crc on");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set iqi off");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set cr 4/5");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set wdt 60000"); //disable for continuous reception
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set sync 12");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
    
    loraSerial.println("radio set bw 125");
    str = loraSerial.readStringUntil('\n');
    usbSerial.println(str);
}

void toggle_led()
{
  analogWrite(12, 40*ledState);
  digitalWrite(13, !ledState);
  ledState = !ledState;
}
