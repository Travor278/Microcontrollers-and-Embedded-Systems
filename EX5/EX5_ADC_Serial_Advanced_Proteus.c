#include "SST89x5x4.h"

/*
 * Proteus-friendly version of experiment 5 advanced task.
 *
 * ADC0809:
 *   D0-D7  -> P0.0-P0.7
 *   ALE    -> P3.4
 *   START  -> P3.5
 *   OE     -> P3.6
 *   EOC    -> P3.3
 *   ADDA/ADDB/ADDC -> GND, so channel IN0 is selected.
 *
 * Display:
 *   P2.0-P2.7 -> seven-segment segment bus a,b,c,d,e,f,g,dp.
 *   P1.0-P1.2 -> digit enables for hundreds/tens/ones, active low.
 *
 * Serial:
 *   P3.1/TXD -> Virtual Terminal RXD
 *   P3.0/RXD -> Virtual Terminal TXD
 *
 * Send 0, 1, 2, or 3 from the terminal to change voltage decimals.
 */

sbit ADC_EOC   = P3^3;
sbit ADC_ALE   = P3^4;
sbit ADC_START = P3^5;
sbit ADC_OE    = P3^6;

unsigned char code seg_cc[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

unsigned char data ad_value = 0;
unsigned char data precision = 2;
unsigned char data display_buf[3] = {0, 0, 0};

void delay_short(void)
{
    unsigned int i;
    for (i = 0; i < 500; i++)
    {
    }
}

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

void uart_put_uint(unsigned int v)
{
    unsigned char hundreds = 0;
    unsigned char tens = 0;

    while (v >= 100)
    {
        v -= 100;
        hundreds++;
    }
    while (v >= 10)
    {
        v -= 10;
        tens++;
    }

    if (hundreds)
    {
        uart_putc((unsigned char)('0' + hundreds));
        uart_putc((unsigned char)('0' + tens));
    }
    else if (tens)
    {
        uart_putc((unsigned char)('0' + tens));
    }
    uart_putc((unsigned char)('0' + v));
}

void uart_put_voltage(unsigned char value)
{
    unsigned long mv;
    unsigned int v_int;
    unsigned int v_frac;

    mv = ((unsigned long)value * 5000UL) / 255UL;
    v_int = (unsigned int)(mv / 1000UL);
    v_frac = (unsigned int)(mv % 1000UL);

    uart_put_uint(v_int);
    if (precision > 0)
    {
        uart_putc('.');
        uart_putc((unsigned char)('0' + (v_frac / 100)));
        if (precision > 1)
        {
            uart_putc((unsigned char)('0' + ((v_frac / 10) % 10)));
        }
        if (precision > 2)
        {
            uart_putc((unsigned char)('0' + (v_frac % 10)));
        }
    }
    uart_putc('V');
}

unsigned char read_adc0809(void)
{
    unsigned char value;

    P0 = 0xFF;
    ADC_OE = 0;

    ADC_ALE = 1;
    ADC_START = 1;
    delay_short();
    ADC_ALE = 0;
    ADC_START = 0;

    while (!ADC_EOC)
    {
    }

    ADC_OE = 1;
    delay_short();
    value = P0;
    ADC_OE = 0;

    return value;
}

void update_display_buf(unsigned char value)
{
    display_buf[0] = value / 100;
    display_buf[1] = (value / 10) % 10;
    display_buf[2] = value % 10;
}

void display_scan_once(void)
{
    P1 = 0xFE;
    P2 = seg_cc[display_buf[0]];
    delay_short();
    P1 = 0xFD;
    P2 = seg_cc[display_buf[1]];
    delay_short();
    P1 = 0xFB;
    P2 = seg_cc[display_buf[2]];
    delay_short();
    P1 = 0xFF;
}

void handle_uart_command(void)
{
    unsigned char c;

    if (RI)
    {
        RI = 0;
        c = SBUF;
        if (c >= '0' && c <= '3')
        {
            precision = c - '0';
            uart_puts("precision=");
            uart_put_uint(precision);
            uart_puts("\r\n");
        }
    }
}

void send_sample(void)
{
    uart_puts("ADC=");
    uart_put_uint(ad_value);
    uart_puts(", U=");
    uart_put_voltage(ad_value);
    uart_puts("\r\n");
}

void main(void)
{
    unsigned int loop_count = 0;

    uart_init();
    P0 = 0xFF;
    P1 = 0xFF;
    P2 = 0x00;
    P3 |= 0x08;
    ADC_ALE = 0;
    ADC_START = 0;
    ADC_OE = 0;

    while (1)
    {
        handle_uart_command();
        ad_value = read_adc0809();
        update_display_buf(ad_value);
        display_scan_once();

        loop_count++;
        if (loop_count >= 80)
        {
            loop_count = 0;
            send_sample();
        }
    }
}
