#include "SST89x5x4.h"
#include "ABSACC.H"

unsigned char code internal_init[16] = {
    0x2A, 0x04, 0x19, 0x33, 0x0C, 0x41, 0x08, 0x1F,
    0x55, 0x10, 0x3B, 0x01, 0x27, 0x0A, 0x60, 0x16
};

unsigned char code external_init[16] = {
    0x31, 0x22, 0x05, 0x7A, 0x18, 0x42, 0x0E, 0x63,
    0x12, 0x2D, 0x09, 0x51, 0x24, 0x03, 0x6B, 0x1C
};

void init_ram_area(void)
{
    unsigned char i;

    AUXR = 0x02;
    for (i = 0; i < 16; i++)
    {
        DBYTE[0x40 + i] = internal_init[i];
        XBYTE[0x0000 + i] = external_init[i];
    }
}

void sort_internal_40h_4fh(void)
{
    unsigned char i;
    unsigned char j;
    unsigned char temp;

    for (i = 0; i < 15; i++)
    {
        for (j = i + 1; j < 16; j++)
        {
            if (DBYTE[0x40 + i] > DBYTE[0x40 + j])
            {
                temp = DBYTE[0x40 + i];
                DBYTE[0x40 + i] = DBYTE[0x40 + j];
                DBYTE[0x40 + j] = temp;
            }
        }
    }
}

void sort_external_0000h_000fh(void)
{
    unsigned char i;
    unsigned char j;
    unsigned char temp;

    for (i = 0; i < 15; i++)
    {
        for (j = i + 1; j < 16; j++)
        {
            if (XBYTE[0x0000 + i] > XBYTE[0x0000 + j])
            {
                temp = XBYTE[0x0000 + i];
                XBYTE[0x0000 + i] = XBYTE[0x0000 + j];
                XBYTE[0x0000 + j] = temp;
            }
        }
    }
}

void main(void)
{
    init_ram_area();
    sort_internal_40h_4fh();
    sort_external_0000h_000fh();

    while (1)
    {
    }
}
