/*
 * EX2 Advanced (with 7-segment display):
 * Traffic light + emergency priority + 2-digit countdown display
 *
 * Based on EX2_Traffic_Emergency.c; display wiring follows the
 * style of section 4.4 (KeyScan.C) in the lab guide.
 *
 * Port assignment:
 *   P1.0  NS red
 *   P1.1  NS yellow
 *   P1.2  NS green
 *   P1.3  EW red
 *   P1.4  EW yellow
 *   P1.5  EW green
 *
 *   P0    7-segment data (common-anode inverted codes)
 *   P2.7  tens  digit select  (active high, directly drives COM of CA display)
 *   P2.6  units digit select  (active high)
 *
 *   INT0  Emergency: all-red 10 s, then restore previous state
 *
 * Note: P1 is already used by traffic LEDs, so digit-select is moved
 * to P2 instead of P1 as in the original 4.4 wiring.
 */

#include "SST89x5x4.h"

/* Traffic light port masks                                             */
#define NS_RED   0x01
#define NS_YEL   0x02
#define NS_GRN   0x04
#define EW_RED   0x08
#define EW_YEL   0x10
#define EW_GRN   0x20

typedef enum
{
    NS_GREEN = 0,
    NS_YELLOW,
    EW_GREEN,
    EW_YELLOW
} traffic_state_t;

/* 7-segment encoding table (common anode, digits 0-9)
   Common anode: segment ON = LOW (0), so codes are bitwise complement
   of the common-cathode table used in section 4.4.               */
unsigned char code seg_code[] =
{
    0xC0, /* 0: ~0x3F */
    0xF9, /* 1: ~0x06 */
    0xA4, /* 2: ~0x5B */
    0xB0, /* 3: ~0x4F */
    0x99, /* 4: ~0x66 */
    0x92, /* 5: ~0x6D */
    0x82, /* 6: ~0x7D */
    0xF8, /* 7: ~0x07 */
    0x80, /* 8: ~0x7F */
    0x90  /* 9: ~0x6F */
};

/* Global state variables                                               */
volatile unsigned int       tick_50ms      = 0;
volatile unsigned char      remain_seconds = 8;
volatile traffic_state_t    state          = NS_GREEN;
volatile bit                second_flag    = 0;
volatile bit                emergency_req  = 0;
volatile bit                emergency_active = 0;
volatile unsigned char      saved_remain   = 0;
volatile traffic_state_t    saved_state    = NS_GREEN;

/* Utilities                                                            */

/* Short blocking delay used between display digit strobes.
   ~1 ms at 12 MHz (each loop iteration ≈ 12 clock cycles). */
void delay_disp(unsigned int t)
{
    unsigned int i;
    for (i = 0; i < t; i++);
}

/* Display                                                              */

/*
 * display_countdown()
 * Drives two common-anode 7-segment digits to show 'seconds' (0-99).
 *
 * Common anode: digit COM = HIGH to select, LOW to blank.
 *   P2 = 0x00  ->  both digits off  (blank)
 *   P2 = 0x80  ->  P2.7 = 1  ->  tens  digit active
 *   P2 = 0x40  ->  P2.6 = 1  ->  units digit active
 */
void display_countdown(unsigned char seconds)
{
    unsigned char tens  = seconds / 10;
    unsigned char units = seconds % 10;

    /* --- tens digit --- */
    P2 = 0x00;              /* blank both COMs first (anti-ghosting)     */
    P0 = seg_code[tens];
    P2 = 0x80;              /* P2.7 = 1: select tens digit               */
    delay_disp(0x400);      /* hold ~1 ms                                */

    /* --- units digit --- */
    P2 = 0x00;
    P0 = seg_code[units];
    P2 = 0x40;              /* P2.6 = 1: select units digit              */
    delay_disp(0x400);

    P2 = 0x00;              /* blank at end of cycle                     */
}

/* Traffic light helpers (unchanged from EX2_Traffic_Emergency.c)      */

void show_state(traffic_state_t s)
{
    switch (s)
    {
    case NS_GREEN:  P1 = NS_GRN | EW_RED; break;
    case NS_YELLOW: P1 = NS_YEL | EW_RED; break;
    case EW_GREEN:  P1 = NS_RED | EW_GRN; break;
    case EW_YELLOW: P1 = NS_RED | EW_YEL; break;
    default:        P1 = 0x00;            break;
    }
}

void all_red(void)
{
    P1 = NS_RED | EW_RED;
}

void next_state(void)
{
    switch (state)
    {
    case NS_GREEN:  state = NS_YELLOW; remain_seconds = 2; break;
    case NS_YELLOW: state = EW_GREEN;  remain_seconds = 8; break;
    case EW_GREEN:  state = EW_YELLOW; remain_seconds = 2; break;
    case EW_YELLOW: state = NS_GREEN;  remain_seconds = 8; break;
    default:        state = NS_GREEN;  remain_seconds = 8; break;
    }
}

/* Interrupt service routines                                           */

void timer0_isr(void) interrupt 1
{
    TH0 = 0x3C;
    TL0 = 0xB0;
    tick_50ms++;
    if (tick_50ms >= 20)
    {
        tick_50ms  = 0;
        second_flag = 1;
    }
}

void int0_isr(void) interrupt 0
{
    emergency_req = 1;
}

/* Main                                                                 */

void main(void)
{
    TMOD = 0x01;
    TH0  = 0x3C;
    TL0  = 0xB0;
    IT0  = 1;
    EX0  = 1;
    ET0  = 1;
    EA   = 1;
    TR0  = 1;

    show_state(state);

    while (1)
    {
        /*
         * Refresh the 7-segment display every loop pass.
         * Two digits × ~1 ms each = ~2 ms per refresh cycle,
         * well within the 50 ms Timer0 tick period.
         */
        display_countdown(remain_seconds);

        /* Handle emergency vehicle request */
        if (emergency_req && !emergency_active)
        {
            emergency_req    = 0;
            emergency_active = 1;
            saved_state      = state;
            saved_remain     = remain_seconds;
            tick_50ms        = 0;
            second_flag      = 0;
            remain_seconds   = 10;
            all_red();
        }

        /* Process one-second tick */
        if (second_flag)
        {
            second_flag = 0;

            if (remain_seconds > 0)
                remain_seconds--;

            if (remain_seconds == 0)
            {
                if (emergency_active)
                {
                    emergency_active = 0;
                    state            = saved_state;
                    remain_seconds   = saved_remain;
                    show_state(state);
                }
                else
                {
                    next_state();
                    show_state(state);
                }
            }
        }
    }
}
