#include "SST89x5x4.h"

sbit PWM_OUT = P1^7;

#define T_HIGH  60
#define T_LOW   40

void delay_unit(unsigned char t)
{
    unsigned char i;
    unsigned int j;

    for (i = 0; i < t; i++)
    {
        for (j = 0; j < 150; j++)
        {
        }
    }
}

void main(void)
{
    while (1)
    {
        PWM_OUT = 1;
        delay_unit(T_HIGH);
        PWM_OUT = 0;
        delay_unit(T_LOW);
    }
}
