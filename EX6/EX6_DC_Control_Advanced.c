#include "SST89x5x4.h"

sbit PWM_OUT = P1^7;
sbit ALARM_LED = P1^0;
sbit BUZZER = P1^1;

sbit KEY_START = P3^2;
sbit KEY_STOP  = P3^3;
sbit KEY_UP    = P3^4;
sbit KEY_DOWN  = P3^5;

#define T0_RELOAD_H 0xFC
#define T0_RELOAD_L 0x18
#define PWM_PERIOD 20

unsigned char code seg_cc[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

unsigned char code duty_table[5] = {0, 5, 10, 15, 20};

volatile unsigned char pwm_count = 0;
volatile unsigned int ms_count = 0;
volatile bit status_due = 0;

bit running = 0;
unsigned char speed = 0;
unsigned int alarm_ms = 0;

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

void uart_put_digit(unsigned char v)
{
    uart_putc((unsigned char)('0' + v));
}

void timer0_init(void)
{
    TMOD = (TMOD & 0xF0) | 0x01;
    TH0 = T0_RELOAD_H;
    TL0 = T0_RELOAD_L;
    ET0 = 1;
    TR0 = 1;
}

void timer0_isr(void) interrupt 1
{
    TH0 = T0_RELOAD_H;
    TL0 = T0_RELOAD_L;

    pwm_count++;
    if (pwm_count >= PWM_PERIOD)
    {
        pwm_count = 0;
    }

    if (running && pwm_count < duty_table[speed])
    {
        PWM_OUT = 1;
    }
    else
    {
        PWM_OUT = 0;
    }

    if (alarm_ms > 0)
    {
        alarm_ms--;
        ALARM_LED = 1;
        BUZZER = (alarm_ms & 0x40) ? 1 : 0;
    }
    else
    {
        ALARM_LED = 0;
        BUZZER = 0;
    }

    ms_count++;
    if (ms_count >= 1000)
    {
        ms_count = 0;
        status_due = 1;
    }
}

void display_state(void)
{
    if (running)
    {
        P2 = seg_cc[speed];
    }
    else
    {
        P2 = 0x40;
    }
}

void send_status(void)
{
    uart_puts("RUN=");
    uart_put_digit(running ? 1 : 0);
    uart_puts(", SPEED=");
    uart_put_digit(speed);
    uart_puts("\r\n");
}

void trigger_error(void)
{
    alarm_ms = 2000;
    uart_puts("error\r\n");
}

void speed_up(void)
{
    if (speed < 4)
    {
        speed++;
    }
    else
    {
        trigger_error();
    }
}

void speed_down(void)
{
    if (speed > 0)
    {
        speed--;
    }
    else
    {
        trigger_error();
    }
}

void delay_key(void)
{
    unsigned int i;
    for (i = 0; i < 3000; i++)
    {
    }
}

void handle_keys(void)
{
    if (!KEY_START)
    {
        delay_key();
        if (!KEY_START)
        {
            running = 1;
            send_status();
            while (!KEY_START)
            {
            }
        }
    }

    if (!KEY_STOP)
    {
        delay_key();
        if (!KEY_STOP)
        {
            running = 0;
            send_status();
            while (!KEY_STOP)
            {
            }
        }
    }

    if (!KEY_UP)
    {
        delay_key();
        if (!KEY_UP)
        {
            speed_up();
            send_status();
            while (!KEY_UP)
            {
            }
        }
    }

    if (!KEY_DOWN)
    {
        delay_key();
        if (!KEY_DOWN)
        {
            speed_down();
            send_status();
            while (!KEY_DOWN)
            {
            }
        }
    }
}

void handle_uart(void)
{
    unsigned char c;

    if (RI)
    {
        RI = 0;
        c = SBUF;
        if (c == 's')
        {
            running = 1;
        }
        else if (c == 'p')
        {
            running = 0;
        }
        else if (c == '+')
        {
            speed_up();
        }
        else if (c == '-')
        {
            speed_down();
        }
        else if (c >= '0' && c <= '4')
        {
            speed = c - '0';
        }
        send_status();
    }
}

void main(void)
{
    P1 = 0x00;
    P2 = 0x40;
    P3 |= 0x3C;

    uart_init();
    timer0_init();
    EA = 1;

    send_status();

    while (1)
    {
        handle_keys();
        handle_uart();
        display_state();

        if (status_due)
        {
            status_due = 0;
            send_status();
        }
    }
}
