#include "SST89x5x4.H"

/*
 * Experiment 6 advanced task, stand-alone hardware version.
 *
 * Default clock assumption follows EX5.c:
 *   11.0592 MHz, UART mode 1, Timer1 mode 2, SMOD=1, 19200 bps.
 *
 * Hardware mapping:
 *   P1.7 -> DC motor PWM output
 *   P1.0 -> alarm LED, active high
 *   P1.1 -> buzzer, active high
 *   P2   -> one common-cathode seven-segment display segment bus
 *   P3.2 -> START key, active low
 *   P3.3 -> STOP key, active low
 *   P3.4 -> SPEED UP key, active low
 *   P3.5 -> SPEED DOWN key, active low
 *   P3.1/TXD, P3.0/RXD -> upper computer serial port
 *
 * Upper-computer commands:
 *   s/S: start
 *   p/P: stop
 *   +  : speed up
 *   -  : speed down
 *   0-4: set speed directly
 *   ?  : query status
 */

sbit PWM_OUT = P1^7;
sbit ALARM_LED = P1^0;
sbit BUZZER = P1^1;

sbit KEY_START = P3^2;
sbit KEY_STOP  = P3^3;
sbit KEY_UP    = P3^4;
sbit KEY_DOWN  = P3^5;

/* Timer0 1 ms reload for 11.0592 MHz / 12T: 65536 - 922 = 0xFC66. */
#define T0_RELOAD_H 0xFC
#define T0_RELOAD_L 0x66
#define PWM_PERIOD 20

unsigned char code seg_cc[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

unsigned char code duty_table[5] = {0, 5, 10, 15, 20};

volatile unsigned char pwm_count = 0;
volatile unsigned char key_ms = 0;
volatile unsigned char key_scan_due = 0;
volatile bit status_due = 0;
volatile unsigned int status_ms = 0;

bit running = 0;
unsigned char speed = 0;
unsigned int alarm_ms = 0;

void UART_Init(void)
{
    TMOD &= 0x0F;
    TMOD |= 0x20;

    PCON |= 0x80;
    TH1 = 0xFD;
    TL1 = 0xFD;

    SCON = 0x50;
    TR1 = 1;
    TI = 1;
}

void UART_SendChar(unsigned char dat)
{
    SBUF = dat;
    while (TI == 0)
    {
    }
    TI = 0;
}

void UART_SendString(unsigned char *str)
{
    while (*str != '\0')
    {
        UART_SendChar(*str);
        str++;
    }
}

void UART_SendDigit(unsigned char dat)
{
    UART_SendChar((unsigned char)('0' + dat));
}

void Timer0_Init(void)
{
    TMOD &= 0xF0;
    TMOD |= 0x01;

    TH0 = T0_RELOAD_H;
    TL0 = T0_RELOAD_L;

    ET0 = 1;
    TR0 = 1;
}

void Timer0_ISR(void) interrupt 1
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
        BUZZER = (alarm_ms & 0x0040) ? 1 : 0;
    }
    else
    {
        ALARM_LED = 0;
        BUZZER = 0;
    }

    key_ms++;
    if (key_ms >= 10)
    {
        key_ms = 0;
        key_scan_due = 1;
    }

    status_ms++;
    if (status_ms >= 1000)
    {
        status_ms = 0;
        status_due = 1;
    }
}

void Display_State(void)
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

void Send_Status(void)
{
    UART_SendString("RUN=");
    UART_SendDigit(running ? 1 : 0);
    UART_SendString(", SPEED=");
    UART_SendDigit(speed);
    UART_SendString("\r\n");
}

void Trigger_Error(void)
{
    alarm_ms = 2000;
    UART_SendString("error\r\n");
}

void Speed_Up(void)
{
    if (speed < 4)
    {
        speed++;
    }
    else
    {
        Trigger_Error();
    }
}

void Speed_Down(void)
{
    if (speed > 0)
    {
        speed--;
    }
    else
    {
        Trigger_Error();
    }
}

void Scan_Keys(void)
{
    static unsigned char last_key = 0x3C;
    unsigned char now_key;

    now_key = P3 & 0x3C;

    if ((last_key & 0x04) && !(now_key & 0x04))
    {
        running = 1;
        Send_Status();
    }
    if ((last_key & 0x08) && !(now_key & 0x08))
    {
        running = 0;
        Send_Status();
    }
    if ((last_key & 0x10) && !(now_key & 0x10))
    {
        Speed_Up();
        Send_Status();
    }
    if ((last_key & 0x20) && !(now_key & 0x20))
    {
        Speed_Down();
        Send_Status();
    }

    last_key = now_key;
}

void Check_UART_Command(void)
{
    unsigned char dat;

    if (RI == 1)
    {
        RI = 0;
        dat = SBUF;

        if ((dat == 's') || (dat == 'S'))
        {
            running = 1;
            Send_Status();
        }
        else if ((dat == 'p') || (dat == 'P'))
        {
            running = 0;
            Send_Status();
        }
        else if (dat == '+')
        {
            Speed_Up();
            Send_Status();
        }
        else if (dat == '-')
        {
            Speed_Down();
            Send_Status();
        }
        else if ((dat >= '0') && (dat <= '4'))
        {
            speed = dat - '0';
            Send_Status();
        }
        else if (dat == '?')
        {
            Send_Status();
        }
    }
}

void main(void)
{
    P1 = 0x00;
    P2 = 0x40;
    P3 |= 0x3C;

    UART_Init();
    Timer0_Init();
    EA = 1;

    UART_SendString("DC Motor Control Experiment Start\r\n");
    UART_SendString("Commands: s=start, p=stop, +=up, -=down, 0-4=set, ?=status\r\n");
    Send_Status();

    while (1)
    {
        Check_UART_Command();

        if (key_scan_due)
        {
            key_scan_due = 0;
            Scan_Keys();
        }

        Display_State();

        if (status_due)
        {
            status_due = 0;
            Send_Status();
        }
    }
}
