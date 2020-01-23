#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include "sha256.h"


/* Define Joystick connection pins */
#define UP     A1
#define DOWN   A3
#define LEFT   A2
#define RIGHT  A5
#define CLICK  A4

//CaCAN
unsigned long sharedSecret = 84364362;
unsigned long randomNonce = 4294967295;

Sha256 sha256;

bool hacking = false;

void setup()
{
    uint8_t *hashs;
    uint32_t a;

    //Initialize analog pins as inputs
    pinMode(UP, INPUT);
    pinMode(DOWN, INPUT);
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(CLICK, INPUT);

    //Pull analog pins high to enable reading of joystick movements
    digitalWrite(UP, HIGH);
    digitalWrite(DOWN, HIGH);
    digitalWrite(LEFT, HIGH);
    digitalWrite(RIGHT, HIGH);
    digitalWrite(CLICK, HIGH);

    //Initialize serial terminal connection
    Serial.begin(115200);

    if(Canbus.init(CANSPEED_500))  //Initialise MCP2515 CAN controller at the specified speed
        Serial.println("CAN Init ok");
    else
        Serial.println("Can't init CAN");

    delay(1000);
}

void loop()
{
    tCAN message;
    //Scan analog pins. If pin reads low, print the corresponding joystick movement.

    if (digitalRead(UP) == 0)
    {
        //Applying brake pressure
        Serial.println("Up");

        //CaCAN
        char hashedValue = hash(1);

        Serial.print("Computed Hash: ");
        Serial.println(hashedValue, HEX);

        message.id = 0x01;
        message.header.rtr = 0;
        message.header.length = 8;
        message.data[0] = 0x00;
        message.data[1] = 0x00;
        message.data[2] = 0x00;
        message.data[3] = 0x00;
        message.data[4] = 0x00;
        message.data[5] = 0x00;
        message.data[6] = hashedValue;
        message.data[7] = 0x01;
        mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
        mcp2515_send_message(&message);
    }

    //Brake
    if (digitalRead(DOWN) == 0)
    {
        //Releasing brake pressure
        Serial.println("Down");

        //CaCAN
        unsigned char hashedValue = hash(0);

        Serial.print("Computed Hash: ");
        Serial.println(hashedValue, HEX);

        message.id = 0x01;
        message.header.rtr = 0;
        message.header.length = 8;
        message.data[0] = 0x00;
        message.data[1] = 0x00;
        message.data[2] = 0x00;
        message.data[3] = 0x00;
        message.data[4] = 0x00;
        message.data[5] = 0x00;
        message.data[6] = hashedValue;
        message.data[7] = 0x00;
        mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
        mcp2515_send_message(&message);
    }

    //New Key
    if (digitalRead(LEFT) == 0)
    {
        Serial.println("Left");

        //Hacker On
        Serial.print("Started flooding malicious messages.");
        hacking = true;
    }

    //Hack
    if (digitalRead(RIGHT) == 0)
    {
        Serial.println("Right");

        //Hacker Off
        Serial.print("Stopped flooding malicious messages.");
        hacking = false;
    }

    if (digitalRead(CLICK) == 0)
    {
        Serial.println("Click");
    }

    if(hacking){
        unsigned char hashedValue = rand() % 256;

        Serial.print("Random Hash: ");
        Serial.println(hashedValue, HEX);

        message.id = 0x01;
        message.header.rtr = 0;
        message.header.length = 8;
        message.data[0] = 0x00;
        message.data[1] = 0x00;
        message.data[2] = 0x00;
        message.data[3] = 0x00;
        message.data[4] = 0x00;
        message.data[5] = 0x00;
        message.data[6] = hashedValue;
        message.data[7] = 0x01;
        mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
        mcp2515_send_message(&message);
    }

    delay(250);
}

unsigned char hash(long data)
{
    unsigned long key = randomNonce + sharedSecret;
    uint8_t *keyArray = (uint8_t *)(&key);
    Serial.println(key);
    sha256.initHmac(keyArray, 4);
    sha256.print(data);
    uint8_t *result = sha256.result();
    return result[0] % 256;
}