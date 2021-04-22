#include "AES.h"
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB

RN2483_P2P peerToPeer(usbSerial, loraSerial);

// please change this to your custom AES-key or generate a random one with a true random number generator.
// this is used for encryption, so both devices mush have the same key.
const byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

const byte targetAddress[1] = {0x00};
const byte deviceAddress[1] = {0x00};

bool ledState = false;
bool stat = true;

void setup() {
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(13, OUTPUT);
    
    pulse();
    
    usbSerial.begin(115200);
    loraSerial.begin(57600);
    loraSerial.setTimeout(1000);
    lora_autobaud();

    usbSerial.println("initializing...");
    
    peerToPeer.initLoRa();
    peerToPeer.setAesKey(key);
    peerToPeer.setAddress(deviceAddress);
}

void handleMessage(const byte *payload)
{
    // print value to the console
    usbSerial.println();
    
    usbSerial.print("Voltage: ");
    double volt = ((payload[4]<<8)|(payload[5]))/100.0;
    usbSerial.println(volt);
    
    usbSerial.print("Amps: ");
    double amp = ((payload[2]<<8)|(payload[3]))/100.0;
    usbSerial.println(amp);
    
    usbSerial.print("Power: ");
    double power = ((payload[0]<<8)|(payload[1]))/100.0;
    usbSerial.println(power);

    usbSerial.println();
    
    delay(200);
    pulse();
}

void loop() 
{
    //transmit(); 

    stat = peerToPeer.receiveMessage(handleMessage);
}

void transmit() {
    byte tempPayload[10] = {0};    
    
    int sumVolt = 0;
    int sumAmp = 0;
    for (int i=0; i<10; i++) {
        sumAmp += analogRead(A1);
        sumVolt += analogRead(A0);
        delay(50);
    }

    double volt = (sumVolt/10)*(3.3 / pow(2,10))*(1+(100000/8200));
    double amp = (sumAmp/10)*(3.3 / pow(2,10))/0.5;

    SerialUSB.print("Volt: ");
    SerialUSB.println(volt);
    
    SerialUSB.print("Amp: ");
    SerialUSB.println(amp);
    
    SerialUSB.print("Pow: ");
    SerialUSB.println(volt*amp);
    
    SerialUSB.println();

    tempPayload[0] = (int)(volt*amp*100)>>8;
    tempPayload[1] = (int)(volt*amp*100)&0x00ff;
    
    tempPayload[2] = (int)(amp*100)>>8;
    tempPayload[3] = (int)(amp*100)&0x00ff;
    
    tempPayload[4] = (int)(volt*100)>>8;
    tempPayload[5] = (int)(volt*100)&0x00ff;
    
    peerToPeer.transmitMessage(tempPayload, targetAddress);
    pulse();

    delay(1000);
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

void pulse()
{
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100);
}
