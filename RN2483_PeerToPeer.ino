#include "AES.h"
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB
#define btSerial                Serial

#define LED_PIN                 13

#define ADC_RESOLUTION          12      //Bits
#define SUPPLY_VOLTAGE          3.3     //Volt

// Both devices mush have the same AES-key.
const byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// set the address of the device where you want te send a packet to
const byte targetAddress[1] = {0x00};

// the adress of this device (The adresses can be the same. It won't loop back)
const byte deviceAddress[1] = {0x00};

bool ledState = false;
bool stat = true;

DHT dht(DHT11PIN, DHTTYPE);
RN2483_P2P peerToPeer(usbSerial, loraSerial);

bool input1 = 0;
bool input2 = 0;
bool input3 = 0;
bool input4 = 0;
bool input5 = 0;
bool input6 = 0;

void setup() {
    pinMode(LED_PIN, OUTPUT);

    pinMode(INPUT1, INPUT);
    pinMode(INPUT2, INPUT);
    pinMode(INPUT3, INPUT);
    pinMode(INPUT4, INPUT);
    pinMode(INPUT5, INPUT);
    pinMode(INPUT6, INPUT);
    
    pulse();

    btSerial.begin(9600);
    usbSerial.begin(115200);
    loraSerial.begin(57600);
    loraSerial.setTimeout(1000);
    lora_autobaud();

    usbSerial.println("initializing...");
    
    peerToPeer.initLoRa();
    peerToPeer.setAesKey(key);
    peerToPeer.setAddress(deviceAddress);

    analogReadResolution(ADC_RESOLUTION);
}


void loop() 
{
    // uncomment for the transmitter
    //transmit(); 

    // uncomment for the receiver
    peerToPeer.receiveMessage(handleMessage);
}

void handleMessage(const byte *payload)
{
    // pulse status led
    pulse();

    if (payload[0] == payload[1]) {
        input1 = (payload[0] >> 8) & 1U;
        input2 = (payload[0] >> 7) & 1U;
        input3 = (payload[0] >> 6) & 1U;
        input4 = (payload[0] >> 5) & 1U;
        input5 = (payload[0] >> 4) & 1U;
        input6 = (payload[0] >> 3) & 1U;
    } else {
        // error redundancy check fail
    }

    digitalWrite(/**/, input1);
    digitalWrite(/**/, input2);
    digitalWrite(/**/, input3);
    digitalWrite(/**/, input4);
    digitalWrite(/**/, input5);
    digitalWrite(/**/, input6);

    // print to usb serial
    printPayload(payload);
}

void transmit() {
    byte tempPayload[2] = {0};    

    tempPayload[0] = 
                (digitalRead(INPUT1) << 8) |
                (digitalRead(INPUT2) << 7) |
                (digitalRead(INPUT3) << 6) |
                (digitalRead(INPUT4) << 5) |
                (digitalRead(INPUT5) << 4) |
                (digitalRead(INPUT6) << 3);

    // redundancy
    tempPayload[1] = tempPayload[0];

    // transmit payload through LoRa
    peerToPeer.transmitMessage(tempPayload, targetAddress);

    // pulse led to display that the data has been sent
    pulse();
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
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
}
