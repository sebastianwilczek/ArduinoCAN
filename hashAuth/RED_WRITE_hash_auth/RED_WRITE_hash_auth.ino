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

// Hash Based Authentication
int groupId = 1;
int counter = 0;
int keyCounter = 0;
unsigned long presharedKey = 84364362;
unsigned long generatedKey;
unsigned long randomValue;
unsigned char values[8];

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

 if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
    Serial.println("CAN Init ok");
  else
    Serial.println("Can't init CAN");

  Serial.println("Setting up Hash Authentication.");
  generatedKey = generateKey(presharedKey, 0);
  Serial.print("First generated key: ");
  Serial.println(generatedKey, HEX);
    
  delay(1000);
}

void loop() {
  tCAN message;
  //Scan analog pins. If pin reads low, print the corresponding joystick movement.
  if (digitalRead(UP) == 0) {
    Serial.println("Up");

    //Hash Auth
    unsigned long hashedValue = hash(1);
    values[0] = hashedValue;
    values[1] = hashedValue >> 8;
    values[2] = hashedValue >> 16;

    Serial.print("Computed Hash: ");
    Serial.println(hashedValue, HEX);

    message.id = 0x01;
    message.header.rtr = 0;
    message.header.length = 8;
    message.data[0] = 0x00;
    message.data[1] = 0x00;
    message.data[2] = 0x00;
    message.data[3] = 0x00;
    message.data[4] = values[0];
    message.data[5] = values[1];
    message.data[6] = values[2];
    message.data[7] = 0x01;
    mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
    mcp2515_send_message(&message);
    counter++;
  }
    
  if (digitalRead(DOWN) == 0) {
    Serial.println("Down");

    //Hash Auth
    unsigned long hashedValue = hash(0);
    values[0] = hashedValue;
    values[1] = hashedValue >> 8;
    values[2] = hashedValue >> 16;

    Serial.print("Computed Hash: ");
    Serial.println(hashedValue, HEX);

    message.id = 0x01;
    message.header.rtr = 0;
    message.header.length = 8;
    message.data[0] = 0x00;
    message.data[1] = 0x00;
    message.data[2] = 0x00;
    message.data[3] = 0x00;
    message.data[4] = values[0];
    message.data[5] = values[1];
    message.data[6] = values[2];
    message.data[7] = 0x00;
    mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
    mcp2515_send_message(&message);
    counter++;
  }
    
  if (digitalRead(LEFT) == 0) {
    Serial.println("Left");

    Serial.println("Starting key renewal");
    renewKey();
  }
  
  if (digitalRead(RIGHT) == 0) {
    Serial.println("Right");

    //Hacked Hash Auth
    unsigned long hashedValue = rand() % 16777216;
    values[0] = hashedValue;
    values[1] = hashedValue >> 8;
    values[2] = hashedValue >> 16;

    Serial.print("Random Hash: ");
    Serial.println(hashedValue, HEX);

    message.id = 0x01;
    message.header.rtr = 0;
    message.header.length = 8;
    message.data[0] = 0x00;
    message.data[1] = 0x00;
    message.data[2] = 0x00;
    message.data[3] = 0x00;
    message.data[4] = values[0];
    message.data[5] = values[1];
    message.data[6] = values[2];
    message.data[7] = 0x01;
    mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
    mcp2515_send_message(&message);

  }

  if (digitalRead(CLICK) == 0) {
    Serial.println("Click");
  }      
  
  delay(250);
}

unsigned long hash(long data)
{
  return (groupId + generatedKey + counter + data) * 5325 % 16777216;
}

unsigned long generateKey(long previousKey, long randomValue)
{
  return (previousKey + randomValue + keyCounter) % 100000000;
}

void renewKey(){
  tCAN message;
  keyCounter++;
  randomValue = rand() % 65536;
  values[0] = randomValue;
  values[1] = randomValue >> 8;

  Serial.print("Random Value: ");
  Serial.println(randomValue, HEX);

  unsigned long keyRenewHashRes = keyRenewHash(generatedKey, randomValue, keyCounter);
  values[2] = keyRenewHashRes;
  values[3] = keyRenewHashRes >> 8;
  values[4] = keyRenewHashRes >> 16;

  Serial.print("Computed Key Hash: ");
  Serial.println(keyRenewHashRes, HEX);

  message.id = 0x02;
  message.header.rtr = 0;
  message.header.length = 8;
  message.data[0] = 0x00;
  message.data[1] = 0x00;
  message.data[2] = values[2];
  message.data[3] = values[3];
  message.data[4] = values[4];
  message.data[5] = values[0];
  message.data[6] = values[1];
  message.data[7] = keyCounter;
  mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
  mcp2515_send_message(&message);
  generatedKey = generateKey(generatedKey, randomValue);
  Serial.print("New generated key is ");
  Serial.println(generatedKey, HEX);
}

unsigned long keyRenewHash(long genKey, long rValue, int c)
{
  return (genKey + rValue + c) * 5325 % 16777216;
}
