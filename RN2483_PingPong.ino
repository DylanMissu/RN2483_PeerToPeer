#define loraSerial Serial2

String str;

bool firstLoop = true;
bool ledState = false;

void setup() {
  //output LED pin
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  led_off();
  
  // Open serial communications and wait for port to open:
  
  SerialUSB.begin(57600);
  
  loraSerial.begin(57600);
  loraSerial.setTimeout(1000);
  lora_autobaud();
  
  led_on();
  delay(1000);
  led_off();

  SerialUSB.println("Initing LoRa");
  resetLora();

  SerialUSB.println("starting loop");
}

void loop() {
    if (firstLoop) {
        transmit();
        delay(500);
    }
    SerialUSB.println("waiting for a message");
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
            str.remove(0,10);
            int strLen = str.length() + 1;
            char charArray[strLen];
            str.toCharArray(charArray, strLen);
            
            SerialUSB.println(nibble(charArray[0]), HEX);
            SerialUSB.println(nibble(charArray[1]), HEX);
            SerialUSB.println(nibble(charArray[2]), HEX);
            SerialUSB.println(nibble(charArray[3]), HEX);
            
            delay(500);
            transmit();
            toggle_led();
        }
        else
        {
            SerialUSB.println("Received nothing");
        }
    }
    else
    {
        SerialUSB.println("radio not going into receive mode");
        delay(1000);
    }
    firstLoop = false;
}

void transmit(){
    led_on();
    loraSerial.println("radio tx 1234");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    led_off();
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
    SerialUSB.println(str);

    loraSerial.println("sys get ver");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("mac pause");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    //  loraSerial.println("radio set bt 0.5");
    //  wait_for_ok();
    
    loraSerial.println("radio set mod lora");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set freq 869100000");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set pwr 14");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set sf sf7");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set afcbw 41.7");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set rxbw 125");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    //  loraSerial.println("radio set bitrate 50000");
    //  wait_for_ok();
    
    //  loraSerial.println("radio set fdev 25000");
    //  wait_for_ok();
    
    loraSerial.println("radio set prlen 8");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set crc on");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set iqi off");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set cr 4/5");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set wdt 60000"); //disable for continuous reception
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set sync 12");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
    
    loraSerial.println("radio set bw 125");
    str = loraSerial.readStringUntil('\n');
    SerialUSB.println(str);
}

void toggle_led()
{
  analogWrite(12, 50*ledState);
  ledState = !ledState;
}

void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}
