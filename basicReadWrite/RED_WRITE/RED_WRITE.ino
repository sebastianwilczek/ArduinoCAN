#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>


/* Define Joystick connection pins */
#define UP     A1
#define DOWN   A3
#define LEFT   A2
#define RIGHT  A5
#define CLICK  A4

int count = -1;

void setup()
{
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

    delay(500);

}



void loop()
{

    tCAN message;
    //Scan analog pins. If pin reads low, print the corresponding joystick movement.

    //Release brake
    if (digitalRead(UP) == 0 || (count >= 0 && count < 500))
    {
        unsigned long start;
        unsigned long elapsed;

        start = micros();
        //Serial.println("Up");
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
        message.data[7] = 0x01;
        mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
        mcp2515_send_message(&message);
        elapsed = micros() - start;
        //Serial.print("Time it took to send in microseconds: ");
        Serial.print("stdcan,send_brake,");
        Serial.println(elapsed);


    }

    //Brake release
    if (digitalRead(DOWN) == 0 || (count >= 500 && count < 1000))
    {
        unsigned long start;
        unsigned long elapsed;

        start = micros();
        //Serial.println("Down");
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
        mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
        mcp2515_send_message(&message);
        elapsed = micros() - start;
        //Serial.print("Time it took to send in microseconds: ");
        Serial.print("stdcan,send_loose,");
        Serial.println(elapsed);
    }

    //Nothing
    if (digitalRead(LEFT) == 0)
    {
        //Serial.println("Left");
        count = 0;
    }

    //Hack
    if (digitalRead(RIGHT) == 0)
    {
        //Serial.println("Right");
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
        message.data[7] = 0x01;
        mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
        mcp2515_send_message(&message);
    }

    if (digitalRead(CLICK) == 0)
    {
        //Serial.println("Click");
        count = -1;
    }

    delay(250);
}