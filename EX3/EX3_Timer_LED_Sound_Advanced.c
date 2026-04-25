#include "SST89x5x4.h"

/*
 * EX3 advanced task
 *
 * Port assignment for Proteus:
 *   P1.0-P1.7 -> LED L1-L8, active high
 *   P2.0-P2.7 -> 1-digit 7-segment display a,b,c,d,e,f,g,dp, common cathode
 *   P3.7      -> speaker driver input
 *
 * Timer1:
 *   mode 1, 50 ms overflow interrupt.
 *   With 11.0592 MHz crystal and 12-clock mode:
 *   timer clock = 921600 Hz, 50 ms = 46080 counts, reload = 0x4C00.
 *
 * Timer0:
 *   mode 1, generates the square wave for the speaker.
 */

sbit SPK = P3^7;

#define TIMER_CLOCK 1000000UL
#define T1_RELOAD_H 0x3C
#define T1_RELOAD_L 0xB0

unsigned char code led_pattern[8] = {
    0x05,   /* 1s: L1, L3 */
    0x0A,   /* 2s: L2, L4 */
    0x50,   /* 3s: L5, L7 */
    0xA0,   /* 4s: L6, L8 */
    0x55,   /* 5s: L1, L3, L5, L7 */
    0xAA,   /* 6s: L2, L4, L6, L8 */
    0xFF,   /* 7s: all on */
    0x00    /* 8s: all off */
};

/* Common-cathode segment codes: 0-9 */
unsigned char code seg_cc[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

/* Song 1: 16 notes * 10 ticks = 8 s */
unsigned int code song1_freq[] = {
    262, 262, 393, 393, 441, 441, 393, 0,
    350, 350, 330, 330, 294, 294, 262, 0
};

unsigned char code song1_time[] = {
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10
};

/* Song 2: 16 notes * 10 ticks = 8 s */
unsigned int code song2_freq[] = {
    330, 330, 350, 393, 393, 350, 330, 294,
    262, 262, 294, 330, 330, 294, 294, 0
};

unsigned char code song2_time[] = {
    10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10
};

unsigned char data tone_h = 0x00;
unsigned char data tone_l = 0x00;

volatile unsigned char tick_50ms = 0;
volatile unsigned char pending_50ms = 0;
volatile unsigned char second_index = 0;

unsigned char data song_no = 0;
unsigned char data note_index = 0;
unsigned char data note_ticks_left = 0;

void display_second(void)
{
    P2 = seg_cc[second_index + 1];
}

void set_tone(unsigned int freq)
{
    unsigned long counts;
    unsigned int reload;

    if (freq == 0)
    {
        TR0 = 0;
        SPK = 0;
        return;
    }

    counts = TIMER_CLOCK / (2UL * freq);
    reload = (unsigned int)(65536UL - counts);

    tone_h = (unsigned char)(reload >> 8);
    tone_l = (unsigned char)(reload & 0xFF);

    TH0 = tone_h;
    TL0 = tone_l;
    TF0 = 0;
    TR0 = 1;
}

void load_note(void)
{
    unsigned int freq;

    if (song_no == 0)
    {
        freq = song1_freq[note_index];
        note_ticks_left = song1_time[note_index];
    }
    else
    {
        freq = song2_freq[note_index];
        note_ticks_left = song2_time[note_index];
    }

    set_tone(freq);
}

void next_note(void)
{
    note_index++;

    if (note_index >= 16)
    {
        note_index = 0;
        song_no++;
        if (song_no >= 2)
        {
            song_no = 0;
        }
    }

    load_note();
}

void music_tick(void)
{
    if (note_ticks_left > 0)
    {
        note_ticks_left--;
    }

    if (note_ticks_left == 0)
    {
        next_note();
    }
}

void timer0_isr(void) interrupt 1
{
    TH0 = tone_h;
    TL0 = tone_l;
    SPK = ~SPK;
}

void timer1_isr(void) interrupt 3
{
    TH1 = T1_RELOAD_H;
    TL1 = T1_RELOAD_L;

    if (pending_50ms < 250)
    {
        pending_50ms++;
    }

    tick_50ms++;
    if (tick_50ms >= 20)
    {
        tick_50ms = 0;
        second_index++;
        if (second_index >= 8)
        {
            second_index = 0;
        }

        P1 = led_pattern[second_index];
        display_second();
    }
}

void main(void)
{
    P1 = led_pattern[0];
    second_index = 0;
    display_second();

    TMOD = 0x11;      /* T1 mode 1 + T0 mode 1 */

    TH1 = T1_RELOAD_H;
    TL1 = T1_RELOAD_L;

    load_note();

    ET0 = 1;
    ET1 = 1;
    EA = 1;

    TR1 = 1;

    while (1)
    {
        if (pending_50ms > 0)
        {
            pending_50ms--;
            music_tick();
        }
    }
}
