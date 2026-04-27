#include "SST89x5x4.h"
#include "ABSACC.H"

void main(void)
{
    unsigned char i;

    AUXR = 0x02;

    for (i = 0; i < 16; i++)
    {
        DBYTE[0x30 + i] = i;
    }

    for (i = 0; i < 16; i++)
    {
        XBYTE[0x0000 + i] = DBYTE[0x30 + i];
    }

    for (i = 0; i < 16; i++)
    {
        DBYTE[0x40 + i] = XBYTE[0x0000 + i];
    }

    while (1)
    {
    }
}
