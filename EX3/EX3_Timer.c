#include "SST89x5x4.h"

sbit Wave1 = P1^0;
sbit Wave2 = P1^1;

void main(void)
{
    TMOD = 0x11;      /* T1 and T0: mode 1, 16-bit timer */

    TH0 = 0xF8;
    TL0 = 0x00;
    TH1 = 0xF8;
    TL1 = 0x00;

    TR0 = 1;
    TR1 = 1;

    while (1)
    {
        if (TF0)
        {
            TF0 = 0;
            TH0 = 0xF8;
            TL0 = 0x00;
            Wave1 = ~Wave1;
        }

        if (TF1)
        {
            TF1 = 0;
            TH1 = 0xF8;
            TL1 = 0x00;
            Wave2 = ~Wave2;
        }
    }
}
