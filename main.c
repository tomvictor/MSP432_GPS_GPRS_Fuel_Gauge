

#include "msp.h"

void main(void)
{
    volatile uint32_t i;

    WDTCTL = WDTPW | WDTHOLD;                    /* Stop watchdog timer */

    // The following code toggles P1.0 port
    P1DIR |= BIT0;                               /* Configure P1.0 as output */

    while(1)
    {
        P1OUT ^= BIT0;                           /* Toggle P1.0 */
        for(i=10000; i>0; i--);                  /* Delay */
    }
}
