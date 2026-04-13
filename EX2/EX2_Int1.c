#include "SST89x5x4.h"

sbit Wave1 = P1^0;
sbit Wave2 = P1^1;

void int_timer0(void) interrupt 1
{
    Wave1 = ~Wave1;
    TH0 = 0xF8;
    TL0 = 0x00;
}

void int_timer1(void) interrupt 3
{
    Wave2 = ~Wave2;
    TH1 = 0xF8;
    TL1 = 0x00;
}

void main(void)
{
    TH0 = 0xF8;
    TL0 = 0x00;
    TH1 = 0xF8;
    TL1 = 0x00;

    TMOD = 0x11;
    TCON = 0x50;
    IE = 0x8A;

    while (1)
    {
    }
}
