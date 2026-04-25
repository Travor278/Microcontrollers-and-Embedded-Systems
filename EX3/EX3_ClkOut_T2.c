#include "SST89x5x4.h"

void main(void)
{
    /*
     * Timer2 programmable clock output.
     * P1.0/T2 outputs a 50% duty cycle clock.
     *
     * Fout = Fosc / (n * (65536 - RCAP2))
     * In 12-clock mode, n = 4.
     */
    RCAP2H = 0xFF;
    RCAP2L = 0x00;

    T2MOD = 0x02;    /* enable Timer2 output */
    T2CON = 0x04;    /* start Timer2 */

    while (1)
    {
    }
}
