#include "SST89x5x4.h"

/*
 * 16x16 dot-matrix display for the new experiment box.
 *
 * Row driver:
 *   P1.0 -> RA0
 *   P1.1 -> RA1
 *   P1.2 -> RA2
 *   P1.3 -> ROE, 0 = upper half, 1 = lower half
 *
 * Column driver:
 *   P0.0 -> CSDI
 *   P0.1 -> CSDK
 *   P0.2 -> CLE
 *   P0.3 -> COE
 *
 * Direction switches, active low:
 *   K1=P3.2, K2=P3.3
 *   K1=1 K2=1: left
 *   K1=0 K2=1: right
 *   K1=1 K2=0: up
 *   K1=0 K2=0: down
 *
 * Display content: Jin Yi Fan.
 */

sbit RA0 = P1^0;
sbit RA1 = P1^1;
sbit RA2 = P1^2;
sbit ROE = P1^3;

sbit CSDI = P0^0;
sbit CSDK = P0^1;
sbit CLE  = P0^2;
sbit COE  = P0^3;

sbit K1 = P3^2;
sbit K2 = P3^3;

#define CHAR_CNT    3
#define SCROLL_RATE 25
#define TOTAL_ROWS  (CHAR_CNT * 16)

/*
 * 16x16 Chinese font data.
 * Each character has 32 bytes, two bytes per row.
 * Byte order follows the reference program: low byte first, high byte second.
 */
unsigned char code hzdot[CHAR_CNT * 32] = {
    /* Jin */
    0x00, 0x00, 0x80, 0x01, 0x80, 0x03, 0xC0, 0x06,
    0x70, 0x0C, 0x38, 0x18, 0xFE, 0x3F, 0xF2, 0x6F,
    0x80, 0x01, 0xF8, 0x1F, 0xF8, 0x1F, 0x98, 0x09,
    0xB0, 0x0D, 0xB0, 0x05, 0xFE, 0x3F, 0x00, 0x00,

    /* Yi */
    0x00, 0x00, 0x80, 0x01, 0xFE, 0x3F, 0xFE, 0x3F,
    0x60, 0x12, 0x68, 0x3A, 0x6C, 0x76, 0xE4, 0x26,
    0xC0, 0x1C, 0x80, 0x09, 0xFE, 0x7F, 0xC0, 0x03,
    0x60, 0x03, 0x38, 0x0E, 0x1E, 0x78, 0x00, 0x20,

    /* Fan */
    0x00, 0x18, 0xF8, 0x18, 0xC8, 0x18, 0x88, 0x7E,
    0xE8, 0x5A, 0xA8, 0x5A, 0xB8, 0x5A, 0xB8, 0x5A,
    0xB8, 0x5A, 0x88, 0x5A, 0x88, 0x5F, 0x8A, 0x5D,
    0x0E, 0x1B, 0x0E, 0x1B, 0x00, 0x00, 0x00, 0x00
};

unsigned char x_off = 0;
unsigned char y_off = 0;
unsigned char char_index = 0;
unsigned int scroll_cnt = 0;

void delay_hold(void)
{
    unsigned char t;
    t = 200;
    while (--t)
    {
    }
}

unsigned int rotate_left16(unsigned int value, unsigned char shift)
{
    while (shift--)
    {
        value = (unsigned int)((value << 1) | ((value & 0x8000) ? 1 : 0));
    }
    return value;
}

unsigned int rotate_right16(unsigned int value, unsigned char shift)
{
    while (shift--)
    {
        value = (unsigned int)((value >> 1) | ((value & 0x0001) ? 0x8000 : 0));
    }
    return value;
}

void send_col_data(unsigned int dat)
{
    unsigned char i;
    unsigned int mask;

    mask = 0x8000;
    for (i = 0; i < 16; i++)
    {
        CSDI = (dat & mask) ? 1 : 0;
        CSDK = 0;
        CSDK = 1;
        mask >>= 1;
    }

    CLE = 0;
    CLE = 1;
}

unsigned int get_row_data(unsigned char screen_row)
{
    unsigned int idx;
    unsigned int dat;
    unsigned char source_row;

    if ((K1 && K2) || (!K1 && K2))
    {
        source_row = (unsigned char)(char_index * 16 + screen_row);
    }
    else
    {
        source_row = screen_row + y_off;
        while (source_row >= TOTAL_ROWS)
        {
            source_row -= TOTAL_ROWS;
        }
    }

    idx = ((unsigned int)source_row) * 2;
    dat = ((unsigned int)hzdot[idx + 1] << 8) | hzdot[idx];

    if (K1 && K2)
    {
        dat = rotate_left16(dat, x_off);
    }
    else if (!K1 && K2)
    {
        dat = rotate_right16(dat, x_off);
    }

    return dat;
}

void select_row(unsigned char row, bit lower_half)
{
    RA0 = row & 0x01;
    RA1 = (row >> 1) & 0x01;
    RA2 = (row >> 2) & 0x01;
    ROE = lower_half ? 1 : 0;
}

void scan_frame(void)
{
    unsigned char row;
    unsigned int dat;

    for (row = 0; row < 8; row++)
    {
        dat = get_row_data(row);
        COE = 1;
        select_row(row, 0);
        send_col_data(dat);
        COE = 0;
        delay_hold();

        dat = get_row_data(row + 8);
        COE = 1;
        select_row(row, 1);
        send_col_data(dat);
        COE = 0;
        delay_hold();
    }
}

void update_offset(void)
{
    scroll_cnt++;
    if (scroll_cnt < SCROLL_RATE)
    {
        return;
    }
    scroll_cnt = 0;

    if (K1 && K2)
    {
        x_off++;
        if (x_off >= 16)
        {
            x_off = 0;
            char_index++;
            if (char_index >= CHAR_CNT)
            {
                char_index = 0;
            }
        }
    }
    else if (!K1 && K2)
    {
        if (x_off == 0)
        {
            x_off = 15;
            if (char_index == 0)
            {
                char_index = CHAR_CNT - 1;
            }
            else
            {
                char_index--;
            }
        }
        else
        {
            x_off--;
        }
    }
    else if (K1 && !K2)
    {
        y_off++;
        if (y_off >= TOTAL_ROWS)
        {
            y_off = 0;
        }
    }
    else
    {
        if (y_off == 0)
        {
            y_off = TOTAL_ROWS - 1;
        }
        else
        {
            y_off--;
        }
    }
}

void main(void)
{
    P0 = 0x00;
    P1 = 0x00;
    P3 |= 0x0C;

    while (1)
    {
        scan_frame();
        update_offset();
    }
}
