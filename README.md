# RN2483_PingPong
 
 This is an example on how to use the RN2483 LoRaWan modem to acheive a secure peer to peer communication.
 
 In this example it broadcasts data and afterwards it starts listening. if the module then receives a packet, it will print it to a serial monitor and broadcast another packet. If two of these devices are in reach of eachother they will constantly transmit back and forth.
 
 In the encryprion example there is a library that handles encryption and most of the serial commands and data handling to make it easy for the user to use this library. the library also handles adressing.
 
 The encryption library used is (this AES library)[https://github.com/bigfighter/arduino-AES]
 
 This code was tested on a RN2483A with firmware version 1.0.4.
