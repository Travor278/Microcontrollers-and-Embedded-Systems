#include "SST89x5x4.h"
#include "ABSACC.H"

#define STARTAD  XBYTE[0x7F00]
#define ADRESULT XBYTE[0x7F08]

sbit ADBUSY = P3^3;

unsigned char data ad_value = 0;

void delay_short(void)
{
    unsigned char i;
    for (i = 0; i < 100; i++)
    {
    }
}

unsigned char read_adc0809(void)
{
    unsigned char result;

    STARTAD = 0x00;
    while (ADBUSY)
    {
    }
    delay_short();
    result = ADRESULT;
    return result;
}

void main(void)
{
    while (1)
    {
        ad_value = read_adc0809();
        P1 = ad_value;
        delay_short();
    }
}
