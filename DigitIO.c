#include "SST89x5x4.H"

void main(void)
{
    unsigned char data i;
    while(1)
    {
        P1 = P1 | 0xF0;      // ?4???
        i = P1;
        P1 = (i >> 4) & 0x0F;
    }
}
