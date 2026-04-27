#include "SST89x5x4.h"
#include "ABSACC.H"

unsigned char code init_data[10] = {
    0x09, 0x11, 0x05, 0x31, 0x20,
    0x16, 0x01, 0x1A, 0x3F, 0x08
};

void init_internal_ram(void)
{
    unsigned char i;
    for (i = 0; i < 10; i++)
    {
        DBYTE[0x30 + i] = init_data[i];
    }
}

void sort_internal_30h(void)
{
    unsigned char i;
    unsigned char j;
    unsigned char temp;

    for (i = 0; i < 9; i++)
    {
        for (j = i + 1; j < 10; j++)
        {
            if (DBYTE[0x30 + i] > DBYTE[0x30 + j])
            {
                temp = DBYTE[0x30 + i];
                DBYTE[0x30 + i] = DBYTE[0x30 + j];
                DBYTE[0x30 + j] = temp;
            }
        }
    }
}

void main(void)
{
    init_internal_ram();
    sort_internal_30h();

    while (1)
    {
    }
}
