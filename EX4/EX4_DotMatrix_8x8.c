#include "SST89x5x4.h"
#include "intrins.h"

/*
 * Proteus-friendly 8x8 dot matrix demo.
 * P1.0-P1.7: row select, active high.
 * P2.0-P2.7: column data, active low.
 * P3.2: direction switch. 1 = left, 0 = right.
 *
 * Replace digit_font[] with the student's own student-number font
 * when using the old 8x8 experiment box.
 */

sbit DIR_SW = P3^2;

unsigned char code digit_font[][8] = {
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00}, /* 0 */
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00}, /* 1 */
    {0x3C, 0x66, 0x06, 0x1C, 0x30, 0x66, 0x7E, 0x00}, /* 2 */
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00}, /* 3 */
    {0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00}  /* 4 */
};

void delay_us(unsigned int n)
{
    while (n--)
    {
        _nop_();
    }
}

void delay_frame(unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
    {
        delay_us(200);
    }
}

void scan_frame(unsigned char char_index, unsigned char shift)
{
    unsigned char row;
    unsigned char col_data;

    for (row = 0; row < 8; row++)
    {
        col_data = digit_font[char_index][row];
        if (DIR_SW)
        {
            col_data = _crol_(col_data, shift);
        }
        else
        {
            col_data = _cror_(col_data, shift);
        }

        P1 = (unsigned char)(1 << row);
        P2 = (unsigned char)(~col_data);
        delay_us(250);
        P1 = 0x00;
        P2 = 0xFF;
    }
}

void main(void)
{
    unsigned char char_index = 0;
    unsigned char shift = 0;
    unsigned int frame_count = 0;

    P1 = 0x00;
    P2 = 0xFF;
    P3 |= 0x04;

    while (1)
    {
        scan_frame(char_index, shift);
        frame_count++;

        if (frame_count >= 60)
        {
            frame_count = 0;
            shift++;
            if (shift >= 8)
            {
                shift = 0;
                char_index++;
                if (char_index >= 5)
                {
                    char_index = 0;
                }
            }
        }
    }
}
