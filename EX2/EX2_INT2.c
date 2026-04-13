#include "SST89x5x4.h"
#include "intrins.h"

void delay(void)
{
    unsigned int x;
    for (x = 0; x < 0xFFFF; x++)
    {
    }
}

void int0_isr(void) interrupt 0
{
    unsigned char j;
    for (j = 0; j < 4; j++)
    {
        P1 = 0xFF;
        delay();
        P1 = 0x00;
        delay();
    }
}

void int1_isr(void) interrupt 2
{
    unsigned char i = 0x03, j;
    for (j = 0; j < 16; j++)
    {
        P1 = i;
        i = _crol_(i, 2);
        delay();
    }
    P1 = 0x00;
}

void main(void)
{
    P1 = 0x00;

    IT0 = 1;
    EX0 = 1;
    IT1 = 1;
    EX1 = 1;
    EA = 1;

    while (1)
    {
    }
}
