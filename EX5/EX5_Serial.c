#include "SST89x5x4.h"

void uart_init(void)
{
    SCON = 0x50;
    TMOD = (TMOD & 0x0F) | 0x20;
    PCON = 0x00;
    TH1 = 0xF3;
    TL1 = 0xF3;
    TR1 = 1;
    TI = 1;
}

void uart_putc(unsigned char c)
{
    SBUF = c;
    while (!TI)
    {
    }
    TI = 0;
}

void uart_puts(char *s)
{
    while (*s)
    {
        uart_putc(*s++);
    }
}

void delay(void)
{
    unsigned int i;
    for (i = 0; i < 35000; i++)
    {
    }
}

void main(void)
{
    uart_init();

    while (1)
    {
        uart_puts("Xi'an Tangdu Corp.\r\n");
        delay();
    }
}
