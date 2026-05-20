#include "SST89x5x4.H"
#include "Absacc.h"

/* =====================================================
   A/D ?? + ??????

   AD0809:
   STARTAD  = XBYTE[0x7F00]  ?? A/D ??
   ADRESULT = XBYTE[0x7F08]  ?? A/D ????
   ADBUSY   = P3.3           ?????

   ??:
   ? UART #1 ???? ADV ????

   ??:
   ??????
   ?????? 1 ???
   ===================================================== */

#define STARTAD   XBYTE[0x7F00]
#define ADRESULT  XBYTE[0x7F08]

sbit ADBUSY = P3^3;

/* ?? 1 ??? */
unsigned char voltage_precision = 1;


/* =====================================================
   ????
   ????,??????
   ===================================================== */
void Delay(unsigned int t)
{
    unsigned int i;
    unsigned int j;

    for(i = 0; i < t; i++)
    {
        for(j = 0; j < 120; j++)
        {
            ;
        }
    }
}


/* =====================================================
   ?????
   11.0592MHz ??,19200bps
   ????????? 9600,??????? TH1
   ===================================================== */
void UART_Init(void)
{
    TMOD &= 0x0F;
    TMOD |= 0x20;      /* ???1,??2,???? */

    PCON |= 0x80;      /* SMOD=1,????? */

    TH1 = 0xFD;        /* 19200bps,11.0592MHz */
    TL1 = 0xFD;

    SCON = 0x50;       /* ????1,???? */

    TR1 = 1;           /* ?????1 */
    TI = 1;
}


/* =====================================================
   ????????
   ===================================================== */
void UART_SendChar(unsigned char dat)
{
    SBUF = dat;
    while(TI == 0);
    TI = 0;
}


/* =====================================================
   ???????
   ===================================================== */
void UART_SendString(unsigned char *str)
{
    while(*str != '\0')
    {
        UART_SendChar(*str);
        str++;
    }
}


/* =====================================================
   ???????
   ===================================================== */
void UART_SendNumber(unsigned int num)
{
    unsigned char buf[5];
    unsigned char i;

    if(num == 0)
    {
        UART_SendChar('0');
        return;
    }

    i = 0;

    while(num > 0)
    {
        buf[i] = num % 10;
        num = num / 10;
        i++;
    }

    while(i > 0)
    {
        i--;
        UART_SendChar(buf[i] + '0');
    }
}


/* =====================================================
   ?? ADC0809
   ===================================================== */
unsigned char AD0809_Read(void)
{
    unsigned char result;
    unsigned int timeout;

    STARTAD = 0x00;       /* ?? A/D ?? */

    timeout = 50000;

    while((ADBUSY == 1) && (timeout > 0))
    {
        timeout--;
    }

    Delay(1);

    result = ADRESULT;    /* ?????? */

    return result;
}


/* =====================================================
   ?????

   ADC0809 ? 8 ?:
   ADV = 0   ?? 0V
   ADV = 255 ?? 5V

   ?? mV = ADV * 5000 / 255
   ===================================================== */
void SendVoltage(unsigned char adv)
{
    unsigned long mv;
    unsigned int integer_part;
    unsigned int decimal_part;

    mv = ((unsigned long)adv * 5000UL) / 255UL;

    integer_part = (unsigned int)(mv / 1000UL);

    UART_SendString("ADV=");
    UART_SendNumber(adv);

    UART_SendString("    Voltage=");
    UART_SendNumber(integer_part);
    UART_SendChar('.');

    if(voltage_precision == 0)
    {
        UART_SendChar('0');
    }
    else if(voltage_precision == 1)
    {
        decimal_part = (unsigned int)((mv % 1000UL) / 100UL);
        UART_SendNumber(decimal_part);
    }
    else if(voltage_precision == 2)
    {
        decimal_part = (unsigned int)((mv % 1000UL) / 10UL);

        if(decimal_part < 10)
        {
            UART_SendChar('0');
        }

        UART_SendNumber(decimal_part);
    }
    else
    {
        decimal_part = (unsigned int)(mv % 1000UL);

        if(decimal_part < 100)
        {
            UART_SendChar('0');
        }

        if(decimal_part < 10)
        {
            UART_SendChar('0');
        }

        UART_SendNumber(decimal_part);
    }

    UART_SendString(" V\r\n");
}


/* =====================================================
   ??????
   0:?? 0 ???
   1:?? 1 ???
   2:?? 2 ???
   3:?? 3 ???

   ???????,????? 1 ?????
   ===================================================== */
void Check_UART_Command(void)
{
    unsigned char dat;

    if(RI == 1)
    {
        RI = 0;
        dat = SBUF;

        if(dat == '0')
        {
            voltage_precision = 0;
            UART_SendString("Set precision: 0 decimal\r\n");
        }
        else if(dat == '1')
        {
            voltage_precision = 1;
            UART_SendString("Set precision: 1 decimal\r\n");
        }
        else if(dat == '2')
        {
            voltage_precision = 2;
            UART_SendString("Set precision: 2 decimals\r\n");
        }
        else if(dat == '3')
        {
            voltage_precision = 3;
            UART_SendString("Set precision: 3 decimals\r\n");
        }
    }
}


/* =====================================================
   ???
   ===================================================== */
void main(void)
{
    unsigned char adv;

    UART_Init();

    UART_SendString("A/D Conversion Experiment Start\r\n");
    UART_SendString("Default: 1 decimal output\r\n");
    UART_SendString("Input 0/1/2/3 to change voltage precision.\r\n\r\n");

    while(1)
    {
        Check_UART_Command();

        adv = AD0809_Read();

        SendVoltage(adv);

        /*
           ???????
           ?? UART #1 ????,?? 300 ??,?? 600?1000?
           ????,???,?? 100?200?
        */
        Delay(300);
    }
}
