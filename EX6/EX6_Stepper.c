#include "SST89x5x4.h"

sbit DIR_SW = P3^2;

unsigned char code step_table[8] = {
    0x01, 0x03, 0x02, 0x06,
    0x04, 0x0C, 0x08, 0x09
};

void delay_step(void)
{
    unsigned int i;
    for (i = 0; i < 4000; i++)
    {
    }
}

void main(void)
{
    signed char index = 0;

    P0 = 0x00;
    P3 |= 0x04;

    while (1)
    {
        P0 = step_table[index] & 0x0F;
        delay_step();

        if (DIR_SW)
        {
            index++;
            if (index >= 8)
            {
                index = 0;
            }
        }
        else
        {
            index--;
            if (index < 0)
            {
                index = 7;
            }
        }
    }
}
