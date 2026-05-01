#include "SST89x5x4.h"
#include "intrins.h"

/*
 * 16x16 dot-matrix template for the new experiment box.
 * The concrete driver pins depend on the SM16206/SM5166 wiring
 * on the laboratory hardware. Use the pin names below as a mapping layer,
 * Name displayed: Jin Yi Fan.
 * P3.2 controls the cyclic display direction.
 */

sbit DIR_SW = P3^2;
sbit ROW_A = P1^0;
sbit ROW_B = P1^1;
sbit ROW_C = P1^2;
sbit ROW_D = P1^3;
sbit COL_DATA = P1^4;
sbit COL_CLK = P1^5;
sbit COL_LAT = P1^6;
sbit COL_OE = P1^7;

unsigned char code name_font[][32] = {
    {
        0x00, 0x00, 0x01, 0x80, 0x03, 0x80, 0x06, 0xC0,
        0x0C, 0x70, 0x18, 0x38, 0x3F, 0xFE, 0x6F, 0xF2,
        0x01, 0x80, 0x1F, 0xF8, 0x1F, 0xF8, 0x09, 0x98,
        0x0D, 0xB0, 0x05, 0xB0, 0x3F, 0xFE, 0x00, 0x00
    },
    {
        0x00, 0x00, 0x01, 0x80, 0x3F, 0xFE, 0x3F, 0xFE,
        0x12, 0x60, 0x3A, 0x68, 0x76, 0x6C, 0x26, 0xE4,
        0x1C, 0xC0, 0x09, 0x80, 0x7F, 0xFE, 0x03, 0xC0,
        0x03, 0x60, 0x0E, 0x38, 0x78, 0x1E, 0x20, 0x00
    },
    {
        0x18, 0x00, 0x18, 0xF8, 0x18, 0xC8, 0x7E, 0x88,
        0x5A, 0xE8, 0x5A, 0xA8, 0x5A, 0xB8, 0x5A, 0xB8,
        0x5A, 0xB8, 0x5A, 0x88, 0x5F, 0x88, 0x5D, 0x8A,
        0x1B, 0x0E, 0x1B, 0x0E, 0x00, 0x00, 0x00, 0x00
    }
};

void delay_short(void)
{
    unsigned char i;
    for (i = 0; i < 20; i++)
    {
        _nop_();
    }
}

void select_row(unsigned char row)
{
    ROW_A = row & 0x01;
    ROW_B = (row >> 1) & 0x01;
    ROW_C = (row >> 2) & 0x01;
    ROW_D = (row >> 3) & 0x01;
}

void shift_out16(unsigned int value)
{
    unsigned char i;
    for (i = 0; i < 16; i++)
    {
        COL_DATA = (value & 0x8000) ? 1 : 0;
        COL_CLK = 0;
        COL_CLK = 1;
        value <<= 1;
    }
    COL_LAT = 0;
    COL_LAT = 1;
}

void scan_char(unsigned char char_index)
{
    unsigned char row;
    unsigned int col;

    for (row = 0; row < 16; row++)
    {
        col = ((unsigned int)name_font[char_index][row * 2] << 8) |
              name_font[char_index][row * 2 + 1];
        COL_OE = 1;
        select_row(row);
        shift_out16(~col);
        COL_OE = 0;
        delay_short();
    }
}

void main(void)
{
    unsigned char char_index = 0;
    unsigned int frame_count = 0;

    P1 = 0xFF;
    while (1)
    {
        scan_char(char_index);
        frame_count++;

        if (frame_count >= 180)
        {
            frame_count = 0;
            if (DIR_SW)
            {
                char_index++;
                if (char_index >= 3)
                {
                    char_index = 0;
                }
            }
            else
            {
                if (char_index == 0)
                {
                    char_index = 2;
                }
                else
                {
                    char_index--;
                }
            }
        }
    }
}
