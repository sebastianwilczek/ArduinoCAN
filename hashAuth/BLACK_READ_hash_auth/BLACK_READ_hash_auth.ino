
// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PINP = 9;
const int LED        = 8;
boolean ledON        = 1;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

// Hash Based Authentication
int groupId = 1;
int counter = 0;
int keyCounter = 0;
unsigned long presharedKey = 84364362;
unsigned long generatedKey;
unsigned char values[8];

void setup()
{
    SERIAL.begin(115200);
    pinMode(LED, OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println("Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");

    SERIAL.println("Setting up Hash Authentication.");
    generatedKey = generateKey(presharedKey, 0);
    SERIAL.print("First generated key: ");
    SERIAL.println(generatedKey, HEX);

    delay(1000);
}


void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = CAN.getCanId();

        SERIAL.println("-----------------------------");
        SERIAL.print("get data from ID: 0x");
        SERIAL.println(canId, HEX);
        SERIAL.println(buf[7], HEX);

        if(canId == 0x01)
        {
            SERIAL.print("Calculated hash: ");
            SERIAL.println(hash(buf[7]), HEX);
            SERIAL.print("Received hash: ");
            SERIAL.println(buf[4] + (buf[5] << 8) + (buf[6] << 16), HEX);

            unsigned long hashedValue = hash(buf[7]);
            values[0] = hashedValue;
            values[1] = hashedValue >> 8;
            values[2] = hashedValue >> 16;

            if(values[0] != buf[4] || values[1] != buf[5] || values[2] != buf[6])
            {
                SERIAL.println("Hash Unmatch. Potential compromised ECU.");
            }
            else
            {
                counter++;
                SERIAL.println("Received vaild brake message.");
                if(buf[7] == 0x01)
                {
                    SERIAL.println("Brakes engaged.");
                }
                else
                {
                    SERIAL.println("Brakes loosened.");
                }
            }
        }
        if(canId == 0x02)
        {
            if (buf[7] == keyCounter + 1)
            {
                unsigned long randomValue = buf[5] + (buf[6] << 8);

                Serial.print("Received Random Value: ");
                Serial.println(randomValue, HEX);

                unsigned long keyRenewHashRes = keyRenewHash(generatedKey, randomValue, keyCounter + 1);
                //unsigned long keyRenewHashRes = buf[2] + (buf[3] << 8) + (buf[4] << 16);
                values[2] = keyRenewHashRes;
                values[3] = keyRenewHashRes >> 8;
                values[4] = keyRenewHashRes >> 16;

                Serial.print("Received Key Hash: ");
                Serial.println(keyRenewHashRes, HEX);

                if(values[2] == buf[2] && values[3] == buf[3] && values[4] == buf[4])
                {
                    keyCounter++;
                    generatedKey = generateKey(generatedKey, randomValue);
                    SERIAL.print("New generated key is ");
                    SERIAL.println(generatedKey, HEX);
                }
                else
                {
                    SERIAL.println("Key Renewal Hash mismatch, ignoring key renewal");
                }
            }
            else
            {
                SERIAL.println("Counter out of sync, ignoring key renewal");
            }
        }
    }
}

unsigned long hash(long data)
{
    return (groupId + generatedKey + counter + data) * 5325 % 16777216;
}

unsigned long generateKey(long previousKey, long randomValue)
{
    return (previousKey + randomValue + keyCounter) % 100000000;
}

unsigned long keyRenewHash(long genKey, long rValue, int c)
{
    return (genKey + rValue + c) * 5325 % 16777216;
}

//END FILE
