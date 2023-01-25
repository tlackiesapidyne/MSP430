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
    CSCTL1 = DCORSEL_7;                          // Set DCO = 24MHz
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
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	P1DIR |= 0x01;				// configure P1.0 as output
	P6DIR |= BIT0;              // configure P6.6 as output
	P6SEL0 |= BIT0;             // options select

	// Configure GPIO
    P1DIR |= BIT2;              // Set P1.2 to output direction
    P1OUT &= ~BIT2;             // Clear P1.2

    // Configure ADC A1 pin
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;

    CSCTL4 = SELA__XT1CLK;      // Set ACLK = XT1; MCLK = SMCLK = DCO

    // Configure ADC
    ADCCTL0 |= ADCON | ADCMSC;                   // ADCON
    ADCCTL1 |= ADCSHP | ADCSHS_2 | ADCCONSEQ_2;  // repeat single channel; TB1.1 trig sample start
    ADCCTL2 &= ~ADCRES;                          // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                         // 12-bit conversion results
    ADCMCTL0 |= ADCINCH_1 | ADCSREF_1;           // A1 ADC input select; Vref=1.5V
    ADCIE |= ADCIE0;                             // Enable ADC conv complete interrupt

    // Configure reference
    PMMCTL0_H = PMMPW_H;              // Unlock the PMM registers
    PMMCTL2 |= INTREFEN | REFVSEL_0;  // Enable internal 1.5V reference
    __delay_cycles(400);              // Delay for reference settling
     ADCCTL0 |= ADCENC;               // ADC Enable

    // ADC conversion trigger signal - TimerB1.1 (32ms ON-period)
    TB1CCR0 = 1024-1;                     // PWM Period
    TB1CCR1 = 512-1;                      // TB1.1 ADC trigger
    TB1CCTL1 = OUTMOD_4;                  // TB1CCR0 toggle
    TB1CTL = TBSSEL__ACLK | MC_1 | TBCLR; // ACLK, up mode

    __bis_SR_register(GIE);               // interrupts

	init_CS();

	PM5CTL0 &= ~LOCKLPM5;           // disable high-impedance mode by clearing LOCKLPM5 bit in PM5CTL0

	TB3CCR0 = 100-1;                // PWM Period
	TB3CCR1 = 50;                   // CCR1 Duty Cycle (pulse width)
	TB3CCTL1 = OUTMOD_7;            // CCR1 Reset/Set
	TB3CTL = TBSSEL_1 | MC_1 | TBCLR;   // ACLK, up mode, clear TBR

	volatile unsigned int i;		// volatile to prevent optimization

	while(1)
	{
		P1OUT ^= 0x01;				// toggle P1.0
		for(i=10000; i>0; i--);     // delay
	}
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            if (ADCMEM0 < 0x555)      // ADCMEM = A0 < 0.5V?
                P1OUT &= ~BIT2;       // Clear P1.2 LED off
            else
                P1OUT |= BIT2;        // Set P1.2 LED on
            ADCIFG = 0;
            break;                    // Clear CPUOFF bit from 0(SR)
        default:
            break;
    }
}

