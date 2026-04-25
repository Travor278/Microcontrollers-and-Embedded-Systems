#include "SST89x5x4.h"

sbit SPK = P0^0;

#define TIMER_CLOCK 921600UL

unsigned char data tone_h = 0x00;
unsigned char data tone_l = 0x00;

unsigned int code freq_list[] = {
    371, 495, 495, 495, 624, 556, 495, 556, 624,
    495, 495, 624, 742, 833, 833, 833, 742, 624,
    624, 495, 556, 495, 556, 624, 495, 416, 416,
    371, 495, 0
};

unsigned char code time_list[] = {
    4, 6, 2, 4, 4, 6, 2, 4, 4,
    6, 2, 4, 4, 12, 1, 3, 6, 2,
    4, 4, 6, 2, 4, 4, 6, 2, 4,
    4, 12
};

void timer0_isr(void) interrupt 1
{
    TH0 = tone_h;
    TL0 = tone_l;
    SPK = ~SPK;
}

void delay_unit(unsigned char cnt)
{
    unsigned char i;
    unsigned int j;

    for (i = 0; i < cnt; i++)
    {
        for (j = 0; j < 0x3600; j++)
        {
        }
    }
}

void set_tone(unsigned int freq)
{
    unsigned long counts;
    unsigned int reload;

    if (freq == 0)
    {
        TR0 = 0;
        SPK = 0;
        return;
    }

    counts = TIMER_CLOCK / (2UL * freq);
    reload = (unsigned int)(65536UL - counts);
    tone_h = (unsigned char)(reload >> 8);
    tone_l = (unsigned char)(reload & 0xFF);

    TH0 = tone_h;
    TL0 = tone_l;
    TF0 = 0;
    TR0 = 1;
}

void main(void)
{
    unsigned char i;

    TMOD = 0x01;     /* T0 mode 1 */
    ET0 = 1;
    EA = 1;

    while (1)
    {
        i = 0;
        while (freq_list[i] != 0)
        {
            set_tone(freq_list[i]);
            delay_unit(time_list[i]);
            i++;
        }
    }
}
