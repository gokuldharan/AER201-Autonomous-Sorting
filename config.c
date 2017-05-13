#include <xc.h>
#include <stdio.h>
#include "configBits.h"
#include "constants.h"
#include "macros.h"
#include "I2C.h"
#include "lcd.h"
#include "main.h"

void startupConfig(void) {
    //Oscillator Configuration
    OSCCON = 0b01110000; // Force internal oscillator operation at 8MHz, Sleep mode causes device sleep (Bit 7)
    OSCTUNEbits.PLLEN = 1; // multiply to 32MHz
    
    //Pin IO, 0 output, 1 input 
    TRISA = 0b001111; // Set Port A (RA0-FLEX,RA1-COND,RA2-PROX1,RA3-PROX2,RA4-MOTORS,RA5-SOL1)
    TRISB = 0xFF; //RB3 for LCD
    TRISC = 0b11000000; //RC1-2 Servo outputs, RC3,RC4 for RTC, RC5 PROXPOW, RC0 FLEXPOW, RC6 TX, RC7 RX
    TRISD = 0x00; //RD2, RD4-7 LCDAll output mode for LCD
    TRISE = 0b000;    //RE0 - SOL2, RE1 - CONDUCTIVITY
    
    LATA = 0x00;
    LATB = 0x00; 
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0x0D;  // bits 0-3 set RA0 and RA1 as analog inputs and the rest as digital
                    // bits 4-5 set usage of VDD and VSS
    CVRCON = 0x00; // Disable CCP reference voltage output
    CMCONbits.CIS = 0;
    ADFM = 1;
    nRBPU = 0;
    
    //Enable Timer Interrupts   
    TMR0IE = 1; 
    TMR1IE = 1; 
    TMR2IE = 1; 
    TMR3IE = 1; 
    
    PEIE = 1; // Set peripheral interrupt enable bit
    initLCD();
    ei();
    return;
}
