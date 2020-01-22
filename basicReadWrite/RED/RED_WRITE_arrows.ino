/****************************************************************************** 
Toni Klopfenstein @ SparkFun Electronics
September 2015
https://github.com/sparkfun/CAN-Bus_Shield

This example sketch works with the CAN-Bus shield from SparkFun Electronics.

It reads the joystick for user inputs. 

Based off of joystick_demo.pde by Sukkin Pang.
Original file available at http://skpang.co.uk/catalog/arduino-canbus-shield-with-usd-card-holder-p-706.html

This code is beerware; if you see me (or any other SparkFun employee) at the local, 
and you've found our code helpful, please buy us a round!

For the official license, please check out the license file included with the library.

Distributed as-is; no warranty is given.
********************************************************************************/
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <SHA3.h>


/* Define Joystick connection pins */
#define UP     A1
#define DOWN   A3
#define LEFT   A2
#define RIGHT  A5
#define CLICK  A4


void setup() {
  //Initialize analog pins as inputs
  pinMode(UP,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(LEFT,INPUT);
  pinMode(RIGHT,INPUT);
  pinMode(CLICK,INPUT);
  
  //Pull analog pins high to enable reading of joystick movements
  digitalWrite(UP, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, HIGH);
  digitalWrite(CLICK, HIGH);
 
 //Initialize serial terminal connection
  Serial.begin(9600);
  Serial.println("Joystick Demo");

 if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
    Serial.println("CAN Init ok");
  else
    Serial.println("Can't init CAN");
    
  delay(1000);

}



void loop() {
  unsigned long psk = 214789542710544005;
  unsigned char hash;
  
     tCAN message;
  //Scan analog pins. If pin reads low, print the corresponding joystick movement.
   if (digitalRead(UP) == 0) {
     Serial.println("Up");
        message.id = 0x01;
        message.header.rtr = 0;
        message.header.length = 8;
        hash = SHA3_256(psk + 0x01) & 0xff;
        message.data =  hash;
        mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
        mcp2515_send_message(&message);
       
        
     }
      
   if (digitalRead(DOWN) == 0) {
      Serial.println("Down");
        message.id = 0x01;
        message.header.rtr = 0;
        message.header.length = 8;
        message.data[0] = 0x00;
        message.data[1] = 0x00;
        message.data[2] = 0x00;
        message.data[3] = 0x00;
        message.data[4] = 0x00;
        message.data[5] = 0x00;
        message.data[6] = 0x00;
        message.data[7] = 0x00;
        mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
        mcp2515_send_message(&message);
        }
      
   if (digitalRead(LEFT) == 0) {
       Serial.println("Left");}
   
   if (digitalRead(RIGHT) == 0) {
       Serial.println("Right");}

   if (digitalRead(CLICK) == 0) {
       Serial.println("Click");}      
       delay(250);

}
