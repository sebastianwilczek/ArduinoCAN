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
const int SPI_CS_PIN = 9;
const int LED        = 8;
boolean ledON        = 1;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin
bool brakesEngaged = false;

void setup()
{
    SERIAL.begin(115200);
    pinMode(LED,OUTPUT);

    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
    {
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println("Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
    delay(500);
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

        if(canId == 0x01 and buf[7]==0x01){
         if(brakesEngaged)
            {
              SERIAL.println("Brakes are already engaged.");
            }
            else
             {
              SERIAL.println("Brakes engaged.");
              brakesEngaged = true;
              digitalWrite(LED, HIGH);
             }
        }
        if(canId == 0x01 and buf[7] ==0){
          if(brakesEngaged)
            {
              SERIAL.println("Brakes loosened.");
              brakesEngaged = false;
              digitalWrite(LED, LOW);
            }
            else
            {
              SERIAL.println("Brakes are already loosened.");
            }
        }
        }
        
    }


//END FILE
