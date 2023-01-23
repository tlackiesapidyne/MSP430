#include <msp430.h>				


/**
 * main.c
 */
void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// stop watchdog timer
	PM5CTL0 &= ~LOCKLPM5;           // disable high-impedance mode by clearing LOCKLPM5 bit in PM5CTL0
	P1DIR |= 0x01;					// configure P1.0 as output

	volatile unsigned int i;		// volatile to prevent optimization

	while(1)
	{
		P1OUT ^= 0x01;				// toggle P1.0
		for(i=20000; i>0; i--);     // delay
	}
}
