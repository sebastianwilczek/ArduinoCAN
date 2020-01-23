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
unsigned long digest1;
unsigned long digest2;

Sha256 sha256;


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
        char hashedValue = hash(0);

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
    }

    //Hack
    if (digitalRead(RIGHT) == 0)
    {
        Serial.println("Right");
    }

    if (digitalRead(CLICK) == 0)
    {
        Serial.println("Click");
    }

    //Monitor Node:
    if (mcp2515_check_message())
    {
        if (mcp2515_get_message(&message))
        {
            if(message.id == 0x01)
            {
                Serial.print("ID: ");
                Serial.print(message.id, HEX);
                Serial.print(", ");
                Serial.print("Data: ");
                Serial.print(message.header.length, DEC);
                for(int i = 0; i < message.header.length; i++)
                {
                    Serial.print(message.data[i], HEX);
                    Serial.print(" ");
                }
                Serial.println("");

                char receivedAuth = hash(message.data[7]) % 256;
                if(message.data[7] == receivedAuth)
                {
                    write("MONITOR: Legitimate brake message.");
                }
                else
                {
                    write("MONITOR: Unauthorized brake message. Potentially compromised ECU. Sending error frame IN THEORY.");
                }
            }
        }
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
    //Serial.print("HASH TEST ");
    //printHash(sha256.result());

    uint8_t *result = sha256.result();
    long hashval = 0;
    for(int i = 0; i < 3; i++)
    {
        hashval = (hashval << 8) + (long)result[i];
    }

    //return atoi((const char *)sha256.result())% 16777216;
    //return hashval;
    return hashval % 256;
}