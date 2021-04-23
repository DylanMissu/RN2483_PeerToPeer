#include <DHT.h>
#include <DHT_U.h>

#include "AES.h"
#include "RN2483_P2P.h"

#define loraSerial              Serial2
#define usbSerial               SerialUSB
#define btSerial                Serial

#define ledPin                  9
#define DHT11PIN                4
#define DHTTYPE                 DHT11

DHT dht(DHT11PIN, DHTTYPE);
RN2483_P2P peerToPeer(usbSerial, loraSerial);

// please change this to your custom AES-key or generate a random one with a true random number generator.
// this is used for encryption, so both devices mush have the same key.
const byte key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// set the address of the device where you want te send a packet to
const byte targetAddress[1] = {0x00};

// the adress of this device (The adresses can be the same. It won't loop back)
const byte deviceAddress[1] = {0x00};

bool ledState = false;
bool stat = true;

void setup() {
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(ledPin, OUTPUT);
    
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

    analogReadResolution(12);

    dht.begin();
}

void handleMessage(const byte *payload)
{
    usbSerial.println();

    double temp = ((payload[4]<<8)|(payload[5]))/100.0;
    double humid = ((payload[2]<<8)|(payload[3]))/100.0;
    double volt = ((payload[4]<<8)|(payload[5]))/100.0;
    double amp = ((payload[2]<<8)|(payload[3]))/100.0;
    double power = ((payload[0]<<8)|(payload[1]))/100.0;
    
    usbSerial.print("Temperature: ");
    usbSerial.println(temp);
    
    usbSerial.print("Humitity: ");
    usbSerial.println(humid);
    
    usbSerial.print("Voltage: ");
    usbSerial.println(volt);
    
    usbSerial.print("Amps: ");
    usbSerial.println(amp);
    
    usbSerial.print("Power: ");
    usbSerial.println(power);

    usbSerial.println();

    // pulse status led
    pulse();
}

void loop() 
{
    // uncomment for the transmitter
    transmit(); 

    // uncomment for the receiver
    //stat = peerToPeer.receiveMessage(handleMessage);
}

void transmit() {
    byte tempPayload[10] = {0};    

    // take average values over 1 second
    int sumVolt = 0;
    int sumAmp = 0;
    double sumTemp = 0;
    double sumHumid = 0;
    for (int i=0; i<10; i++) {        
        sumTemp += (double)dht.readTemperature();
        sumHumid += (double)dht.readHumidity();
        sumAmp += analogRead(A1);
        sumVolt += analogRead(A0);
        delay(100);
    }

    double volt = (sumVolt/10)*(3.3 / pow(2,12))*(1+(100000/8200));
    double amp = (sumAmp/10)*(3.3 / pow(2,12))/0.5;
    double temp = sumTemp/10.0;
    double humid = sumHumid/10.0;

    // display values on serial moditor
    SerialUSB.print("Temperature: ");
    SerialUSB.println(temp);
    
    SerialUSB.print("Humidity: ");
    SerialUSB.println(humid);

    SerialUSB.print("Volt: ");
    SerialUSB.println(volt);
    
    SerialUSB.print("Amp: ");
    SerialUSB.println(amp);
    
    SerialUSB.print("Pow: ");
    SerialUSB.println(volt*amp);
    
    SerialUSB.println();

    // add values to payload for transmission through lorawan
    tempPayload[0] = (int)(volt*amp*100)>>8;
    tempPayload[1] = (int)(volt*amp*100)&0x00ff;
    
    tempPayload[2] = (int)(amp*100)>>8;
    tempPayload[3] = (int)(amp*100)&0x00ff;
    
    tempPayload[4] = (int)(volt*100)>>8;
    tempPayload[5] = (int)(volt*100)&0x00ff;

    tempPayload[6] = (int)(sumTemp*10)>>8;
    tempPayload[7] = (int)(sumTemp*10)&0x00ff;
    
    tempPayload[8] = (int)(sumHumid*10)>>8;
    tempPayload[9] = (int)(sumHumid*10)&0x00ff;

    // transmit payload
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
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
}
