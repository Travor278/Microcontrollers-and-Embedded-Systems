#include "SST89x5x4.h"
#include "intrins.h"

/*
 * 16x16 dot-matrix template for the new experiment box.
 * The concrete driver pins depend on the SM16206/SM5166 wiring
 * on the laboratory hardware. Use the pin names below as a mapping layer,
 * then replace name_font[] with the 16x16 font of your own name.
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
        0x00, 0x00, 0x7F, 0xFC, 0x04, 0x20, 0x04, 0x20,
        0x04, 0x20, 0x3F, 0xF8, 0x04, 0x20, 0x04, 0x20,
        0x04, 0x20, 0x7F, 0xFC, 0x04, 0x20, 0x04, 0x20,
        0x08, 0x20, 0x10, 0x20, 0x20, 0x20, 0x00, 0x00
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
        if (!DIR_SW)
        {
            col = (unsigned int)((col >> 1) | (col << 15));
        }

        COL_OE = 1;
        select_row(row);
        shift_out16(~col);
        COL_OE = 0;
        delay_short();
    }
}

void main(void)
{
    P1 = 0xFF;
    while (1)
    {
        scan_char(0);
    }
}
