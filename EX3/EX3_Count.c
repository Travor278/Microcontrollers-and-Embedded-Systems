#include "SST89x5x4.h"

sbit P10Value = P1^0;

void main(void)
{
    /*
     * T1 mode 2 counter:
     * C/T1 = 1, mode = 2, reload = 0xF6.
     * 0x100 - 0xF6 = 10, so P1.0 toggles every 10 input pulses.
     */
    TMOD = 0x60;
    TH1 = 0xF6;
    TL1 = 0xF6;
    TR1 = 1;

    while (1)
    {
        while (!TF1)
        {
        }
        TF1 = 0;
        P10Value = ~P10Value;
    }
}
