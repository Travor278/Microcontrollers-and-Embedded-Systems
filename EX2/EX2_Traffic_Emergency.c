#include "SST89x5x4.h"

/*
 * EX2 Advanced: Traffic light system with emergency vehicle priority
 *
 * The system is modeled as a time-driven finite state machine:
 *   1. NS green  / EW red
 *   2. NS yellow / EW red
 *   3. EW green  / NS red
 *   4. EW yellow / NS red
 *
 * Main loop:
 *   - Advances countdown using software seconds from Timer0
 *   - Switches to next state when current duration expires
 *   - On emergency request: saves context, switches to all-red mode
 *
 * Interrupts:
 *   - Timer0: stable time base (50 ms tick)
 *   - INT0:   sets emergency_req flag only; main loop does the rest
 */

/*
 * Port assignment:
 * P1.0  NS red
 * P1.1  NS yellow
 * P1.2  NS green
 * P1.3  EW red
 * P1.4  EW yellow
 * P1.5  EW green
 *
 * INT0  Emergency request, low-level triggered.
 *       All-red for 10 s, then restore previous state.
 */

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

/* tick_50ms: incremented by Timer0 ISR every 50 ms */
volatile unsigned int tick_50ms = 0;

/* remain_seconds: how many software-seconds remain in the current state */
volatile unsigned char remain_seconds = 8;

/* state: current traffic light state, initialized to NS green */
volatile traffic_state_t state = NS_GREEN;

/* second_flag: set by Timer0 ISR when 1 s has elapsed */
volatile bit second_flag = 0;

/* emergency_req: set by INT0 ISR when an emergency vehicle is detected */
volatile bit emergency_req = 0;

/* emergency_active: true while the all-red protection phase is running */
volatile bit emergency_active = 0;

/* saved state and remaining time before emergency, restored afterwards */
volatile unsigned char saved_remain = 0;
volatile traffic_state_t saved_state = NS_GREEN;

/*
 * show_state() - translate abstract state to P1 port output.
 * Wiring changes only require editing this function.
 */
void show_state(traffic_state_t s)
{
    switch (s)
    {
    case NS_GREEN:
        P1 = NS_GRN | EW_RED;
        break;
    case NS_YELLOW:
        P1 = NS_YEL | EW_RED;
        break;
    case EW_GREEN:
        P1 = NS_RED | EW_GRN;
        break;
    case EW_YELLOW:
        P1 = NS_RED | EW_YEL;
        break;
    default:
        P1 = 0x00;
        break;
    }
}

/* all_red() - both directions stopped; used during emergency phase */
void all_red(void)
{
    P1 = NS_RED | EW_RED;
}

/*
 * next_state() - advance state machine and load the new duration.
 * Cycle: NS_GREEN(8s) -> NS_YELLOW(2s) -> EW_GREEN(8s) -> EW_YELLOW(2s) -> ...
 */
void next_state(void)
{
    switch (state)
    {
    case NS_GREEN:
        state = NS_YELLOW;
        remain_seconds = 2;
        break;
    case NS_YELLOW:
        state = EW_GREEN;
        remain_seconds = 8;
        break;
    case EW_GREEN:
        state = EW_YELLOW;
        remain_seconds = 2;
        break;
    case EW_YELLOW:
        state = NS_GREEN;
        remain_seconds = 8;
        break;
    default:
        state = NS_GREEN;
        remain_seconds = 8;
        break;
    }
}

/*
 * Timer0 ISR:
 * 1. Reload TH0/TL0 for the next period
 * 2. Increment tick_50ms
 * 3. Set second_flag when 20 ticks (1 s) have accumulated
 */
void timer0_isr(void) interrupt 1
{
    TH0 = 0x3C;
    TL0 = 0xB0;

    tick_50ms++;
    if (tick_50ms >= 20)
    {
        tick_50ms = 0;
        second_flag = 1;
    }
}

/*
 * INT0 ISR:
 * Only sets emergency_req. Main loop handles the actual all-red logic
 * to keep the ISR short and avoid re-entrancy issues.
 */
void int0_isr(void) interrupt 0
{
    emergency_req = 1;
}

void main(void)
{
    /* Timer0 mode 1: 16-bit timer */
    TMOD = 0x01;

    TH0 = 0x3C;
    TL0 = 0xB0;

    /* INT0 falling-edge triggered */
    IT0 = 1;

    EX0 = 1;
    ET0 = 1;
    EA = 1;

    TR0 = 1;

    show_state(state);

    while (1)
    {
        /*
         * Handle emergency request.
         * Only enter all-red if not already in emergency phase.
         */
        if (emergency_req && !emergency_active)
        {
            emergency_req = 0;
            emergency_active = 1;

            saved_state = state;
            saved_remain = remain_seconds;

            /* Reset tick so the 10 s all-red counts from now */
            tick_50ms = 0;
            second_flag = 0;

            remain_seconds = 10;
            all_red();
        }

        if (second_flag)
        {
            second_flag = 0;

            if (remain_seconds > 0)
            {
                remain_seconds--;
            }

            if (remain_seconds == 0)
            {
                if (emergency_active)
                {
                    /* End of emergency phase: restore saved context */
                    emergency_active = 0;
                    state = saved_state;
                    remain_seconds = saved_remain;
                    show_state(state);
                }
                else
                {
                    /* Normal operation: advance to next state */
                    next_state();
                    show_state(state);
                }
            }
        }
    }
}
