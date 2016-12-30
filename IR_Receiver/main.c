/* IR_Receiver_V3
 *
 * Demonstrates reception of bytes from a Vishay TSOP38238 IR receiver.  Assumes that
 * first byte received will be a 0, then ensuing bytes will count sequentially up to
 * 255 where it will pause.  Out of sequence bits will count as an error.
 *
 * The receiver output is input on P1.5 of a MSP430G2553.
 *
 * Signal characteristics are as follows:
 * modulation: 38 kHz
 * LSB received first
 * Start signal:  936 uSec pulse, 312 uSec pause
 * 0 bit:         312 uSec pulse, 312 uSec pause
 * 1 bit:         624 uSec pulse, 312 uSec pause
 * Stop signal:   936 uSec pulse, 312 uSec pause
 *
 * MSP430G2553 has two outputs:
 * P2.5  toggles on rising edge - useful for debug on oscilloscope
 * LED pins can show the byte received on 8 pins (binary)
 * Any errors detected will show when byte 255 is received
 *
 * Master Clock running at 16 MHz
 * TA0 timer running at 2MHz (period = 0.5 uSec)
 *
 * Code compiled and linked in CCS V7.0 using TI v16.12.0.STS compiler.
 *
 * Credit for IR code framework to RobG on 43oh.com
 * Released into the public domain
 * F Milburn    December 2016
 */
#include "msp430g2553.h"

// Note:  A0CLK running at 2 MHz
#define T0min   400*2                       // min "0" pulse
#define T0max   750*2                       // max "0" pulse - should be ~ 620 uSec
#define T1min   850*2                       // min "1" pulse
#define T1max   1100*2                      // max "1" pulse - should be ~ 940 uSec
#define Tsmin   300*2                       // min "start" pulse
#define Tsmax   1100*2                      // max "start" pulse - should be ~ 940 uSec
#define TMAX    30000

#define IR_DETECTOR_PIN BIT7                // IR input on P1.5
const int oscopePin = BIT6;                 // O'scope monitor on P1.6
// LED binary output monitor
const int LEDpinMap[8] = {BIT4, BIT5, BIT0, BIT1, BIT2, BIT3, BIT4, BIT5};

volatile unsigned char rxData = 0;          // received data
volatile unsigned int  bitCounter = 0;
volatile unsigned int  pulseTime = 0;
volatile int           validBytes = 0;


void reset();
void initTimer();
void initIO();
void initClock();
void showByte(unsigned char x);

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;                // stop WDT

    initClock();
    initTimer();
    initIO();

    __bis_SR_register(GIE);                  // enable interrupts

    while(1){
        if (rxData == 255){
            showByte(255 - (unsigned char)validBytes);
            validBytes = 0;
            reset();
        }
    }

}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {

    if (P1IFG & IR_DETECTOR_PIN) {           // if the interrupt flag is on IR_DETECTOR_PIN
        P1IE &= ~IR_DETECTOR_PIN;            // set interrupt enable to 0 (i.e. turn off)

        if (bitCounter == 0) {
            P1OUT ^= oscopePin;
            P1IES &= ~IR_DETECTOR_PIN;       // look for rising edge
            bitCounter++;
            TA0CTL |= TACLR;                 // clear TA0
            TA0CTL |= MC_1;                  // up mode - count to TACCR0 (TMAX)
            CCTL0 = CCIE;                    // TA0 interrupt enabled
        }
        else {
            pulseTime = TA0R;
            switch (bitCounter) {
            case 10:                         // received all bits
                showByte(rxData);
                validBytes++;
                reset();
                break;
            case 1:                          // start bit?
                if ((pulseTime < Tsmin) || (pulseTime > Tsmax)) {
                    reset();                 // invalid
                    break;
                } else {                     // this is a valid start bit
                    TA0CTL |= TACLR;         // clear
                    TA0CTL |= MC_1;          // up mode
                    P1OUT ^= oscopePin;
                    bitCounter++;
                }
                break;
            default:                          // data bit
                rxData >>= 1;                 // shift right one place
                P1OUT ^= oscopePin;
                if ((pulseTime > T1min) && (pulseTime < T1max)) {    // valid "1"
                    rxData |= 0x080;          // long pause, set bit 7 of rxData
                }
                else if (pulseTime < T0min){
                    reset();                  // invalid
                    break;
                }
                TA0CTL |= TACLR;              // clear timer
                TA0CTL |= MC_1;               // count up
                bitCounter++;                 // increase bit counter
                break;
            }
        }
        P1IFG &= ~IR_DETECTOR_PIN;            // interrupt flag cleared
        P1IE |= IR_DETECTOR_PIN;              // enable interrupts again
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    P1IE &= ~IR_DETECTOR_PIN;
    reset();
    P1IFG &= ~IR_DETECTOR_PIN;
    P1IE |= IR_DETECTOR_PIN;
}

void reset() {
    TA0CCTL0 &= ~CCIE;
    P1IES |= IR_DETECTOR_PIN;                // look for falling edge
    rxData = 0;
    bitCounter = 0;
}

void initTimer() {
    TA0CCR0 = TMAX;                          // interrupt if no edge for T32
    TA0CTL = TASSEL_2;                       // use SMCLK
    TA0CTL |= ID_3;                          // divide clock input by 8
    TA0CTL |= MC_1;                          // count up
}


void initIO(){

    P1IE |= IR_DETECTOR_PIN;                 // interrupt enabled
    P1IES |= IR_DETECTOR_PIN;                // look for falling edge
    P1IFG &= ~IR_DETECTOR_PIN;               // clear IFG (interrupt flag)

    P1DIR |= oscopePin;
    P1OUT &= ~oscopePin;                     // output for oscilloscope monitoring

    unsigned int i;
    for (i = 0; i < 8; i++){                 // output for binary display on 8 LEDs
        if (i < 2){
            P1DIR |= LEDpinMap[i];
            P1OUT &= ~LEDpinMap[i];
        }
        else{
            P2DIR |= LEDpinMap[i];
            P2OUT &= ~LEDpinMap[i];
        }
    }
}

void showByte(unsigned char x){
// shows (outputs) x on 8 external LEDs
    unsigned int i;
    for (i = 0; i < 8; i++){
        if (i < 2){
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

void initClock(){
    BCSCTL1 = CALBC1_16MHZ;                   // load calibrated data
    DCOCTL = CALDCO_16MHZ;
}
