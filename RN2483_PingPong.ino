#define loraSerial Serial2
#define usbSerial SerialUSB

String str;

bool firstLoop = true;
bool ledState = false;

byte receivedBytes[10];

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

  usbSerial.println("Initing LoRa");
  resetLora();

  usbSerial.println("starting loop");
}

void loop() {
    if (firstLoop) {
        byte bytes[2] = {0, 0};
        transmit(bytes);
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

            receivedBytes[0] = combineNibbles(nibble(charArray[0]), nibble(charArray[1]));
            receivedBytes[1] = combineNibbles(nibble(charArray[2]), nibble(charArray[3]));

            int value = (receivedBytes[0] << 8) | receivedBytes[1];

            value += 1;

            receivedBytes[0] = (value >> 8) & 0xff;
            receivedBytes[1] = value & 0xff;
            
            usbSerial.println();
            usbSerial.println("value: ");
            usbSerial.println(value);
            
            delay(200);
            transmit(receivedBytes);
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
    loraSerial.println("mac pause");
    str = loraSerial.readStringUntil('\n');

    loraSerial.print("radio tx ");
    for (int i=0; i<sizeof(bytes); i++){
        loraSerial.print(bytes[i] >> 4, HEX);
        loraSerial.print(bytes[i] & 0x0f, HEX);
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
