/* Output the value of a single byte / char on 8 LEDs with MSP 430G2553
 *
 * Tested on MSP430G2553 LaunchPad with CCS V7.1 with TI v16.9.1 compiler
 * Counts from 0 to 255 and outputs current number on 8 LEDs
 * LEDs are on P1.0, P1.1, P1.2, P1.3, P1.4, P1.5, P2.0, P2.1
 * Clock is set at 1 MHz
 * Includes a millisecond timer from Timer A0 counting up
*/
#include <msp430.h>				

#define  COUNT               1000        // Timer A counts to ~ 1 millisecond

int LEDpinMap[8] = {BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT0, BIT1};
volatile unsigned long millis = 0;       // Total milliseconds counted

void showByte(unsigned char x);
void initIO();
void initClock();
void initTimer();

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		    // Stop watchdog timer

	initClock();
	initIO();
	initTimer();
    _BIS_SR(GIE);                       // Global activation of interrupts

    for(;;) {
		unsigned char i;
		for (i = 0; i <256; i++){

		    showByte(i);                // shows byte on LEDs

	        unsigned long startMillis = millis;
	        unsigned long endMillis = startMillis + 50;
	        while(millis < endMillis){} // software delay
		}
	}
	
	return 0;
}

void showByte(unsigned char x){
// shows (outputs) x on 8 external LEDs
    unsigned int i;
    for (i = 0; i < 8; i++){
        if (i < 6){
            if (x & 0X01){
                P1OUT |= LEDpinMap[i];
            }
            else {
                P1OUT &= ~LEDpinMap[i];
            }
        }
        else{
            if (x & 0X01){
                P2OUT |= LEDpinMap[i];
            }
            else {
                P2OUT &= ~LEDpinMap[i];
            }
        }
        x = x >> 1;
    }
}

void initIO(){
    // set the LED pins as output, then turn LEDs off
    unsigned int i;
    for (i = 0; i < 8; i++){
        if (i < 6){
            P1DIR |= LEDpinMap[i];
            P1OUT &= ~LEDpinMap[i];
        }
        else{
            P2DIR |= LEDpinMap[i];
            P2OUT &= ~LEDpinMap[i];
        }
    }
}

void initTimer() {
    TA0CCR0 = COUNT;                  // Timer A0 counts to this number
    TA0CTL = TASSEL_2;                // SMCLK
    TA0CTL |= MC_1;                   // Count up
    TA0CCTL0 = CCIE;                  // Enable the interrupt for Timer A0
}

void initClock() {
    BCSCTL1 = CALBC1_1MHZ;            // load calibrated data for 1 MHz
    DCOCTL = CALDCO_1MHZ;
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_ISR (void)
{
    millis++;                         // Increment number of seconds since reset                                                                                    // REMOVE
}
