/* IR_sendByte_V3
 *
 * Sketch to demonstrate sending a single byte by modulated 38 kHz infrared
 * using a MSP430G2553 LaunchPad, Energia V17, and 940 nm infrared LED.  The
 * sketch waits for a button push and then counts from 0 to 255 while sending 
 * the current count as a byte.  It then waits for another button push.
 * The red LED is on while IR is being sent.
 *
 * Characteristics:
 * modulation: 38 kHz
 * Start signal:  startPulse uSec pulse, startPause uSec pause
 * 0 bit:         zeroPulse uSec pulse, zeroPulse uSec pause
 * 1 bit:         onePulse uSec pulse, onePause uSec pause
 * Stop signal:   stopPulse uSec pulse, stopPause uSec pause
 *
 * Connect IR LED to pin 6 and GND
 *
 * Reception tested with Vishay TSOP38238
 *
 * Credit for portions of code framework to adafruit.com
 * Released into the public domain
 * F. Milburn      December 2016
 */

// Pulses and pauses (uSec) should be multiples of 26 and at least 260 long 
const int startPulse = 936;
const int startPause = 312;
const int zeroPulse = 312;
const int zeroPause = 312;
const int onePulse = 624;
const int onePause = 312;
const int stopPulse = 936;
const int stopPause = 312;
const int IRpin = 6;              // connect IR LED to this pin
const int PUSH1 = P1_3;           // LaunchPad push button
const int delayBetweenBytes = 50; // delay between bytes in mSec

void pulseIR(unsigned long microSecs);
void startIR();
void sendIR0();
void sendIR1();
void sendIR(byte data);

void setup(){
  pinMode(IRpin, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  pinMode(PUSH1, INPUT_PULLUP);
}

void loop(){
  
  while (digitalRead(PUSH1) == HIGH){
    // wait for button push
  }
  digitalWrite(RED_LED, HIGH);
  int i = 0;
  do {
    sendIR(i);                      // send the byte here...
    delay(delayBetweenBytes);       // pause between sent bytes
    i++;
  } while (i < 256);
  digitalWrite(RED_LED, LOW);
}

void pulseIR(unsigned long microSecs){
  // pulses at 38 KHz IRpin for user defined duration in micro seconds
  // microSecs should be a multiple of 26, and at least 260 uS in length
  noInterrupts();                  // interrupts screw up the timing
  while (microSecs > 0){  
    // 38 KHz has period of approximately 26 microseconds    
    digitalWrite(IRpin, HIGH);     // takes about 4 microseconds
    delayMicroseconds(9);          // wait 9 microseconds
    digitalWrite(IRpin, LOW);      // takes about 4 microseconds
    delayMicroseconds(9);          // wait 9 microseconds
                                  
    microSecs -= 26;               // 26 microseconds have passed
  }
  interrupts();                    // interrupts allowed again
}
void startIR(){  
  pulseIR(startPulse);             // pulse to start transmission   
  delayMicroseconds(startPause);
}  
void stopIR(){  
  pulseIR(stopPulse);              // pulse to stop transmission   
  delayMicroseconds(stopPause);   
}
void sendIR0(){                            
  pulseIR(zeroPulse);              // pulse for 0
  delayMicroseconds(zeroPause);
}
void sendIR1(){         
  pulseIR(onePulse);               // pulse for 1
  delayMicroseconds(onePause);
}
void sendIR(byte data){
  // sends a single byte, least significant bit first
  startIR();
  int i;
  for (i = 0; i<8; i++){
    if (data & 0x01){
      sendIR1();
    }
    else{
      sendIR0();
    }
    data = data >> 1;
  }
  stopIR();
}
