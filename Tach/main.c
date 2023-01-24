#include <msp430.h>				


/**
 * main.c
 */

void init_CS(void) {
    // Configure two FRAM waitstate as required by the device datasheet for MCLK
    // operation at 24MHz(beyond 8MHz) _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_2 ;

    P2SEL1 |= BIT6 | BIT7;                       // P2.6~P2.7: crystal pins
    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);           // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag

    __bis_SR_register(SCG0);                     // disable FLL
    CSCTL3 |= SELREF__XT1CLK;                    // Set XT1 as FLL reference source
    CSCTL0 = 0;                                  // clear DCO and MOD registers
    CSCTL1 = DCORSEL_7;                         // Set DCO = 24MHz
    CSCTL2 = FLLD_0 + 731;                       // DCOCLKDIV = 24MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                     // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));   // FLL locked

    CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK;   // set XT1 (~32768Hz) as ACLK source, ACLK = 32768Hz
                                                 // default DCOCLKDIV as MCLK and SMCLK source

    P3DIR |= BIT4;
    P3SEL0 |= BIT4;
    P3SEL1 &= ~BIT4;
}

void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;		// stop watchdog timer

	P1DIR |= 0x01;					// configure P1.0 as output
	P6DIR |= BIT6;                  // configure P6.6 as output
	P6SEL0 |= BIT6;                 // options select

	init_CS();

/*	do
	{
	    CSCTL7 &= ~(XT1OFFG | DCOFFG);                // Clear XT1 and DCO fault flag
	    SFRIFG1 &= ~OFIFG;
	}while (SFRIFG1 & OFIFG);                         // Test oscillator fault flag
*/
	PM5CTL0 &= ~LOCKLPM5;           // disable high-impedance mode by clearing LOCKLPM5 bit in PM5CTL0

	TB3CCR0 = 100-1;                // PWM Period
	TB3CCR1 = 50;                   // CCR1 Duty Cycle
	TB3CCTL1 = OUTMOD_7;            // CCR1 Reset/Set
	TB3CTL = TBSSEL_1 | MC_1 | TBCLR;   // ACLK, up mode, clear TBR

	volatile unsigned int i;		// volatile to prevent optimization

	while(1)
	{
		P1OUT ^= 0x01;				// toggle P1.0
		for(i=200000; i>0; i--);     // delay
	}
}
