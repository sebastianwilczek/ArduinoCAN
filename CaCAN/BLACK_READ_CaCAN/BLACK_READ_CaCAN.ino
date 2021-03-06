
// demo: CAN-BUS Shield, receive data with check mode
// send data coming to fast, such as less than 10ms, you can use this way
// loovee, 2014-6-13

#include <SPI.h>
#include "mcp_can.h"
#include "sha256.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#else
#define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
const int LED        = 8;
boolean ledON        = 1;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

unsigned long sharedSecret = 84364362;
unsigned long randomNonce = 4294967295;
bool brakesEngaged = false;

Sha256 sha256;

void setup()
{
    uint8_t *hashs;
    uint32_t a;
    SERIAL.begin(115200);

    pinMode(LED, OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        //SERIAL.println("CAN BUS Shield init fail");
        //SERIAL.println("Init CAN BUS Shield again");
        delay(100);
    }
    //SERIAL.println("CAN BUS Shield init ok!");

    delay(1000);
}


void loop()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        //start timer
        unsigned long start;
        unsigned long elapsed;
        start = micros();
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        unsigned long canId = CAN.getCanId();
        if(canId == 0x01)
        {
            unsigned char receivedAuth = hash(buf[7]) % 256;
            if(buf[6] == receivedAuth)
            {
                //Serial.println("MONITOR: Legitimate brake message.");
                if(buf[7] == 0x01)
                {
                    if(brakesEngaged)
                    {
                        //SERIAL.println("Brakes are already engaged.");
                        elapsed = micros() - start;
                        //Serial.print("Time it took to proces in microseconds: ");
                        Serial.print("cacan,process_brake,");
                        Serial.println(elapsed);
                    }
                    else
                    {
                        //SERIAL.println("Brakes engaged.");
                        brakesEngaged = true;
                        digitalWrite(LED, HIGH);
                        elapsed = micros() - start;
                        //Serial.print("Time it took to proces in microseconds: ");
                        Serial.print("cacan,process_brake,");
                        Serial.println(elapsed);
                    }
                }
                else
                {
                    if(brakesEngaged)
                    {

                        //SERIAL.println("Brakes loosened.");
                        brakesEngaged = false;
                        digitalWrite(LED, LOW);
                        elapsed = micros() - start;
                        //Serial.print("Time it took to send in microseconds: ");
                        Serial.print("cacan,process_loose,");
                        Serial.println(elapsed);
                    }
                    else
                    {
                        //SERIAL.println("Brakes are already loosened.");
                        elapsed = micros() - start;
                        //Serial.print("Time it took to proces in microseconds: ");
                        Serial.print("cacan,process_loose,");
                        Serial.println(elapsed);
                    }
                }
            }
            else
            {
                //Serial.println("MONITOR: Unauthorized brake message. Potentially compromised ECU. Sending error frame IN THEORY.");
                elapsed = micros() - start;
                //Serial.print("Time it took to proces in microseconds: ");
                Serial.print("cacan,process_unauth,");
                Serial.println(elapsed);
            }
        }
    }
}

unsigned char hash(long data)
{
    unsigned long key = randomNonce + sharedSecret;
    uint8_t *keyArray = (uint8_t *)(&key);
    //Serial.println(key);
    sha256.initHmac(keyArray, 4);
    sha256.print(data);
    uint8_t *result = sha256.result();
    return result[0] % 256;
}